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
#include "mainwindow.h"

#include <math.h>

#include <QKeyEvent>
#include <QApplication>
#include <QMessageBox>
#include <QDebug>
#include <QElapsedTimer>

#ifdef _DEBUG
#   include <chrono>
#endif //DEBUG

constexpr short TM_ZOOMING = 400;
constexpr short MIN_OFF_PX = 2;
static bool fixDrawTime = false;
static QElapsedTimer fixDrawtimer;

int ngsQtDrawingProgressFunc(enum ngsCode status,
                             double /*complete*/,
                             const char* /*message*/,
                             void* progressArguments) {

//    qDebug() << "Qt draw notify: " << message << " - complete: " << complete * 100;


    if(status == ngsCode::COD_FINISHED) {
        if(fixDrawTime) {
            fixDrawTime = false;

            if(fixDrawtimer.isValid()) {
                qDebug() << "The drawing took " << fixDrawtimer.elapsed() << " milliseconds";
                GlMapView* pView = static_cast<GlMapView*>(progressArguments);
                pView->reportSpeed(fixDrawtimer.elapsed());
            }
            fixDrawtimer.invalidate();
        }
        return 1;
    }
    else if(!fixDrawTime) {
        fixDrawTime = true;
        fixDrawtimer.start();
    }

    GlMapView* pView = static_cast<GlMapView*>(progressArguments);
    pView->update();
    return pView->cancelDraw() ? 0 : 1;
}


GlMapView::GlMapView(ILocationStatus *status, QWidget *parent) :
    QOpenGLWidget(parent),
    m_locationStatus(status),
    m_drawState(DS_NORMAL),
    m_mapModel(nullptr)
{
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimer()));

    setMouseTracking(true);
    setFocusPolicy(Qt::WheelFocus);

    connect(this, SIGNAL(setStatusText(QString, int)), parent,
            SLOT(setStatusText(QString, int)));
}

void GlMapView::setModel(MapModel *mapModel)
{
    if (m_mapModel == mapModel)
        return;

    if(nullptr != m_mapModel) {
        disconnect(m_mapModel, SIGNAL(destroyed()),
                   this, SLOT(modelDestroyed()));
        disconnect(m_mapModel, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
                   this, SLOT(dataChanged(QModelIndex,QModelIndex,QVector<int>)));
        disconnect(m_mapModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
                   this, SLOT(layersInserted(QModelIndex,int,int)));
        disconnect(m_mapModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                   this, SLOT(layersRemoved(QModelIndex,int,int)));
        disconnect(m_mapModel, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
                   this, SLOT(layersMoved(QModelIndex,int,int,QModelIndex,int)));
        disconnect(m_mapModel, SIGNAL(modelReset()), this, SLOT(modelReset()));
        disconnect(m_mapModel, SIGNAL(editGeometryCreated(QModelIndex)),
                   this, SLOT(editGeometryCreated(QModelIndex)));
        disconnect(m_mapModel, SIGNAL(editGeometryAdded()),
                   this, SLOT(editGeometryAdded()));
        disconnect(m_mapModel, SIGNAL(editGeometryDeleted()),
                   this, SLOT(editGeometryDeleted()));
        disconnect(m_mapModel, SIGNAL(editHistoryUndoMade()),
                   this, SLOT(editHistoryUndoMade()));
        disconnect(m_mapModel, SIGNAL(editHistoryRedoMade()),
                   this, SLOT(editHistoryRedoMade()));
    }


    m_mapModel = mapModel;
    if(nullptr == m_mapModel)
        return;
    const QSize viewSize = size();
    m_mapModel->setSize(viewSize.width(), viewSize.height());
    ngsRGBA bk = {230, 255, 255, 255}; // green {0, 255, 0, 255};
    m_mapModel->setBackground(bk);

    connect(m_mapModel, SIGNAL(destroyed()), this, SLOT(modelDestroyed()));
    connect(m_mapModel, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
               this, SLOT(dataChanged(QModelIndex,QModelIndex,QVector<int>)));
    connect(m_mapModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
               this, SLOT(layersInserted(QModelIndex,int,int)));
    connect(m_mapModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
               this, SLOT(layersRemoved(QModelIndex,int,int)));
    connect(m_mapModel, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
               this, SLOT(layersMoved(QModelIndex,int,int,QModelIndex,int)));
    connect(m_mapModel, SIGNAL(modelReset()), this, SLOT(modelReset()));
    connect(m_mapModel, SIGNAL(editGeometryCreated(QModelIndex)),
               this, SLOT(editGeometryCreated(QModelIndex)));
    connect(m_mapModel, SIGNAL(editGeometryAdded()),
               this, SLOT(editGeometryAdded()));
    connect(m_mapModel, SIGNAL(editGeometryDeleted()),
               this, SLOT(editGeometryDeleted()));
    connect(m_mapModel, SIGNAL(editHistoryUndoMade()),
               this, SLOT(editHistoryUndoMade()));
    connect(m_mapModel, SIGNAL(editHistoryRedoMade()),
               this, SLOT(editHistoryRedoMade()));

    draw(DS_REDRAW);
}

