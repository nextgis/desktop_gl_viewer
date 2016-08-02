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
#ifndef MAPVIEW_H
#define MAPVIEW_H

#include "progressstatus.h"

#include <QWidget>
#include <QTimer>
#include <QImage>
#include <QChar>

namespace ngv {


class MapView : public QWidget, public IProgressFinish
{
    Q_OBJECT
public:
    explicit MapView(QWidget *parent = 0);
    virtual ~MapView();
public:
    enum class State
    {
        None		= 0,
        Panning,
        Zooming,
        Rotating,
        Resizing,
        Drawing,
        Flashing
    };

public:
    bool isOk() const;
    unsigned int mapId() const;
    void setMapId(unsigned int mapId);
    bool continueDraw() const;
    void newMap();

signals:

public slots:
    void onTimer(void);

    // QWidget interface
protected:
    virtual void paintEvent(QPaintEvent *) override;
    virtual void resizeEvent(QResizeEvent *) override;

    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void wheelEvent(QWheelEvent* event) override;

protected:
    void drawBackground();

protected:
    QTimer* m_timer;
    enum State m_state;
    uchar* m_buffer;
    QImage* m_glImage;
    QColor m_bkcolor;
    bool m_ok;
    unsigned int m_mapId;

    QPoint m_imageLastDragPos;
    QPoint m_imageOffset;
    double m_imageScale;
    QPoint m_mapStartDragPos;
    QPoint m_mapOffset;
    double m_mapScale;
    double m_curScale;
    QPoint m_mapDisplayCenter;

    // IProgressFinish interface
public:
    virtual void onFinish(int type, const std::string &data) override;
};

}

#endif // MAPVIEW_H
