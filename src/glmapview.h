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
#ifndef GLMAPVIEW_H
#define GLMAPVIEW_H

#include <QOpenGLWidget>
#include <QTimer>

#include "api.h"
#include "locationstatus.h"
#include "progressstatus.h"

namespace ngv {

class GlMapView : public QOpenGLWidget, public IProgressFinish
{
    Q_OBJECT
public:
    GlMapView(ILocationStatus *status = 0, QWidget *parent = 0);
    void closeMap();
    void newMap();
    bool openMap(const QString &path);
    bool saveMap(const QString &path);

signals:

public slots:
    void onTimer(void);

    // QOpenGLWidget interface
protected:
    virtual void initializeGL() override;
    virtual void resizeGL(int w, int h) override;
    virtual void paintGL() override;

    // mouse events
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void wheelEvent(QWheelEvent* event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;
protected:
    void initMap();

protected:
    unsigned char m_mapId;
    ngsCoordinate m_mapCenter;
    QPoint m_mouseStartPoint;
    QPoint m_center;
    double m_startRotateZ, m_startRotateX, m_beginRotateAngle;
    ILocationStatus *m_locationStatus;
    enum ngsDrawState m_drawState;
    QTimer* m_timer;

    // IProgressFinish interface
public:
    virtual void onFinish(unsigned int taskId) override;
};

}

#endif // GLMAPVIEW_H
