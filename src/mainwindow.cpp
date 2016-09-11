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
#include "api.h"
#include "lib/src/version.h"

#include <QStatusBar>
#include <QSettings>
#include <QCloseEvent>
#include <QApplication>
#include <QtWidgets>


using namespace ngv;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    createActions ();
    createMenus();
    readSettings();
    ngsInit (nullptr, nullptr);

    // statusbar setup
    statusBar ()->showMessage(tr("Ready"), 30000); // time limit 30 sec.
    m_progressStatus = new ProgressStatus;
    statusBar ()->addPermanentWidget (m_progressStatus);
    m_progressStatus->hide ();
    m_locationStatus = new LocationStatus;
    statusBar ()->addPermanentWidget (m_locationStatus);

    m_eventsStatus = new EventsStatus;
    statusBar ()->addPermanentWidget (m_eventsStatus);
    statusBar ()->setStyleSheet("QStatusBar::item { border: none }"); // disable borders

    // storage setup
    if(ngsDataStoreInit ("./tmp/ngs.gpkg") != ngsErrorCodes::EC_SUCCESS) {
        QMessageBox::critical (this, tr("Error"), tr("Storage initialize failed"));
        return;
    }

    // mapview setup
    m_mapView = new GlMapView(m_locationStatus);
    setCentralWidget (m_mapView);
    m_progressStatus->setFinish (m_mapView);

    /*m_eventsStatus->addMessage ();
    m_eventsStatus->addWarning ();
    m_eventsStatus->addError ();*/
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    ngsUninit();
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

// TODO: need add layer to map dialog

void MainWindow::newFile()
{
    m_mapView->newMap();
}

void MainWindow::open()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Load map"), "", tr("NextGIS map document (*.ngmd)"));
    if(fileName.isEmpty ())
        return;
    if(!m_mapView->openMap (fileName)) {
        QMessageBox::critical (this, tr("Error"), tr("Map load failed"));
    }
}

void MainWindow::save()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save map as ..."), "", tr("NextGIS map document (*.ngmd)"));
    if(fileName.isEmpty ())
        return;
    if(!m_mapView->saveMap (fileName)) {
        QMessageBox::critical (this, tr("Error"), tr("Map save failed"));
    }
    else {
        statusBar ()->showMessage(tr("Map saved"), 10000); // time limit 10 sec.
    }
}

void MainWindow::load()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Load file to storage"), "", tr("ESRI Shape file (*.shp)"));
    // TODO: m_progressStatus should have child progresses and show full status of all progresses
    if(fileName.isEmpty ())
        return;

    const char* pszPath = fileName.toStdString ().c_str ();
    QFileInfo fileInfo(fileName);
    const char* pszName = fileInfo.baseName ().toStdString ().c_str ();

    if(ngsDataStoreLoad(pszName, pszPath, "", false, 1, LoadingProgressFunc,
               m_progressStatus) == 0) {
        QString message = QString(tr("Load %1 failed")).arg (fileName);
        QMessageBox::critical (this, tr("Error"), message);
    }
}

void MainWindow::about()
{
    QString message =  QString(tr("The <b>GL View application</b> "
                                  "test OpenGL ES rendering.<p>"
                                  "Comiled with&nbsp;libngstore %1<p>"
                                  "Run with&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
                                  "&nbsp;&nbsp;&nbsp;&nbsp;libngstore %2")).arg (
                NGS_VERSION, ngsGetVersionString("self"));
    QMessageBox::about(this, tr("About Menu"),
            message);
}

void MainWindow::createActions()
{
    m_pNewAct = new QAction(QIcon(":/images/doc_new.svg"), tr("&New"), this);
    m_pNewAct->setShortcuts(QKeySequence::New);
    m_pNewAct->setStatusTip(tr("Create a new map document"));
    connect(m_pNewAct, SIGNAL(triggered()), this, SLOT(newFile()));

    m_pOpenAct = new QAction(QIcon(":/images/doc_open.svg"), tr("&Open..."), this);
    m_pOpenAct->setShortcuts(QKeySequence::Open);
    m_pOpenAct->setStatusTip(tr("Open an existing map document"));
    connect(m_pOpenAct, SIGNAL(triggered()), this, SLOT(open()));

    m_pSaveAct = new QAction(QIcon(":/images/doc_save.svg"), tr("&Save"), this);
    m_pSaveAct->setShortcuts(QKeySequence::Save);
    m_pSaveAct->setStatusTip(tr("Save the map document to disk"));
    connect(m_pSaveAct, SIGNAL(triggered()), this, SLOT(save()));

    m_pUploadAct = new QAction(tr("&Load"), this);
    m_pUploadAct->setStatusTip(tr("Load spatial data to internal storage"));
    connect(m_pUploadAct, SIGNAL(triggered()), this, SLOT(load()));

    m_pExitAct = new QAction(tr("E&xit"), this);
    m_pExitAct->setShortcuts(QKeySequence::Quit);
    m_pExitAct->setStatusTip(tr("Exit the application"));
    connect(m_pExitAct, SIGNAL(triggered()), this, SLOT(close()));

    m_pAboutAct = new QAction(QIcon(":/images/main_logo.svg"), tr("&About"), this);
    m_pAboutAct->setStatusTip(tr("Show the application's About box"));
    m_pAboutAct->setMenuRole(QAction::AboutRole);
    connect(m_pAboutAct, SIGNAL(triggered()), this, SLOT(about()));

    m_pAboutQtAct = new QAction(tr("About &Qt"), this);
    m_pAboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    m_pAboutQtAct->setMenuRole(QAction::AboutQtRole);
    connect(m_pAboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void MainWindow::createMenus()
{
    QMenu *pFileMenu = menuBar()->addMenu(tr("&File"));
    pFileMenu->addAction(m_pNewAct);
    pFileMenu->addAction(m_pOpenAct);
    pFileMenu->addAction(m_pSaveAct);
    pFileMenu->addSeparator();
    pFileMenu->addAction(m_pUploadAct);
    pFileMenu->addSeparator();
    pFileMenu->addAction(m_pExitAct);

    QMenu *pHelpMenu = menuBar()->addMenu(tr("&Help"));
    pHelpMenu->addAction(m_pAboutAct);
    pHelpMenu->addAction(m_pAboutQtAct);
}
