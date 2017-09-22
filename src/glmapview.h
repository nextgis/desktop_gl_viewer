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

#include "locationstatus.h"
#include "mapmodel.h"

class GlMapView : public QOpenGLWidget
{
    Q_OBJECT
public:
    enum ViewMode {
        M_PAN,
        M_IDENTIFY,
        M_ZOOMIN,
        M_ZOOMOUT
    };

public:
    GlMapView(ILocationStatus *status = 0, QWidget *parent = 0);
    void setModel(MapModel *mapModel);
    bool cancelDraw() const { return false; }
    void reportSpeed(qint64 ms);
    void setMode(enum ViewMode mode);

signals:
    void setStatusText(const QString &text, int timeout = 0);

protected slots:
    virtual void onTimer(void);
    virtual void modelDestroyed();
    virtual void modelReset();
    virtual void dataChanged(const QModelIndex &topLeft,
                             const QModelIndex &bottomRight,
                             const QVector<int> &roles = QVector<int>());
    virtual void layersInserted(const QModelIndex &parent, int start, int end);
    virtual void layersRemoved(const QModelIndex &parent, int first, int last);
    virtual void layersMoved(const QModelIndex &parent, int start, int end,
                             const QModelIndex &destination, int row);
    virtual void undoEditFinished();
    virtual void redoEditFinished();
    virtual void editSaved();
    virtual void editCanceled();
    virtual void geometryCreated(const QModelIndex& parent);
    virtual void geometryEditStarted();
    virtual void geometryDeleted();
    virtual void pointAdded();
    virtual void pointDeleted();
    virtual void geometryPartAdded();
    virtual void geometryPartDeleted();

    // QOpenGLWidget interface
protected:
    virtual void resizeGL(int w, int h) override;
    virtual void paintGL() override;

    // mouse events
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void wheelEvent(QWheelEvent* event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;

protected:
    void draw(enum ngsDrawState state);

protected:
    ngsCoordinate m_mapCenter;
    QPoint m_mouseStartPoint;
    QPoint m_mouseCurrentPoint;
    QPoint m_center;
    double m_startRotateZ, m_startRotateX, m_beginRotateAngle;
    ILocationStatus *m_locationStatus;
    enum ngsDrawState m_drawState;
    QTimer* m_timer;
    MapModel* m_mapModel;
    enum ViewMode m_mode;
    bool m_editMode;
};

#endif // GLMAPVIEW_H
