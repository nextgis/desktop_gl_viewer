/******************************************************************************
*  Project: NextGIS GL Viewer
*  Purpose: GUI viewer for spatial data.
*  Author:  Dmitry Baryshnikov, bishop.dev@gmail.com
*******************************************************************************
*  Copyright (C) 2016-2017 NextGIS, <info@nextgis.com>
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

// progress function
int loadProgressFunction(enum ngsCode /*status*/,
                          double complete,
                          const char* /*message*/,
                          void* progressArguments)
{
    ProgressDialog* dlg = static_cast<ProgressDialog*>(progressArguments);
    if(nullptr != dlg) {
        dlg->setProgress(static_cast<int>(complete * 100));
        if(dlg->isCancel()) {
            return 0;
        }
    }
    return 1;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_progressDlg(nullptr)
{
    setWindowIcon(QIcon(":/images/main_logo.svg"));

    char** options = nullptr;
    options = ngsAddNameValue(options, "DEBUG_MODE", "ON");
    options = ngsAddNameValue(options, "SETTINGS_DIR",
                              ngsFormFileName(ngsGetCurrentDirectory(), "tmp",
                                              nullptr));
    options = ngsAddNameValue(options, "GDAL_DATA",
                              qgetenv("GDAL_DATA").constData());
    options = ngsAddNameValue(options, "NUM_THREADS", "7");
    options = ngsAddNameValue(options, "GL_MULTISAMPLE", "ON");
    int result = ngsInit(options);

    ngsListFree(options);

    if(result == COD_SUCCESS && createDatastore()) {
        m_mapModel = new MapModel();
        // create empty map
        m_mapModel->create();

        // statusbar setup
        setStatusText(tr("Ready"), 30000); // time limit 30 sec.
        m_locationStatus = new LocationStatus;
        statusBar()->addPermanentWidget(m_locationStatus);

        m_eventsStatus = new EventsStatus;
        statusBar()->addPermanentWidget(m_eventsStatus);
        statusBar()->setStyleSheet("QStatusBar::item { border: none }"); // disable borders

        createActions ();
        createMenus();
        createDockWindows();
        readSettings();

        /*m_eventsStatus->addMessage ();
        m_eventsStatus->addWarning ();
        m_eventsStatus->addError ();*/


        connect(&m_watcher, SIGNAL(finished()), this, SLOT(loadFinished()));

    }
    else {
        QMessageBox::critical(this, tr("Error"), tr("Storage initialize failed"));
    }
}

void MainWindow::setStatusText(const QString &text, int timeout)
{
    statusBar()->showMessage(text, timeout);
}

void MainWindow::statusBarShowHide()
{
    statusBar()->setVisible(!statusBar()->isVisible());
    m_statusBarAct->setChecked(statusBar()->isVisible());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    if(m_mapModel)
        delete m_mapModel;
    m_mapModel = nullptr;
    ngsUnInit();
    event->accept();
}

void MainWindow::writeSettings()
{
    QSettings settings;

    settings.beginGroup("MainWindow");
    if(isMaximized()){
        settings.setValue("frame_maximized", true);
    }
    else{
        settings.setValue("frame_size", size());
        settings.setValue("frame_pos", pos());
    }
    settings.setValue("frame_state", saveState());
    settings.setValue("frame_statusbar_shown", statusBar()->isVisible());
    settings.setValue("splitter_sizes", m_splitter->saveState());
    settings.endGroup();
}

void MainWindow::readSettings()
{
    QSettings settings;

    settings.beginGroup("MainWindow");
    if(settings.value("frame_maximized", false).toBool()){
        showMaximized();
    }
    else{
        resize(settings.value("frame_size", QSize(400, 400)).toSize());
        move(settings.value("frame_pos", QPoint(200, 200)).toPoint());
    }
    restoreState(settings.value("frame_state").toByteArray());
    statusBar()->setVisible(settings.value("frame_statusbar_shown", true).toBool());
    m_statusBarAct->setChecked(statusBar()->isVisible());
    m_splitter->restoreState(settings.value("splitter_sizes").toByteArray());
    settings.endGroup();
}

void MainWindow::newFile()
{
    m_mapModel->create();
}

void MainWindow::open()
{
    CatalogDialog dlg(CatalogDialog::OPEN, tr("Open map"),
                      ngsCatalogObjectType::CAT_FILE_NGMAPDOCUMENT, this);
    int result = dlg.exec();

    if(1 == result) {
        std::string openPath = dlg.getCatalogPath().c_str();
        if(!m_mapModel->open(openPath.c_str())) {
            QMessageBox::critical(this, tr("Error"), tr("Map load failed"));
        }
        else {
            statusBar()->showMessage(tr("Map opened"), 10000); // time limit 10 sec.
        }
    }
}

void MainWindow::save()
{
    CatalogDialog dlg(CatalogDialog::SAVE, tr("Save map as..."),
                      ngsCatalogObjectType::CAT_FILE_NGMAPDOCUMENT, this);
    int result = dlg.exec();

    if(1 == result) {
        std::string savePath = dlg.getCatalogPath();

        if(ngsMapSave(m_mapModel->mapId(), savePath.c_str()) != COD_SUCCESS) {
            QMessageBox::critical (this, tr("Error"), tr("Map save failed"));
        }
        else {
            statusBar ()->showMessage(tr("Map saved"), 10000); // time limit 10 sec.
        }
    }
}

void MainWindow::load()
{
    // 1. Choose file dialog
    CatalogDialog dlg(CatalogDialog::OPEN, tr("Select data to load"),
                      ngsCatalogObjectType::CAT_FC_ANY, this);
    int result = dlg.exec();

    if(1 == result) {
        std::string shapePath = dlg.getCatalogPath();

        // 2. Show progress dialog
        m_progressDlg = new ProgressDialog(tr("Loading ..."), this);

        const char *options[2] = {"FEATURES_SKIP=EMPTY_GEOMETRY", nullptr};
        char** popt = const_cast<char**>(options);
        ngsProgressFunc func = loadProgressFunction;


        CatalogObjectH shapeFile = ngsCatalogObjectGet(shapePath.c_str());
        CatalogObjectH store = ngsCatalogObjectGet(m_storePath.c_str());
        QFuture<int> future = QtConcurrent::run(ngsCatalogObjectLoad,
                                                shapeFile,
                                                store,
                                                popt,
                                                func,
                                                static_cast<void*>(m_progressDlg));

        m_watcher.setFuture(future);

        m_progressDlg->open();
    }
}

void MainWindow::createOverviews()
{
    // Select feature class
    CatalogDialog dlg(CatalogDialog::OPEN, tr("Select feature class to create overviews"),
                      ngsCatalogObjectType::CAT_FC_ANY, this);
    int result = dlg.exec();

    if(1 == result) {
        std::string shapePath = dlg.getCatalogPath();

        // 2. Show progress dialog
        m_progressDlg = new ProgressDialog(tr("Proceeding ..."), this);

        const char *options[3] = {"FORCE=ON",
                                  "ZOOM_LEVELS=0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18",
                                  nullptr};
        char** popt = const_cast<char**>(options);
        ngsProgressFunc func = loadProgressFunction;


        CatalogObjectH shapeFile = ngsCatalogObjectGet(shapePath.c_str());
        // Run overview create
        QFuture<int> future = QtConcurrent::run(ngsFeatureClassCreateOverviews,
                                                shapeFile,
                                                popt,
                                                func,
                                                static_cast<void*>(m_progressDlg));

        m_watcher.setFuture(future);

        m_progressDlg->open();
    }
}

void MainWindow::undoEdit()
{
    m_mapModel->undoEdit();
}

void MainWindow::redoEdit()
{
    m_mapModel->redoEdit();
}

void MainWindow::saveEdit()
{
    m_mapModel->saveEdit();
}

void MainWindow::cancelEdit()
{
    m_mapModel->cancelEdit();
}

void MainWindow::createNewGeometry()
{
    QModelIndexList selection = m_mapLayersView->selectionModel()->selectedRows();
    for(const QModelIndex& index : selection) {
        m_mapModel->createNewGeometry(index);
        break;
    }
}

void MainWindow::editSelectedGeometry()
{
    m_mapModel->editSelectedGeometry();
}

void MainWindow::addGeometryPart()
{
    m_mapModel->addGeometryPart();
}

void MainWindow::deleteGeometryPart()
{
    m_mapModel->deleteGeometryPart();
}

void MainWindow::addMapLayer()
{
    // 1. Choose file dialog
    CatalogDialog dlg(CatalogDialog::OPEN, tr("Select data to add to map"),
                      ngsCatalogObjectType::CAT_RASTER_FC_ANY, this);
    int result = dlg.exec();

    if(1 == result) {
        std::string path = dlg.getCatalogPath();
        std::string name = "Layer " + std::to_string(m_mapModel->rowCount());
        m_mapModel->createLayer(name.c_str(), path.c_str());
    }
}

void MainWindow::removeMapLayer()
{
    QModelIndexList selection = m_mapLayersView->selectionModel()->selectedRows();
    for(const QModelIndex& index : selection) {
        m_mapModel->deleteLayer(index);
    }
}

void MainWindow::loadFinished()
{
    int result = m_watcher.result();
    if(result != COD_SUCCESS && result != COD_CANCELED) {
        QString message = QString(tr("Load to store failed.\nError: %1")).arg(
                    ngsGetLastErrorMessage());
        QMessageBox::critical(this, tr("Error"), message);
    }
    delete m_progressDlg;
    m_progressDlg = nullptr;
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

    m_createOverviewsAct = new QAction(tr("Create vector overviews"), this);
    m_createOverviewsAct->setStatusTip(tr("Create vector layer overviews"));
    connect(m_createOverviewsAct, SIGNAL(triggered()), this, SLOT(createOverviews()));

    m_undoEditAct = new QAction(tr("Undo editing"), this);
    m_undoEditAct->setStatusTip(tr("Undo editing"));
    connect(m_undoEditAct, SIGNAL(triggered()), this, SLOT(undoEdit()));

    m_redoEditAct = new QAction(tr("Redo editing"), this);
    m_redoEditAct->setStatusTip(tr("Redo editing"));
    connect(m_redoEditAct, SIGNAL(triggered()), this, SLOT(redoEdit()));

    m_saveEditAct = new QAction(tr("Save editing"), this);
    m_saveEditAct->setStatusTip(tr("Save editing to layer"));
    connect(m_saveEditAct, SIGNAL(triggered()), this, SLOT(saveEdit()));

    m_cancelEditAct = new QAction(tr("Cancel editing"), this);
    m_cancelEditAct->setStatusTip(tr("Cancel editing"));
    connect(m_cancelEditAct, SIGNAL(triggered()), this, SLOT(cancelEdit()));

    m_createNewGeometryAct = new QAction(tr("Create new geometry"), this);
    m_createNewGeometryAct->setStatusTip(tr("Create new geometry in selected layer"));
    connect(m_createNewGeometryAct, SIGNAL(triggered()), this, SLOT(createNewGeometry()));

    m_editSelectedGeometryAct = new QAction(tr("Edit selected geometry"), this);
    m_editSelectedGeometryAct->setStatusTip(tr("Edit selected geometry"));
    connect(m_editSelectedGeometryAct, SIGNAL(triggered()), this, SLOT(editSelectedGeometry()));

    m_addGeometryPartAct = new QAction(tr("Add geometry part"), this);
    m_addGeometryPartAct->setStatusTip(tr("Add part to multi geometry"));
    connect(m_addGeometryPartAct, SIGNAL(triggered()), this, SLOT(addGeometryPart()));

    m_deleteGeometryPartAct = new QAction(tr("Delete geometry part"), this);
    m_deleteGeometryPartAct->setStatusTip(tr("Delete part from multi geometry"));
    connect(m_deleteGeometryPartAct, SIGNAL(triggered()), this, SLOT(deleteGeometryPart()));

    m_pAddLayerAct = new QAction(tr("Add layer"), this);
    m_pAddLayerAct->setStatusTip(tr("Add new layer to map"));
    connect(m_pAddLayerAct, SIGNAL(triggered()), this, SLOT(addMapLayer()));

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

    m_statusBarAct = new QAction(tr("Status bar"), this);
    m_statusBarAct->setStatusTip(tr("Show/hide status bar"));
    m_statusBarAct->setCheckable(true);
    m_statusBarAct->setChecked(statusBar()->isVisible());
    connect(m_statusBarAct, SIGNAL(triggered()), this, SLOT(statusBarShowHide()));

    m_identify = new QAction(tr("Identify"), this);
    m_identify->setStatusTip(tr("Identify features"));
    m_identify->setCheckable(true);
    connect(m_identify, SIGNAL(triggered()), this, SLOT(identifyMode()));

    m_pan = new QAction(tr("Pan"), this);
    m_pan->setStatusTip(tr("Pan map"));
    m_pan->setCheckable(true);
    connect(m_pan, SIGNAL(triggered()), this, SLOT(panMode()));

    m_zoomIn = new QAction(tr("Zoom in"), this);
    m_zoomIn->setStatusTip(tr("Zoom in map"));
    m_zoomIn->setCheckable(true);
    connect(m_zoomIn, SIGNAL(triggered()), this, SLOT(zoomInMode()));

    m_zoomOut = new QAction(tr("Zoom out"), this);
    m_zoomOut->setStatusTip(tr("Zoom out map"));
    m_zoomOut->setCheckable(true);
    connect(m_zoomOut, SIGNAL(triggered()), this, SLOT(zoomOutMode()));


    m_mapGroup = new QActionGroup(this);
    m_mapGroup->addAction(m_pan);
    m_mapGroup->addAction(m_identify);
    m_mapGroup->addAction(m_zoomIn);
    m_mapGroup->addAction(m_zoomOut);
    m_pan->setChecked(true);
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

        CatalogObjectH storeDir = ngsCatalogObjectGet(catalogPath.c_str());
        return ngsCatalogObjectCreate(storeDir, storeName.c_str(),
                                      options) == COD_SUCCESS;
    }
    return true;
}

