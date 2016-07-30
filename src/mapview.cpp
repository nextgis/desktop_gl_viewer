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

using namespace ngv;

#define TM_RESIZING 250
#define DEFAULT_MAP_NAME "default"
#define DEFAULT_EPSG 3857
#define DEFAULT_MAX_X 20037508.34 // 180.0
#define DEFAULT_MAX_Y 20037508.34 // 90.0
#define DEFAULT_MIN_X -DEFAULT_MAX_X
#define DEFAULT_MIN_Y -DEFAULT_MAX_Y

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
    m_glImage(nullptr), m_ok(false), m_mapId(15000)
{
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

void MapView::onTimer()
{
    if(m_state == State::Resizing){
        m_state = State::Drawing;
        m_timer->stop ();

        if(m_ok) {
            const QSize viewSize = size();
            ngsInitMap (m_mapId, m_buffer, viewSize.width (), viewSize.height (), true);

            delete m_glImage;
            m_glImage = new QImage(m_buffer, viewSize.width (), viewSize.height (),
                                   QImage::Format_RGBA8888);

            gComplete = 0;
            ngsDrawMap (m_mapId, ngsQtDrawingProgressFunc, (void*)this);
        }
    }
}

void MapView::paintEvent(QPaintEvent *)
{
    if(m_state == State::None){
        return drawBackground();
    }

    if(nullptr != m_glImage){
        if(m_state == State::Drawing){
            QPainter painter(this);
            painter.drawImage (QPoint(0,0), *m_glImage);
        }
        else if(m_state == State::Resizing){
            const QSize viewSize = size();
            QPoint insertPoint(int(viewSize.width () * .5) -
                               int(m_glImage->width () * .5),
                               int(viewSize.height () * .5) -
                               int(m_glImage->height () * .5) );
            QPainter painter(this);
            painter.setBrush(m_bkcolor);
            painter.drawRect(0, 0, viewSize.width (), viewSize.height ());
            painter.drawImage (insertPoint, *m_glImage);
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

void MapView::setMapId(unsigned int mapId)
{
    m_mapId = mapId;
    m_ok = false;
    const QSize viewSize = size();
    if(ngsInitMap (m_mapId, m_buffer, viewSize.width (), viewSize.height (), true) ==
            ngsErrorCodes::SUCCESS) {
        m_ok = true;
        gComplete = 0;
        ngsDrawMap (m_mapId, ngsQtDrawingProgressFunc, (void*)this);
    }
}

bool MapView::continueDraw() const
{
    return true;
}

void MapView::newMap()
{
    const QSize viewSize = size();
    int mapId = ngsCreateMap (DEFAULT_MAP_NAME, "test gl map", DEFAULT_EPSG,
                              DEFAULT_MIN_X, DEFAULT_MIN_Y, DEFAULT_MAX_X,
                              DEFAULT_MAX_Y);
    if(mapId != -1) {
        m_mapId = static_cast<unsigned int>(mapId);
        if(ngsInitMap (m_mapId, m_buffer, viewSize.width (),
                   viewSize.height (), true) == ngsErrorCodes::SUCCESS) {
            // set green gl background to see offscreen raster in window
            ngsSetMapBackgroundColor (m_mapId, 0, 255, 0, 255);
            m_ok = true;
        }
    }
}

unsigned int MapView::mapId() const
{
    return m_mapId;
}


void ngv::MapView::onFinish(int /*type*/, const std::string &data)
{
    if(ngsCreateLayer (m_mapId, "ov3", data.c_str ()) == ngsErrorCodes::SUCCESS) {
        gComplete = 0;
        ngsDrawMap (m_mapId, ngsQtDrawingProgressFunc, (void*)this);
    }
}