void GlMapView::reportSpeed(qint64 ms)
{
    emit setStatusText(tr("Drawing took %1 ms").arg(ms), 2000);
}

void GlMapView::onTimer()
{
    m_timer->stop(); // one shoot for update gl view
    draw(DS_NORMAL);
}

void GlMapView::modelDestroyed()
{
    draw(DS_REDRAW);
}

void GlMapView::modelReset()
{
    if(nullptr == m_mapModel)
        return;

    const QSize viewSize = size();
    m_mapModel->setSize(viewSize.width(), viewSize.height());
    ngsRGBA bk = {230, 255, 255, 255}; // green {0, 255, 0, 255};
    m_mapModel->setBackground(bk);

    draw(DS_REDRAW);
}

void GlMapView::dataChanged(const QModelIndex &/*topLeft*/,
                            const QModelIndex &/*bottomRight*/,
                            const QVector<int> &/*roles*/)
{
    draw(DS_REDRAW);
}

void GlMapView::layersInserted(const QModelIndex &/*parent*/, int /*start*/, int /*end*/)
{
    draw(DS_REDRAW);
}

void GlMapView::layersRemoved(const QModelIndex &/*parent*/, int /*first*/, int /*last*/)
{
    draw(DS_REDRAW);
}

void GlMapView::layersMoved(const QModelIndex &/*parent*/, int /*start*/, int /*end*/,
                            const QModelIndex &/*destination*/, int /*row*/)
{
    draw(DS_REDRAW);
}

void GlMapView::editGeometryCreated(const QModelIndex& /*parent*/)
{
    draw(DS_PRESERVED);
}

void GlMapView::editGeometryAdded()
{
    draw(DS_PRESERVED);
}

void GlMapView::editGeometryDeleted()
{
    draw(DS_PRESERVED);
}

void GlMapView::editHistoryUndoMade()
{
    draw(DS_PRESERVED);
}

void GlMapView::editHistoryRedoMade()
{
    draw(DS_PRESERVED);
}

void GlMapView::resizeGL(int w, int h)
{
    if(nullptr == m_mapModel)
        return;
    m_drawState = DS_PRESERVED;
    m_center.setX (w / 2);
    m_center.setY (h / 2);
    m_mapModel->setSize(w, h);
    // send event to full redraw
    m_timer->start(TM_ZOOMING);
}


void GlMapView::paintGL()
{
    if(nullptr == m_mapModel)
        return;
    m_mapModel->draw(m_drawState, ngsQtDrawingProgressFunc,
                        static_cast<void*>(this));
    m_drawState = DS_PRESERVED; // draw from cache on display update
}

