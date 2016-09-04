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

#define DEFAULT_MAP_NAME "default"
#define DEFAULT_EPSG 3857
#define DEFAULT_MAX_X 20037508.34 // 180.0
#define DEFAULT_MAX_Y 20037508.34 // 90.0
#define DEFAULT_MIN_X -DEFAULT_MAX_X
#define DEFAULT_MIN_Y -DEFAULT_MAX_Y

static double gComplete = 0;

using namespace ngv;

int ngsQtDrawingProgressFunc(double complete, const char* /*message*/,
                       void* progressArguments) {
    GlMapView* pView = static_cast<GlMapView*>(progressArguments);
    if(complete - gComplete > 0.045) { // each 5% redraw
        pView->update ();
        gComplete = complete;
    }

    return /*pView->continueDraw() ?*/ 1 /*: 0*/;
}


GlMapView::GlMapView(ILocationStatus *status, QWidget *parent) :
    QOpenGLWidget(parent), m_mapId(0), m_locationStatus(status)
{
    // TODO: map id as input parameter
    newMap();
    setMouseTracking(true);
}


void GlMapView::initializeGL()
{
    if(!m_mapId)
        return;
    ngsMapInit(m_mapId);
}

void GlMapView::resizeGL(int w, int h)
{
    if(!m_mapId)
        return;
    m_center.setX (w / 2);
    m_center.setY (h / 2);
    ngsMapSetSize(m_mapId, w, h, 1);
}


void GlMapView::paintGL()
{
    ngsMapDraw(m_mapId, ngsQtDrawingProgressFunc, (void*)this);
}

void GlMapView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if(QApplication::keyboardModifiers().testFlag(Qt::ControlModifier) == true){
            m_startRotateZ = ngsMapGetRotate (m_mapId, ngsDirection::Z);
            QSize winSize = size ();
            m_mouseStartPoint.setX (winSize.width () / 2);
            m_mouseStartPoint.setY (winSize.height () / 2);
        }
        else if(QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier) == true){
            m_startRotateX = ngsMapGetRotate (m_mapId, ngsDirection::X);
            QSize winSize = size ();
            m_mouseStartPoint.setX (winSize.width () / 2);
            m_mouseStartPoint.setY (winSize.height () / 2);
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
                   event->pos().x () - m_mouseStartPoint.x ());

            ngsMapSetRotate (m_mapId, ngsDirection::Z, m_startRotateZ + rotate);
        }
        else if(QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier) == true){
            // rotate
            double rotate = atan2 (event->pos().y () - m_mouseStartPoint.y (),
                   event->pos().x () - m_mouseStartPoint.x ());

            double newAng = m_startRotateX + rotate;

            if(newAng > 0 && newAng < 0.4) {
                ngsMapSetRotate (m_mapId, ngsDirection::X, newAng);

            }
        }
        else {
            // pan
            QPoint mapOffset = event->pos() - m_mouseStartPoint;
            ngsCoordinate offset = ngsMapGetDistance (m_mapId, mapOffset.x (),
                                                        mapOffset.y ());
            m_mapCenter.X -= offset.X;
            m_mapCenter.Y += offset.Y;
            ngsMapSetCenter (m_mapId, m_mapCenter.X, m_mapCenter.Y);
            m_mouseStartPoint = event->pos();
        }
        update ();
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
    update ();
}

void GlMapView::closeMap()
{
    if(!m_mapId)
        ngsMapClose (m_mapId);
}


void GlMapView::newMap()
{
    closeMap ();
    unsigned char mapId = ngsMapCreate (DEFAULT_MAP_NAME, "test gl map",
                                        DEFAULT_EPSG, DEFAULT_MIN_X,
                                        DEFAULT_MIN_Y, DEFAULT_MAX_X,
                                        DEFAULT_MAX_Y);
    m_mapId = mapId;
    initMap();
}


void GlMapView::initMap()
{
    if(!m_mapId)
        return;
    const QSize viewSize = size();
    if(ngsMapSetSize (m_mapId, viewSize.width (), viewSize.height (), 1) ==
            ngsErrorCodes::SUCCESS) {
        m_mapCenter = ngsMapGetCenter(m_mapId);
    }
    ngsMapSetBackgroundColor (m_mapId, 0, 255, 0, 255);
}

