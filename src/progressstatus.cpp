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
#include "progressstatus.h"

#include <QHBoxLayout>
#include <QDebug>
#include <QShowEvent>
#include <QHideEvent>
#include <QApplication>

#define DELTA 0.00000001

using namespace ngv;

ProgressStatus::ProgressStatus(QWidget *parent) : QWidget(parent),
    m_continue(true), m_finishObject(nullptr)
{
    m_text = new QLabel(tr("process running ..."));
    m_progress = new QProgressBar;
    m_progress->setRange (0, 100);
    m_progress->setValue (50);
    m_progress->setMinimumWidth (32);
    m_progress->setMaximumHeight (16);

    QHBoxLayout *layout = new QHBoxLayout;
    // TODO: does this needed? Or can be overridy by style.
    QFrame* vFrame = new QFrame;
    vFrame->setFrameShape(QFrame::VLine);
    layout->addWidget (vFrame);
    layout->addSpacing (2);

    layout->addWidget (m_progress);
    layout->addSpacing (2);
    layout->addWidget (m_text);

    layout->setMargin(0);
    layout->setSpacing(0);

    setLayout (layout);
}

void ProgressStatus::setValue(int value) {
    m_progress->setValue (value);
}

void ProgressStatus::setFinish(IProgressFinish *object, int type, const std::string &data)
{
    m_finishObject = object;
    m_finishType = type;
    m_finishData = data;
}

void ProgressStatus::onFinish()
{
    if(nullptr != m_finishObject)
        m_finishObject->onFinish (m_finishType, m_finishData);
}

int ngv::LoadingProgressFunc(double complete, const char* /*message*/,
                       void* progressArguments) {
//    if(nullptr != message)
//        qDebug() << "Qt load notiy: " << complete << " msg:" << message;

    ProgressStatus* status = reinterpret_cast<ProgressStatus*>(progressArguments);
    if(nullptr != status) {
        if(status->isHidden () && complete < .99999999)
            QMetaObject::invokeMethod(status, "show");

        if(!status->isHidden ()) {
            if ( 1 - complete < DELTA) {
                QMetaObject::invokeMethod(status, "hide");
                status->onFinish ();
            }
            else {
                status->setValue (static_cast<int>(complete * 100));
            }
        }

        return status->m_continue ? 1 : 0;
    }
    return 1;
}