void MainWindow::createMenus()
{
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(m_pNewAct);
    fileMenu->addAction(m_pOpenAct);
    fileMenu->addAction(m_pSaveAct);
    fileMenu->addSeparator();
    fileMenu->addAction(m_pExitAct);

    QMenu* editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(m_undoEditAct);
    editMenu->addAction(m_redoEditAct);
    editMenu->addSeparator();
    editMenu->addAction(m_saveEditAct);
    editMenu->addAction(m_cancelEditAct);
    editMenu->addSeparator();
    editMenu->addAction(m_createNewGeometryAct);
    editMenu->addAction(m_editSelectedGeometryAct);
    editMenu->addSeparator();
    editMenu->addAction(m_addGeometryPartAct);
    editMenu->addAction(m_deleteGeometryPartAct);

    QMenu* viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(m_statusBarAct);
//  refresh

    QMenu* dataMenu = menuBar()->addMenu(tr("&Data"));
    dataMenu->addAction(m_pLoadAct);
    dataMenu->addAction(m_createOverviewsAct);

    QMenu* mapMenu = menuBar()->addMenu(tr("&Map"));
    mapMenu->addAction(m_pAddLayerAct);
    mapMenu->addSeparator();
    mapMenu->addAction(m_identify);
    mapMenu->addAction(m_pan);
    mapMenu->addAction(m_zoomIn);
    mapMenu->addAction(m_zoomOut);
    // prev extent
    // next extent

    QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(m_pAboutAct);
    helpMenu->addAction(m_pAboutQtAct);
}

