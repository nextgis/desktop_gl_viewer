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
#include "mainwindow.h"
#include "mapview.h"

#include <QStatusBar>
#include <QSettings>
#include <QCloseEvent>
#include <QApplication>

using namespace ngv;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    readSettings();
    statusBar()->showMessage(tr("Ready"));
    setCentralWidget (new MapView());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

void MainWindow::writeSettings()
{
    QSettings settings;

    settings.beginGroup("MainWindow");
    if(isMaximized()){
        settings.setValue("frame/maximized", true);
    }
    else{
        settings.setValue("frame/size", size());
        settings.setValue("frame/pos", pos());
    }
    settings.setValue("frame/state", saveState());
    settings.setValue("frame/statusbar/shown", statusBar()->isVisible());
    settings.endGroup();
}

void MainWindow::readSettings()
{
    setWindowIcon(QIcon(":/images/main_logo.svg"));

    QSettings settings;
    settings.beginGroup("MainWindow");
    if(settings.value("frame/maximized", false).toBool()){
        showMaximized();
    }
    else{
        resize(settings.value("frame/size", QSize(400, 400)).toSize());
        move(settings.value("frame/pos", QPoint(200, 200)).toPoint());
    }
    restoreState(settings.value("frame/state").toByteArray());
    statusBar()->setVisible(settings.value("frame/statusbar/shown", true).toBool());
    settings.endGroup();
}