void GlMapView::mousePressEvent(QMouseEvent *event)
{
    if(nullptr == m_mapModel)
        return;

    // For mouse press event this includes the button that caused the event.
    if (event->button() == Qt::LeftButton) {
        if(QApplication::keyboardModifiers().testFlag(Qt::ControlModifier) == true){
            m_startRotateZ = m_mapModel->getRotate(ngsDirection::DIR_Z);
            QSize winSize = size ();
            m_mouseStartPoint.setX (winSize.width () / 2);
            m_mouseStartPoint.setY (winSize.height () / 2);
            m_beginRotateAngle = atan2(event->pos().y() - m_mouseStartPoint.y(),
                                        event->pos().x() - m_mouseStartPoint.x());
        }
        else if(QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier) == true){
            m_startRotateX = m_mapModel->getRotate(ngsDirection::DIR_X);
            m_mouseStartPoint = event->pos();
        }
        else {
            m_mouseStartPoint = event->pos();
            m_mapModel->mapTouch(
                    m_mouseStartPoint.x(), m_mouseStartPoint.y(), MTT_ON_DOWN);
        }
    }
}

void GlMapView::mouseMoveEvent(QMouseEvent *event)
{
    if(nullptr == m_mapModel) {
        return;
    }

    // For mouse move events, this is all buttons that are pressed down.
    if (event->buttons() & Qt::LeftButton) {
        if(QApplication::keyboardModifiers().testFlag(Qt::ControlModifier)) {
            // rotate
            double rotate = atan2(event->pos().y() - m_mouseStartPoint.y(),
                   event->pos().x() - m_mouseStartPoint.x()) - m_beginRotateAngle;

            m_mapModel->setRotate(ngsDirection::DIR_Z, -rotate + m_startRotateZ);
        }
        else if(QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)) {
            // rotate
            double rotate = (event->pos().y() - m_mouseStartPoint.y()) *
                    M_PI / size().height();

            double newAng = m_startRotateX + rotate;

            // limit from -17 to 80 degree
            if(newAng < -0.3 || newAng > 1.41)
                return;
            m_mapModel->setRotate(ngsDirection::DIR_X, newAng);
        }
        else {
            // pan
            QPoint mapOffset = event->pos() - m_mouseStartPoint;
            if(abs(mapOffset.x()) > MIN_OFF_PX ||
               abs(mapOffset.y()) > MIN_OFF_PX) {
                m_mouseStartPoint = event->pos();
                m_mapModel->mapTouch(m_mouseStartPoint.x(),
                        m_mouseStartPoint.y(), MTT_ON_MOVE);
            }
        }
        draw (DS_PRESERVED);
        m_timer->start(TM_ZOOMING);
    }

    if(m_locationStatus) {
        ngsCoordinate coord = m_mapModel->getCoordinate(event->pos().x(),
                                                        event->pos().y());
        m_locationStatus->setLocation(coord.X, coord.Y);
    }
}

void GlMapView::mouseReleaseEvent(QMouseEvent *event)
{
    if(nullptr == m_mapModel)
        return;

    // For mouse release events this excludes the button that caused the event.
    if(!(event->modifiers() & Qt::LeftButton)) {
        if(QApplication::keyboardModifiers().testFlag(Qt::ControlModifier) == true){
        }
        else if(QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier) == true){
        }
        else {
            ngsDrawState drawState = m_mapModel->mapTouch(
                    m_mouseStartPoint.x(), m_mouseStartPoint.y(), MTT_ON_UP);
            draw(drawState);
        }
    }
}

void GlMapView::wheelEvent(QWheelEvent* event)
{
    if(nullptr == m_mapModel)
        return;

    double scale = 1;
    double delta = event->delta();
    double add = fabs(delta) / 60;
    scale = m_mapModel->getScale();
    if(delta > 0)
        scale *= add;
    else
        scale /= add;

    m_mapModel->setScale(scale);
    draw(DS_PRESERVED);

    // Send event to full redraw
    m_timer->start(TM_ZOOMING);
}

void GlMapView::draw(ngsDrawState state)
{
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