void MainWindow::createDockWindows()
{
    m_splitter = new QSplitter(Qt::Horizontal);
    m_mapLayersView = new QListView();
    m_mapLayersView->setStyleSheet("QListView { border: none; }");
    m_mapLayersView->setAttribute(Qt::WA_MacShowFocusRect, false);
    m_mapLayersView->setDragEnabled(true);
    m_mapLayersView->setModel(m_mapModel);
    m_mapLayersView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_mapLayersView->setDragDropMode(QAbstractItemView::InternalMove);
    m_mapLayersView->setDefaultDropAction(Qt::MoveAction);
    //m_mapLayersView->setDropIndicatorShown(true);
    m_mapLayersView->setMovement(QListView::Snap);
    m_mapLayersView->setDragDropOverwriteMode(false);
    connect(m_mapLayersView, SIGNAL(customContextMenuRequested(QPoint)), this,
            SLOT(showContextMenu(QPoint)));


    m_splitter->addWidget(m_mapLayersView);

    // mapview setup
    m_mapView = new GlMapView(m_locationStatus, this);
    m_mapView->setModel(m_mapModel);
    m_mapView->setMode(GlMapView::M_PAN);

    m_splitter->addWidget(m_mapView);
    m_splitter->setHandleWidth(1);
    m_splitter->setStretchFactor(1, 3);

    setCentralWidget(m_splitter);
}

void MainWindow::showContextMenu(const QPoint &pos)
{
    // Handle global position
    QPoint globalPos = m_mapLayersView->mapToGlobal(pos);

    // Create menu and insert some actions
    QMenu myMenu;
    myMenu.addAction("Remove", this, SLOT(removeMapLayer()));

    // Show context menu at handling position
    myMenu.exec(globalPos);
}

void MainWindow::identifyMode()
{
    m_mapView->setMode(GlMapView::M_IDENTIFY);
}

void MainWindow::panMode()
{
    m_mapView->setMode(GlMapView::M_PAN);
}

void MainWindow::zoomInMode()
{
    m_mapView->setMode(GlMapView::M_ZOOMIN);
}

void MainWindow::zoomOutMode()
{
    m_mapView->setMode(GlMapView::M_ZOOMOUT);
}


