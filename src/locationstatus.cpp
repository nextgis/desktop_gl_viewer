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
#include "locationstatus.h"
#include <QHBoxLayout>

LocationStatus::LocationStatus()
{
    m_text = new QLabel(tr("unknown"));
    m_text->setMinimumWidth (260);
    QHBoxLayout *layout = new QHBoxLayout;
    // Vertical one pixel width line
    QFrame* vFrame = new QFrame;
    vFrame->setFrameShape(QFrame::VLine);
    layout->addWidget (vFrame);
    layout->addSpacing (2);
    layout->addWidget (m_text);

    layout->setMargin(0);
    layout->setSpacing(0);

    setLayout (layout);

}


void LocationStatus::setLocation(double x, double y)
{
    m_text->setText (QString("X: %1, Y: %2").arg(x, 0, 'f', 4).arg(y, 0, 'f', 4));
}
