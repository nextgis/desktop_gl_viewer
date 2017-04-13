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

// Qt
#include <QApplication>
#include <QCloseEvent>
#include <QSettings>
#include <QStatusBar>
#include <QtWidgets>

// ngstore
#include "ngstore/api.h"
#include "ngstore/version.h"

#include "catalogdialog.h"
#include "version.h"

using namespace ngv;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    char** options = nullptr;
    options = ngsAddNameValue(options, "DEBUG_MODE", "ON");
    options = ngsAddNameValue(options, "SETTINGS_DIR",
                              ngsFormFileName(ngsGetCurrentDirectory(), "tmp",
                                              nullptr));

    int result = ngsInit(options);

    ngsDestroyList(options);

    if(result == ngsErrorCodes::EC_SUCCESS && createDatastore()) {

        createActions ();
        createMenus();
        readSettings();


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

        // mapview setup
        m_mapView = new GlMapView(m_locationStatus);
        setCentralWidget (m_mapView);
        m_progressStatus->setFinish (m_mapView);

        /*m_eventsStatus->addMessage ();
        m_eventsStatus->addWarning ();
        m_eventsStatus->addError ();*/

    }
    else {
        QMessageBox::critical(this, tr("Error"), tr("Storage initialize failed"));
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    ngsUnInit();
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
    settings.setValue("frame/statusbar_shown", statusBar()->isVisible());
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
    statusBar()->setVisible(settings.value("frame/statusbar_shown", true).toBool());

    settings.endGroup();
}

void MainWindow::newFile()
{
    m_mapView->newMap();
}

void MainWindow::open()
{
    // TODO: Change on catalog open dialog
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Load map"), "", tr("NextGIS map document (*.ngmd)"));
    if(fileName.isEmpty ())
        return;
    if(!m_mapView->openMap (fileName)) {
        QMessageBox::critical(this, tr("Error"), tr("Map load failed"));
    }
}

void MainWindow::save()
{
    // TODO: Change on catalog save dialog
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
    // 1. Choose file dialog
    CatalogDialog dlg(this);
    dlg.exec();

//    // 2. Show progress dialog
//    if(ngsCatalogObjectLoad(shapePath, m_storePath.c_str(), nullptr,
//                            ngsTestProgressFunc,
//                            nullptr) != ngsErrorCodes::EC_SUCCESS) {
//        QString message = QString(tr("Load %1 to store failed")).arg (shapePath);
//        QMessageBox::critical (this, tr("Error"), message);
//    }
    // 3. Hide/destroy progress dialog

    // TODO: need methods (C API?) to get various filtes and  "All vector datasets", "All raster datasets" filters. Also for selected dataset need corresponding GDALDriver, and create/open options.
    // TODO: Own data model for datasets
/*    QString vecFilters(ngsGetFilters(DT_VECTOR_ALL, FM_READ, ""));
    QString rasFilters(ngsGetFilters(DT_RASTER_ALL, FM_READ, ""));
    QString filters(vecFilters + ";;" + rasFilters);
    QString selectedFilter;

    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Load file to storage"), "", filters, &selectedFilter);
    // TODO: m_progressStatus should have child progresses and show full status of all progresses
    if(fileName.isEmpty ())
        return;

    const char* pszPath = fileName.toUtf8().constData();
    QFileInfo fileInfo(fileName);
    const char* pszName = fileInfo.baseName ().toUtf8().constData();

    if(selectedFilter.startsWith ("Vector")) {
        const char *options[3] = {"LOAD_OP=COPY", "FEATURES_SKIP=EMPTY_GEOMETRY",
                                  nullptr};

/*        if(ngsDataStoreLoad(pszName, pszPath, "", options, LoadingProgressFunc,
                   m_progressStatus) == 0) {
            QString message = QString(tr("Load %1 failed")).arg (fileName);
            QMessageBox::critical (this, tr("Error"), message);
        }*//*
    }
    else if(selectedFilter.startsWith ("Raster")) {
        const char *options[3] = {"LOAD_OP=COPY", "RASTER_PROJECT=ON",
                                  nullptr};

/*        if(ngsDataStoreLoad(pszName, pszPath, "", options, LoadingProgressFunc,
                   m_progressStatus) == 0) {
            QString message = QString(tr("Load %1 failed")).arg (fileName);
            QMessageBox::critical (this, tr("Error"), message);
        }*//*
    }*/
}

void MainWindow::addMapLayer()
{

}

void MainWindow::removeMapLayer()
{

}

void MainWindow::about()
{
    QString appVersion(NGGLV_VERSION_STRING);
    QString libVersion(NGS_VERSION);
#ifdef Q_OS_MACOS
    QString format("OpenGL");
#else
    QString format("OpenGL ES");
#endif
    QString message =  QString(tr("The <b>GL View application</b> "
                                  "test %1 rendering (version %2).<p>"
                                  "Compiled with&nbsp;&nbsp;libngstore %3<p>"
                                  "Run with&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
                                  "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;libngstore %4")).arg (
                format, appVersion, libVersion, ngsGetVersionString("self"));
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

    m_pLoadAct = new QAction(tr("&Load"), this);
    m_pLoadAct->setStatusTip(tr("Load spatial data to internal storage"));
    connect(m_pLoadAct, SIGNAL(triggered()), this, SLOT(load()));

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

    m_pAddLayerAct = new QAction(tr("Add layer"), this);
    connect(m_pAboutQtAct, SIGNAL(triggered()), qApp, SLOT(addMapLayer()));
    m_pDeleteLayerAct = new QAction(tr("Remove layer"), this);
    connect(m_pAboutQtAct, SIGNAL(triggered()), qApp, SLOT(removeMapLayer()));
}

bool MainWindow::createDatastore()
{
    // Check if datastore exists
    std::string path = ngsFormFileName(ngsGetCurrentDirectory(), "tmp", nullptr);
    std::string catalogPath = ngsCatalogPathFromSystem(path.c_str());
    std::string storeName("main.ngst");
    m_storePath = catalogPath + "/" + storeName;

    if(ngsCatalogObjectGet(m_storePath.c_str()) == nullptr) {
        char** options = nullptr;
        options = ngsAddNameValue(
                    options, "TYPE", std::to_string(
                        ngsCatalogObjectType::CAT_CONTAINER_NGS).c_str());
        options = ngsAddNameValue(options, "CREATE_UNIQUE", "ON");

        return ngsCatalogObjectCreate(catalogPath.c_str(), storeName.c_str(),
                                      options) == ngsErrorCodes::EC_SUCCESS;
    }
    return true;
}

void MainWindow::createMenus()
{
    QMenu *pFileMenu = menuBar()->addMenu(tr("&File"));
    pFileMenu->addAction(m_pNewAct);
    pFileMenu->addAction(m_pOpenAct);
    pFileMenu->addAction(m_pSaveAct);
    pFileMenu->addSeparator();
    pFileMenu->addAction(m_pExitAct);

    QMenu *pDataMenu = menuBar()->addMenu(tr("&Data"));
    pDataMenu->addAction(m_pLoadAct);

    QMenu *pMapMenu = menuBar()->addMenu(tr("&Map"));
    pMapMenu->addAction(m_pAddLayerAct);
    pMapMenu->addAction(m_pDeleteLayerAct);

    QMenu *pHelpMenu = menuBar()->addMenu(tr("&Help"));
    pHelpMenu->addAction(m_pAboutAct);
    pHelpMenu->addAction(m_pAboutQtAct);
}
