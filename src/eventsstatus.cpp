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
#include "eventsstatus.h"

#include <QHBoxLayout>
#include <QIcon>

#define ICON_SIZE 14

EventsStatus::EventsStatus(QWidget *parent) : QWidget(parent), m_errorCount(0),
    m_messageCount(0), m_warningCount(0)
{
    QHBoxLayout *layout = new QHBoxLayout;
    // TODO: does this needed? Or can be overridy by style.
    QFrame* vFrame = new QFrame;
    vFrame->setFrameShape(QFrame::VLine);
    layout->addWidget (vFrame);
    layout->addSpacing (2);

    m_icon = new QLabel;
    m_icon->setPixmap (QIcon(":/images/event_empty.svg").pixmap (ICON_SIZE, ICON_SIZE,
                                                     QIcon::Normal, QIcon::On));
    layout->addWidget (m_icon);

    m_count = new QLabel;
    QFont font = m_count->font();
    font.setPointSize(font.pointSize () - 4);
    m_count->setFont(font);
    m_count->setAlignment (Qt::AlignBottom | Qt::AlignLeft);

    layout->addWidget (m_count);

    layout->addSpacing(3);
    QLabel *el = new QLabel(tr("Event log"));
    layout->addWidget (el);

    layout->setMargin(0);
    layout->setSpacing(0);

    setLayout (layout);
}

void EventsStatus::addError()
{
    m_errorCount++;
    m_icon->setPixmap (QIcon(":/images/event_error.svg").pixmap (ICON_SIZE, ICON_SIZE,
                                                    QIcon::Normal, QIcon::On));
    m_count->setText (QString::number (m_errorCount));
}

void EventsStatus::addMessage()
{
    m_messageCount++;
    if(m_errorCount == 0 && m_warningCount == 0) {
        m_icon->setPixmap (QIcon(":/images/event_msg.svg").pixmap (ICON_SIZE, ICON_SIZE,
                                                    QIcon::Normal, QIcon::On));
        m_count->setText (QString::number (m_messageCount));
    }
}

void EventsStatus::addWarning()
{
    m_warningCount++;
    if(m_errorCount == 0) {
        m_icon->setPixmap (QIcon(":/images/event_warn.svg").pixmap (ICON_SIZE, ICON_SIZE,
                                                    QIcon::Normal, QIcon::On));
        m_count->setText (QString::number (m_warningCount));
    }
}
