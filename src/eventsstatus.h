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
#ifndef EVENTSSTATUS_H
#define EVENTSSTATUS_H

#include <QWidget>
#include <QLabel>

class EventsStatus : public QWidget
{
    Q_OBJECT
public:
    explicit EventsStatus(QWidget *parent = 0);
    void addError();
    void addMessage();
    void addWarning();
signals:

public slots:

protected:
    QLabel *m_count, *m_icon;
    int m_errorCount, m_messageCount, m_warningCount;
};

#endif // EVENTSSTATUS_H
