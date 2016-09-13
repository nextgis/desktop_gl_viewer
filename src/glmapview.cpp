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
#include "glmapview.h"

#include <QKeyEvent>
#include <QApplication>
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

#define DEFAULT_MAP_NAME "default"
#define DEFAULT_EPSG 3857
#define DEFAULT_MAX_X 20037508.34 // 180.0
#define DEFAULT_MAX_Y 20037508.34 // 90.0
#define DEFAULT_MIN_X -DEFAULT_MAX_X
#define DEFAULT_MIN_Y -DEFAULT_MAX_Y
#define MIN_OFF_PX 2
#define TM_ZOOMING 350
#define YORIENT 0

static double gComplete = 0;

using namespace ngv;

int ngsQtDrawingProgressFunc(unsigned int /*taskId*/, double complete,
                             const char* /*message*/,
                             void* progressArguments) {

    qDebug() << "Qt draw notiy: " << complete << " g:" << gComplete;

    GlMapView* pView = static_cast<GlMapView*>(progressArguments);
    if(complete - gComplete > 0.045) { // each 5% redraw
        pView->update ();
        gComplete = complete;
    }

    return /*pView->continueDraw() ?*/ 1 /*: 0*/;
}


GlMapView::GlMapView(ILocationStatus *status, QWidget *parent) :
    QOpenGLWidget(parent), m_mapId(0), m_locationStatus(status),
    m_drawState(DS_NORMAL)
{
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimer()));

    // TODO: map id as input parameter. GlMapView for each map
    newMap();
    setMouseTracking(true);
    setFocusPolicy(Qt::WheelFocus);
}

void GlMapView::onTimer()
{
    m_timer->stop(); // one shoot for update gl view
    draw (DS_NORMAL);
}

void GlMapView::initializeGL()
{
    if(0 == m_mapId)
        return;
    ngsMapInit(m_mapId);
}

void GlMapView::resizeGL(int w, int h)
{
    if(0 == m_mapId)
        return;
    m_drawState = DS_PRESERVED;
    m_center.setX (w / 2);
    m_center.setY (h / 2);
    ngsMapSetSize(m_mapId, w, h, YORIENT);
    // send event to full redraw
    m_timer->start(TM_ZOOMING);
}


void GlMapView::paintGL()
{
    ngsMapDraw(m_mapId, m_drawState, ngsQtDrawingProgressFunc, (void*)this);
    m_drawState = DS_PRESERVED; // draw from cache on display update
}

void GlMapView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if(QApplication::keyboardModifiers().testFlag(Qt::ControlModifier) == true){
            m_startRotateZ = ngsMapGetRotate (m_mapId, ngsDirection::Z);
            QSize winSize = size ();
            m_mouseStartPoint.setX (winSize.width () / 2);
            m_mouseStartPoint.setY (winSize.height () / 2);
            m_beginRotateAngle = atan2 (event->pos().y () - m_mouseStartPoint.y (),
                                        event->pos().x () - m_mouseStartPoint.x ());
        }
        else if(QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier) == true){
            m_startRotateX = ngsMapGetRotate (m_mapId, ngsDirection::X);
            QSize winSize = size ();
            m_mouseStartPoint.setX (winSize.width () / 2);
            m_mouseStartPoint.setY (winSize.height () / 2);
            m_beginRotateAngle = atan2 (event->pos().y () - m_mouseStartPoint.y (),
                                        event->pos().x () - m_mouseStartPoint.x ());
        }
        else {
            m_mouseStartPoint = event->pos ();
        }
    }
}

