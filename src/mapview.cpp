/******************************************************************************
*  Project: NextGIS GL Viewer
*  Purpose: GUI viewer for spatial data.
*  Author:  Dmitry Baryshnikov, bishop.dev@gmail.com
*******************************************************************************
*  Copyright (C) 2016 NextGIS, <info@nextgis.com>
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 2 of the License, or
*   (at your option) any later version.
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/
#include "mapview.h"
#include "api.h"

#include <QDebug>
#include <QPainter>
#include <QApplication>
#include <QDesktopWidget>
#include <QKeyEvent>

using namespace ngv;

#define TM_RESIZING 350
#define TM_ZOOMING 550
#define DEFAULT_MAP_NAME "default"
#define DEFAULT_EPSG 3857
#define DEFAULT_MAX_X 20037508.34 // 180.0
#define DEFAULT_MAX_Y 20037508.34 // 90.0
#define DEFAULT_MIN_X -DEFAULT_MAX_X
#define DEFAULT_MIN_Y -DEFAULT_MAX_Y
#define DEFAULT_SCALE 1.0
#define ZOOM_FACTOR 0.8

static double gComplete = 0;

int ngsQtDrawingProgressFunc(double complete, const char* /*message*/,
                       void* progressArguments) {
    MapView* pView = static_cast<MapView*>(progressArguments);
    if(complete - gComplete > 0.045) { // each 5% redraw
        pView->update ();
        gComplete = complete;
    }

    return pView->continueDraw() ? 1 : 0;
}

MapView::MapView(QWidget *parent) : QWidget(parent), m_state(State::None),
    m_glImage(nullptr), m_ok(false), m_mapId(-1)
{
    m_mapScale = m_imageScale = m_mapScale = DEFAULT_SCALE;
    m_imageOffset = m_mapOffset = QPoint();

    QRect rec = QApplication::desktop()->screenGeometry();
    size_t bufferSize = size_t(rec.height() * rec.width() * 4);
    m_buffer = new uchar[bufferSize];

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimer()));

    m_bkcolor = QColor(210, 245, 255);

    // init buffer with grey
    memset (m_buffer, 128, bufferSize);

    const QSize viewSize = size();
    m_glImage = new QImage(m_buffer, viewSize.width (), viewSize.height (),
                           QImage::Format_RGBA8888);
    newMap();
}

MapView::~MapView()
{
    delete m_glImage;
    delete m_buffer;
}

bool MapView::isOk() const
{
    return m_ok;
}

void MapView::closeMap()
{
    if(-1 != m_mapId)
        ngsMapClose (m_mapId);
}

bool MapView::openMap(const QString &path)
{
    m_mapId = ngsMapOpen (path.toStdString ().c_str ());
    initMap();
    return -1 != m_mapId;
}

bool MapView::saveMap(const QString &path)
{
    return ngsMapSave (m_mapId,  path.toStdString ().c_str ())
            == ngsErrorCodes::SUCCESS;
}

void MapView::onTimer()
{
    switch (m_state) {
        case State::Resizing: {
            m_state = State::Drawing;
            m_timer->stop();

            if(m_ok) {
                const QSize viewSize = size();
                ngsMapInit(m_mapId, m_buffer, viewSize.width(), viewSize.height(), true);

                delete m_glImage;
                m_glImage = new QImage(m_buffer, viewSize.width(), viewSize.height(),
                                       QImage::Format_RGBA8888);
                gComplete = 0;
                ngsMapDraw(m_mapId, ngsQtDrawingProgressFunc, (void*) this);
            }
            break;
        }

        case State::Panning: {
        // put current scaled picture of display to m_glImage buffer
        // this need if user right after wheel make pan and during panning
        // old unscaled image seen
            QPixmap pixmap = grab();
            QPainter qPainter(m_glImage);
            qPainter.drawPixmap (0, 0, pixmap);

            m_state = State::Drawing;
            m_timer->stop();

            if(m_ok) {
                ngsMapSetDisplayCenter(m_mapId, m_mapDisplayCenter.x(), m_mapDisplayCenter.y());

                gComplete = 0;
                ngsMapDraw(m_mapId, ngsQtDrawingProgressFunc, (void*) this);
            }
            break;
        }

        case State::Zooming: {
        // put current scaled picture of display to m_glImage buffer
        // this need if user right after wheel make pan and during panning
        // old unscaled image seen
            QPixmap pixmap = grab();
            QPainter qPainter(m_glImage);
            qPainter.drawPixmap (0, 0, pixmap);

            m_state = State::Drawing;
            m_timer->stop();

            if(m_ok) {


                ngsMapGetScale(m_mapId, &m_mapScale);
                m_mapScale /= m_curScale;
                ngsMapSetScale(m_mapId, m_mapScale);

                gComplete = 0;
                ngsMapDraw(m_mapId, ngsQtDrawingProgressFunc, (void*) this);
            }
            break;
        }

        case State::Rotating: {
            break;
        }

        case State::Drawing: {
            break;
        }

        case State::Flashing: {
            break;
        }

        case State::None: {
            break;
        }
    }

    m_imageOffset = m_mapOffset = QPoint();
    m_imageScale = m_curScale = DEFAULT_SCALE;
}

