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
#ifndef PROGRESSSTATUS_H
#define PROGRESSSTATUS_H

#include <QWidget>
#include <QLabel>
#include <QProgressBar>

#include <string>

class IProgressFinish
{
public:
    virtual ~IProgressFinish(){}
    virtual void onFinish(unsigned int taskId) = 0;
};

int LoadingProgressFunc(unsigned int taskId, double complete, const char *message,
                             void *progressArguments);

class ProgressStatus : public QWidget
{
    friend int LoadingProgressFunc(unsigned int taskId, double complete,
                                   const char *message, void *progressArguments);
    Q_OBJECT
public:
    explicit ProgressStatus(QWidget *parent = 0);
    void setFinish(IProgressFinish *object);

signals:
    void valueChanged(int value);
    void finish(unsigned int taskId);

public slots:
    void onFinish(unsigned int taskId);

protected:
    QLabel *m_text;
    QProgressBar *m_progress;
    bool m_continue;
    IProgressFinish* m_finishObject;
};

#endif // PROGRESSSTATUS_H