void GlMapView::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        if(QApplication::keyboardModifiers().testFlag(Qt::ControlModifier) == true){
            // rotate
            double rotate = atan2 (event->pos().y () - m_mouseStartPoint.y (),
                   event->pos().x () - m_mouseStartPoint.x ()) - m_beginRotateAngle;

            ngsMapSetRotate (m_mapId, ngsDirection::Z, -(m_startRotateZ + rotate));
        }
        else if(QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier) == true){
            // rotate
            //  - M_PI
            double rotate = (event->pos().y () - m_mouseStartPoint.y ()) *
                    M_PI / size().height ();

            double newAng = m_startRotateX + rotate;

            // limit from -17 to 80 degree
            if(newAng < -0.3 || newAng > 1.41)
                return;
            ngsMapSetRotate (m_mapId, ngsDirection::X, newAng);
        }
        else {
            // pan
            QPoint mapOffset = event->pos() - m_mouseStartPoint;
            if(abs(mapOffset.x ()) > MIN_OFF_PX ||
               abs(mapOffset.y ()) > MIN_OFF_PX) {
                ngsCoordinate offset = ngsMapGetDistance (m_mapId, mapOffset.x (),
                                                            mapOffset.y ());
                m_mapCenter.X -= offset.X;
                m_mapCenter.Y += offset.Y;
                ngsMapSetCenter (m_mapId, m_mapCenter.X, m_mapCenter.Y);
                m_mouseStartPoint = event->pos();
            }
        }
        draw (DS_PRESERVED);
        m_timer->start(TM_ZOOMING);
    }

    if(m_locationStatus) {
        ngsCoordinate coord = ngsMapGetCoordinate (m_mapId, event->pos ().x (),
                                                   event->pos ().y ());
        m_locationStatus->setLocation (coord.X, coord.Y);
    }
}

void GlMapView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        if(QApplication::keyboardModifiers().testFlag(Qt::ControlModifier) == true){
        }
        else if(QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier) == true){
        }
        else {
            m_mapCenter = ngsMapGetCenter(m_mapId);
        }
    }
}

void GlMapView::wheelEvent(QWheelEvent* event)
{
    double scale = 1;
    double delta = event->delta();
    double add = fabs(delta) / 80;
    scale = ngsMapGetScale(m_mapId);
    if(delta > 0)
        scale *= add;
    else
        scale /= add;

    ngsMapSetScale(m_mapId, scale);
    draw(DS_PRESERVED);

    // send event to full redraw
    m_timer->start(TM_ZOOMING);
}

void GlMapView::closeMap()
{
    if(0 != m_mapId) {
        makeCurrent();
        if(ngsMapClose (m_mapId) == ngsErrorCodes::EC_SUCCESS)
            m_mapId = 0;
        else
            QMessageBox::critical (this, tr("Error"), tr("Close map failed"));
    }
}

bool GlMapView::openMap(const QString &path)
{
    closeMap ();
    m_mapId = ngsMapOpen (path.toStdString ().c_str ());    
    m_drawState = DS_REDRAW;
    initMap();
    return 0 != m_mapId;
}

bool GlMapView::saveMap(const QString &path)
{
    return ngsMapSave (m_mapId,  path.toStdString ().c_str ())
            == ngsErrorCodes::EC_SUCCESS;
}


void GlMapView::newMap()
{
    closeMap ();
    m_mapId = ngsMapCreate (DEFAULT_MAP_NAME, "test gl map",
                                        DEFAULT_EPSG, DEFAULT_MIN_X,
                                        DEFAULT_MIN_Y, DEFAULT_MAX_X,
                                        DEFAULT_MAX_Y);
    initMap();
}


void GlMapView::initMap()
{
    if(0 == m_mapId)
        return;

    const QSize viewSize = size();
    if(ngsMapSetSize (m_mapId, viewSize.width (), viewSize.height (), YORIENT) ==
            ngsErrorCodes::EC_SUCCESS) {
        m_mapCenter = ngsMapGetCenter(m_mapId);
    }
    ngsMapSetBackgroundColor (m_mapId, 0, 255, 0, 255);
    makeCurrent();
    ngsMapInit(m_mapId);
}

void GlMapView::draw(ngsDrawState state)
{
    gComplete = 0;
    m_drawState = state;
    update();
}

void GlMapView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F5) {
        draw(DS_REDRAW);
        return;
    }

    QWidget::keyPressEvent(event);
}


void GlMapView::onFinish(unsigned int taskId)
{
    ngsLoadTaskInfo info = ngsDataStoreGetLoadTaskInfo(taskId);
    if(info.status != ngsErrorCodes::EC_SUCCESS) {
        QString message = QString(tr("Load %1 failed")).arg (info.name);
        QMessageBox::critical (this, tr("Error"), message);
        return;
    }

    QFileInfo fileInfo(QDir(info.dstPath), info.newName);
    // add loaded data to map as new layer
    if(ngsMapCreateLayer (m_mapId, info.name,
                          fileInfo.absoluteFilePath ().toUtf8 ().constData ()) ==
            ngsErrorCodes::EC_SUCCESS) {
        draw(DS_NORMAL);
    }
}