void MapView::paintEvent(QPaintEvent *)
{
    if(m_state == State::None){
        return drawBackground();
    }

    if(nullptr != m_glImage){

        switch (m_state) {
            case State::Drawing: {
                QPainter painter(this);
                painter.drawImage(QPoint(0,0), *m_glImage);
                break;
            }

            case State::Resizing: {
                const QSize viewSize = size();
                QPoint insertPoint(int(viewSize.width() * .5) -
                                   int(m_glImage->width() * .5),
                                   int(viewSize.height() * .5) -
                                   int(m_glImage->height() * .5) );
                QPainter painter(this);
                painter.setBrush(m_bkcolor);
                painter.drawRect(0, 0, viewSize.width(), viewSize.height());
                painter.drawImage(insertPoint, *m_glImage);
                break;
            }

            case State::Panning: {
                const QSize viewSize = size();
                QPainter painter(this);
                painter.setBrush(m_bkcolor);
                painter.drawRect(0, 0, viewSize.width(), viewSize.height());
                painter.drawImage(m_imageOffset, *m_glImage);
                break;
            }

            case State::Zooming: {
            // TODO: maybe set new matrix for zoom/pan and get result buffer without
            // reread from datasource will be faster than raster scale operation?

                double scaleFactor = m_imageScale / m_curScale;
                int newWidth = int (m_glImage->width() * scaleFactor);
                int newHeight = int (m_glImage->height() * scaleFactor);
                int newX = m_imageOffset.x() + (m_glImage->width() - newWidth) / 2;
                int newY = m_imageOffset.y() + (m_glImage->height() - newHeight) / 2;

                const QSize viewSize = size();
                QPainter painter(this);

                painter.setBrush(m_bkcolor);
                painter.drawRect(0, 0, viewSize.width(), viewSize.height());

                painter.save();
                painter.translate(newX, newY);
                painter.scale(scaleFactor, scaleFactor);
                QRectF exposed = painter.matrix().inverted().mapRect(rect()).adjusted(-1, -1, 1, 1);
                painter.drawImage(exposed, *m_glImage, exposed);
                painter.restore();                
                break;
            }

            case State::Rotating: {
                break;
            }

            case State::Flashing: {
                break;
            }

            case State::None: {
                break;
            }
        }
    }
}

void MapView::resizeEvent(QResizeEvent *)
{
    if(m_state != State::Resizing) {
        // start resizing action
        m_state = State::Resizing;
    }
    m_timer->start(TM_RESIZING);
}

void MapView::drawBackground()
{
    const QSize viewSize = size();
    QPainter painter(this);
    painter.setBrush(m_bkcolor);
    painter.drawRect(0, 0, viewSize.width (), viewSize.height ());
}

void MapView::initMap()
{
    m_ok = false;
    if(-1 == m_mapId)
        return;
    const QSize viewSize = size();
    if(ngsMapInit (m_mapId, m_buffer, viewSize.width (), viewSize.height (), true) ==
            ngsErrorCodes::SUCCESS) {
        ngsMapGetDisplayCenter(m_mapId, &m_mapDisplayCenter.rx(),
                               &m_mapDisplayCenter.ry());
        m_ok = true;
        gComplete = 0;
        ngsMapDraw (m_mapId, ngsQtDrawingProgressFunc, (void*)this);
    }
}

bool MapView::continueDraw() const
{
    return true;
}

void MapView::newMap()
{
    closeMap ();
    const QSize viewSize = size();
    int mapId = ngsMapCreate (DEFAULT_MAP_NAME, "test gl map", DEFAULT_EPSG,
                              DEFAULT_MIN_X, DEFAULT_MIN_Y, DEFAULT_MAX_X,
                              DEFAULT_MAX_Y);
    m_mapId = mapId;
    initMap();
    if(mapId != -1) {
        // set green gl background to see offscreen raster in window
        ngsMapSetBackgroundColor (m_mapId, 0, 255, 0, 255);
        ngsMapGetDisplayCenter(m_mapId, &m_mapDisplayCenter.rx(), &m_mapDisplayCenter.ry());
    }
}

void ngv::MapView::onFinish(int /*type*/, const std::string &data)
{
    if(ngsMapCreateLayer (m_mapId, "orbv3", data.c_str ()) == ngsErrorCodes::SUCCESS) {
        gComplete = 0;
        ngsMapDraw (m_mapId, ngsQtDrawingProgressFunc, (void*)this);
    }
}

void MapView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_imageOffset = m_mapOffset = QPoint();
        m_imageLastDragPos = m_mapStartDragPos = event->pos();

        // if just after zoom
        ngsMapGetScale(m_mapId, &m_mapScale);
        m_mapScale /= m_curScale;
        ngsMapSetScale(m_mapId, m_mapScale);

        ngsMapGetDisplayCenter(m_mapId, &m_mapDisplayCenter.rx(), &m_mapDisplayCenter.ry());
    }
}

void MapView::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton) {
        if(m_state != State::Panning) {
            // start panning action
            m_state = State::Panning;
        }

        m_imageOffset += event->pos() - m_imageLastDragPos;
        m_imageLastDragPos = event->pos();

        update();
    }
}

void MapView::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        if(m_state != State::Panning) {
            // end panning action
            m_state = State::Panning;
        }

        m_imageOffset += event->pos() - m_imageLastDragPos;
        m_mapOffset += event->pos() - m_mapStartDragPos;
        m_mapDisplayCenter -= m_mapOffset;

        update();
        m_timer->start(TM_RESIZING);
    }
}

void MapView::wheelEvent(QWheelEvent* event)
{
    if(m_state != State::Zooming) {
        // start zooming action
        m_state = State::Zooming;
    }

    int numDegrees = event->delta() / 8;
    double numSteps = numDegrees / 15.0f;
    m_curScale *= pow(ZOOM_FACTOR, numSteps);

    update();
    m_timer->start(TM_ZOOMING);
}
