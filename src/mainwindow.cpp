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
#include "createtmsrasterwizard.h"
#include "loginmynextgiscomdialog.h"
#include "version.h"

constexpr unsigned char maxRecentFiles = 5;

// progress function
int loadProgressFunction(enum ngsCode /*status*/,
                          double complete,
                          const char * /*message*/,
                          void *progressArguments)
{
    ProgressDialog *dlg = static_cast<ProgressDialog*>(progressArguments);
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
    m_progressDlg(nullptr),
    m_recentSeparator(nullptr)
{
    setWindowIcon(QIcon(":/images/main_logo.svg"));

    QString config, cache;
#ifdef Q_OS_MACOS
    config = QLatin1String("Library/Application Support");
    cache = QLatin1String("Library/Caches");
#else
    config = QLatin1String(".config");
    cache = QLatin1String(".cache");
#endif

    QString configDir = QString( "%1%2glviewer" )
                .arg( QDir::homePath() + QDir::separator() + config + QDir::separator())
                .arg( QDir::separator() + QLatin1String(VENDOR) + QDir::separator() );
    QString cacheDir = QString( "%1%2ngstd" )
                .arg( QDir::homePath() + QDir::separator() + cache + QDir::separator())
                .arg( QDir::separator() + QLatin1String(VENDOR) + QDir::separator() );

    char **options = nullptr;
    options = ngsListAddNameValue(options, "DEBUG_MODE", "ON");
    options = ngsListAddNameValue(options, "SETTINGS_DIR", configDir.toLatin1().data());
    options = ngsListAddNameValue(options, "CACHE_DIR", cacheDir.toLatin1().data());
    options = ngsListAddNameValue(options, "GDAL_DATA",
                              qgetenv("GDAL_DATA").constData());
    options = ngsListAddNameValue(options, "NUM_THREADS", "ALL_CPUS");
    options = ngsListAddNameValue(options, "GL_MULTISAMPLE", "OFF");
//    const char *key = ngsGeneratePrivateKey();
    options = ngsListAddNameValue(options, "CRYPT_KEY", "2904dc49c0957fff9236f043c5f1c74688d64f2b5007babe9d3c0aed391b9df7");
    int result = ngsInit(options);

    ngsListFree(options);

    if(result == COD_SUCCESS) {
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

        createActions();
        createMenus();
        createDockWindows();
        readSettings();

        // Add recent menus
        foreach (QAction *action, menuBar()->actions()) {
            if(action->menu() && action->text() == tr("&File")) {
                QMenu *fileMenu = action->menu();
                QList<QAction*> actions = fileMenu->actions();
                QAction *lastItem = actions.last();
                fileMenu->insertActions(lastItem, recentFileActs);
                m_recentSeparator = fileMenu->insertSeparator(lastItem);
                m_recentSeparator->setVisible(false);
                break;
            }
        }

        /*m_eventsStatus->addMessage ();
        m_eventsStatus->addWarning ();
        m_eventsStatus->addError ();*/


        connect(&m_watcher, SIGNAL(finished()), this, SLOT(loadFinished()));

    }
    else {
        QMessageBox::critical(this, tr("Error"), tr("Library initialize failed"));
    }
}

void MainWindow::updateRecentFileActions()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("MainWindow"));
    QStringList files = settings.value("recentFileList").toStringList();

    int numRecentFiles = qMin(files.size(), static_cast<int>(maxRecentFiles));

    for (int i = 0; i < numRecentFiles; ++i) {
        QFileInfo info(files[i]);
        QString text = tr("&%1 %2").arg(i + 1).arg(info.fileName());
        recentFileActs[i]->setText(text);
        recentFileActs[i]->setData(files[i]);
        recentFileActs[i]->setVisible(true);
    }

    for (int j = numRecentFiles; j < maxRecentFiles; ++j) {
        recentFileActs[j]->setVisible(false);
    }

    if(m_recentSeparator)
        m_recentSeparator->setVisible(numRecentFiles > 0);
}

void MainWindow::addRecentFile(const QString &fileName)
{
    QSettings settings;
    settings.beginGroup(QLatin1String("MainWindow"));
    QStringList files = settings.value("recentFileList").toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > maxRecentFiles) {
        files.removeLast();
    }

    settings.setValue("recentFileList", files);
    settings.endGroup();

    updateRecentFileActions();
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
    if(m_mapModel) {
        delete m_mapModel;
    }
    m_mapModel = nullptr;
    ngsUnInit();
    event->accept();
}

void MainWindow::writeSettings()
{
    QSettings settings;

    settings.beginGroup("MainWindow");
    if(isMaximized()) {
        settings.setValue("frame_maximized", true);
    }
    else {
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
    if(settings.value("frame_maximized", false).toBool()) {
        showMaximized();
    }
    else {
        resize(settings.value("frame_size", QSize(400, 400)).toSize());
        move(settings.value("frame_pos", QPoint(200, 200)).toPoint());
    }
    restoreState(settings.value("frame_state").toByteArray());
    statusBar()->setVisible(settings.value("frame_statusbar_shown", true).toBool());
    m_statusBarAct->setChecked(statusBar()->isVisible());
    m_splitter->restoreState(settings.value("splitter_sizes").toByteArray());
    settings.endGroup();

    updateRecentFileActions();
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

            addRecentFile(openPath.c_str());
        }
    }
}

void MainWindow::save()
{
    CatalogDialog dlg(CatalogDialog::SAVE, tr("Save map as..."),
                      ngsCatalogObjectType::CAT_FILE_NGMAPDOCUMENT, this);
    int result = dlg.exec();

    if(1 == result) {
        std::string savePath = dlg.getCatalogPath() + "/" + dlg.getNewName();

        if(ngsMapSave(m_mapModel->mapId(), savePath.c_str()) != COD_SUCCESS) {
            QMessageBox::critical (this, tr("Error"), tr("Map save failed"));
        }
        else {
            statusBar()->showMessage(tr("Map saved"), 10000); // time limit 10 sec.
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
        std::string name = dlg.getNewName();

        CatalogDialog dlgDst(CatalogDialog::OPEN, tr("Select destination"),
                          ngsCatalogObjectType::CAT_CONTAINER_NGS, this);

        result = dlgDst.exec();
        if(1 == result) {
            std::string dstPath = dlgDst.getCatalogPath();

            // 2. Show progress dialog
            m_progressDlg = new ProgressDialog(tr("Loading ..."), this);

            char **options = nullptr;
            options = ngsListAddNameValue(options, "NEW_NAME", name.c_str());
            options = ngsListAddNameValue(options, "FEATURES_SKIP", "EMPTY_GEOMETRY");
            options = ngsListAddNameValue(options, "FORCE", "ON");
            options = ngsListAddNameValue(options, "CREATE_OVERVIEWS", "ON");
            options = ngsListAddNameValue(options, "ZOOM_LEVELS", "1,2,3,4,5,6,7,8,9,10,11,12,13,14"); // ,15,16,17,18

            ngsProgressFunc func = loadProgressFunction;


            CatalogObjectH shapeFile = ngsCatalogObjectGet(shapePath.c_str());
            CatalogObjectH store = ngsCatalogObjectGet(dstPath.c_str());
            QFuture<int> future = QtConcurrent::run(ngsCatalogObjectCopy,
                                                    shapeFile,
                                                    store,
                                                    options,
                                                    func,
                                                    static_cast<void*>(m_progressDlg));

            m_watcher.setFuture(future);

            m_progressDlg->open();

            ngsListFree(options);
        }
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
                                  "ZOOM_LEVELS=0,1,2,3,4,5,6,7,8,9,10,11,12,13,14", // ,15,16,17,18
                                  nullptr};
        char **popt = const_cast<char**>(options);
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

void MainWindow::createNewGeometryByWalk()
{
    QModelIndexList selection = m_mapLayersView->selectionModel()->selectedRows();
    for(const QModelIndex& index : selection) {
        m_mapModel->createNewGeometry(index, true);
        break;
    }
}

void MainWindow::editSelectedGeometry()
{
    m_mapModel->editSelectedGeometry();
}

void MainWindow::deleteGeometry()
{
    m_mapModel->deleteGeometry();
}

void MainWindow::addPoint()
{
    m_mapModel->addPoint();
}

void MainWindow::deletePoint()
{
    m_mapModel->deletePoint();
}

void MainWindow::addHole()
{
    m_mapModel->addHole();
}

void MainWindow::deleteHole()
{
    m_mapModel->deleteHole();
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
    m_newAct = new QAction(QIcon(":/images/doc_new.svg"), tr("&New"), this);
    m_newAct->setShortcuts(QKeySequence::New);
    m_newAct->setStatusTip(tr("Create a new map document"));
    connect(m_newAct, &QAction::triggered, this, &MainWindow::newFile);

    m_openAct = new QAction(QIcon(":/images/doc_open.svg"), tr("&Open..."), this);
    m_openAct->setShortcuts(QKeySequence::Open);
    m_openAct->setStatusTip(tr("Open an existing map document"));
    connect(m_openAct, &QAction::triggered, this, &MainWindow::open);

    m_saveAct = new QAction(QIcon(":/images/doc_save.svg"), tr("&Save"), this);
    m_saveAct->setShortcuts(QKeySequence::Save);
    m_saveAct->setStatusTip(tr("Save the map document to disk"));
    connect(m_saveAct, &QAction::triggered, this, &MainWindow::save);

    m_createStore = new QAction(tr("&Create store"), this);
    m_createStore->setStatusTip(tr("Create NextGIS storage"));
    connect(m_createStore, &QAction::triggered, this, &MainWindow::createStore);

    m_createTMS = new QAction(tr("&Create TMS"), this);
    m_createTMS->setStatusTip(tr("Create TMS raster"));
    connect(m_createTMS, &QAction::triggered, this, &MainWindow::createTMS);

    m_loadAct = new QAction(tr("&Load"), this);
    m_loadAct->setStatusTip(tr("Load spatial data to internal storage"));
    connect(m_loadAct, &QAction::triggered, this, &MainWindow::load);

    m_createOverviewsAct = new QAction(tr("Create vector overviews"), this);
    m_createOverviewsAct->setStatusTip(tr("Create vector layer overviews"));
    connect(m_createOverviewsAct, &QAction::triggered, this, &MainWindow::createOverviews);

    m_undoEditAct = new QAction(tr("Undo editing"), this);
    m_undoEditAct->setStatusTip(tr("Undo editing"));
    connect(m_undoEditAct, &QAction::triggered, this, &MainWindow::undoEdit);

    m_redoEditAct = new QAction(tr("Redo editing"), this);
    m_redoEditAct->setStatusTip(tr("Redo editing"));
    connect(m_redoEditAct, &QAction::triggered, this, &MainWindow::redoEdit);

    m_saveEditAct = new QAction(tr("Save editing"), this);
    m_saveEditAct->setStatusTip(tr("Save editing to layer"));
    connect(m_saveEditAct, &QAction::triggered, this, &MainWindow::saveEdit);

    m_cancelEditAct = new QAction(tr("Cancel editing"), this);
    m_cancelEditAct->setStatusTip(tr("Cancel editing"));
    connect(m_cancelEditAct, &QAction::triggered, this, &MainWindow::cancelEdit);

    m_createNewGeometryAct = new QAction(tr("Create new geometry"), this);
    m_createNewGeometryAct->setStatusTip(tr("Create new geometry in selected layer"));
    connect(m_createNewGeometryAct, &QAction::triggered, this, &MainWindow::createNewGeometry);

    m_createNewGeometryByWalkAct = new QAction(tr("Create new geometry by walk"), this);
    m_createNewGeometryByWalkAct->setStatusTip(tr("Create new geometry by walk in selected layer"));
    connect(m_createNewGeometryByWalkAct, &QAction::triggered, this, &MainWindow::createNewGeometryByWalk);

    m_editSelectedGeometryAct = new QAction(tr("Edit selected geometry"), this);
    m_editSelectedGeometryAct->setStatusTip(tr("Edit selected geometry"));
    connect(m_editSelectedGeometryAct, &QAction::triggered, this, &MainWindow::editSelectedGeometry);

    m_deleteGeometryAct = new QAction(tr("Delete edited geometry"), this);
    m_deleteGeometryAct->setStatusTip(tr("Delete edited geometry with saving"));
    connect(m_deleteGeometryAct, &QAction::triggered, this, &MainWindow::deleteGeometry);

    m_addPointAct = new QAction(tr("Add point"), this);
    m_addPointAct->setStatusTip(tr("Add point to line"));
    connect(m_addPointAct, &QAction::triggered, this, &MainWindow::addPoint);

    m_deletePointAct = new QAction(tr("Delete point"), this);
    m_deletePointAct->setStatusTip(tr("Delete point in line"));
    connect(m_deletePointAct, &QAction::triggered, this, &MainWindow::deletePoint);

    m_addHoleAct = new QAction(tr("Add hole"), this);
    m_addHoleAct->setStatusTip(tr("Add hole to polygon"));
    connect(m_addHoleAct, &QAction::triggered, this, &MainWindow::addHole);

    m_deleteHoleAct = new QAction(tr("Delete hole"), this);
    m_deleteHoleAct->setStatusTip(tr("Delete hole in polygon"));
    connect(m_deleteHoleAct, &QAction::triggered, this, &MainWindow::deleteHole);

    m_addGeometryPartAct = new QAction(tr("Add geometry part"), this);
    m_addGeometryPartAct->setStatusTip(tr("Add part to multi geometry"));
    connect(m_addGeometryPartAct, &QAction::triggered, this, &MainWindow::addGeometryPart);

    m_deleteGeometryPartAct = new QAction(tr("Delete geometry part"), this);
    m_deleteGeometryPartAct->setStatusTip(tr("Delete part from multi geometry"));
    connect(m_deleteGeometryPartAct, &QAction::triggered, this, &MainWindow::deleteGeometryPart);

    m_addLayerAct = new QAction(tr("Add layer"), this);
    m_addLayerAct->setStatusTip(tr("Add new layer to map"));
    connect(m_addLayerAct, &QAction::triggered, this, &MainWindow::addMapLayer);

    m_exitAct = new QAction(tr("E&xit"), this);
    m_exitAct->setShortcuts(QKeySequence::Quit);
    m_exitAct->setStatusTip(tr("Exit the application"));
    connect(m_exitAct, &QAction::triggered, this, &MainWindow::close);

    m_aboutAct = new QAction(QIcon(":/images/main_logo.svg"), tr("&About"), this);
    m_aboutAct->setStatusTip(tr("Show the application's About box"));
    m_aboutAct->setMenuRole(QAction::AboutRole);
    connect(m_aboutAct, &QAction::triggered, this, &MainWindow::about);

    m_aboutQtAct = new QAction(tr("About &Qt"), this);
    m_aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    m_aboutQtAct->setMenuRole(QAction::AboutQtRole);
    connect(m_aboutQtAct, &QAction::triggered, qApp, &QApplication::aboutQt);

    m_statusBarAct = new QAction(tr("Status bar"), this);
    m_statusBarAct->setStatusTip(tr("Show/hide status bar"));
    m_statusBarAct->setCheckable(true);
    m_statusBarAct->setChecked(statusBar()->isVisible());
    connect(m_statusBarAct, &QAction::triggered, this, &MainWindow::statusBarShowHide);

    m_identify = new QAction(tr("Identify"), this);
    m_identify->setStatusTip(tr("Identify features"));
    m_identify->setCheckable(true);
    connect(m_identify, &QAction::triggered, this, &MainWindow::identifyMode);

    m_pan = new QAction(tr("Pan"), this);
    m_pan->setStatusTip(tr("Pan map"));
    m_pan->setCheckable(true);
    connect(m_pan, &QAction::triggered, this, &MainWindow::panMode);

    m_zoomIn = new QAction(tr("Zoom in"), this);
    m_zoomIn->setStatusTip(tr("Zoom in map"));
    m_zoomIn->setCheckable(true);
    connect(m_zoomIn, &QAction::triggered, this, &MainWindow::zoomInMode);

    m_zoomOut = new QAction(tr("Zoom out"), this);
    m_zoomOut->setStatusTip(tr("Zoom out map"));
    m_zoomOut->setCheckable(true);
    connect(m_zoomOut, &QAction::triggered, this, &MainWindow::zoomOutMode);

    m_loginMyNextGISCom = new QAction(tr("Login to my.nextgis.com"), this);
    m_loginMyNextGISCom->setStatusTip(tr("Login to my.nextgis.com"));
    connect(m_loginMyNextGISCom, &QAction::triggered, this, &MainWindow::loginMyNextGISCom);

    m_createTracker = new QAction(tr("Create new tracker"), this);
    m_createTracker->setStatusTip(tr("Create new tracker in web GIS"));
    connect(m_createTracker, &QAction::triggered, this, &MainWindow::createTracker);

    m_mapGroup = new QActionGroup(this);
    m_mapGroup->addAction(m_pan);
    m_mapGroup->addAction(m_identify);
    m_mapGroup->addAction(m_zoomIn);
    m_mapGroup->addAction(m_zoomOut);
    m_pan->setChecked(true);

    for (int i = 0; i < maxRecentFiles; ++i) {
        QAction *recentAction = new QAction(this);
        recentAction->setVisible(false);
        connect(recentAction, &QAction::triggered,
                this, &MainWindow::onOpenRecentFile);
        recentFileActs << recentAction;
    }
}

bool MainWindow::createDatastore()
{
    // Check if datastore exists
    std::string path = ngsFormFileName(ngsGetCurrentDirectory(), "tmp", nullptr);
    std::string catalogPath = ngsCatalogPathFromSystem(path.c_str());
    std::string storeName("main.ngst");
    std::string storePath = catalogPath + "/" + storeName;

    if(ngsCatalogObjectGet(storePath.c_str()) == nullptr) {
        char **options = nullptr;
        options = ngsListAddNameValue(
                    options, "TYPE", std::to_string(
                        ngsCatalogObjectType::CAT_CONTAINER_NGS).c_str());
        options = ngsListAddNameValue(options, "CREATE_UNIQUE", "ON");

        CatalogObjectH storeDir = ngsCatalogObjectGet(catalogPath.c_str());
        return ngsCatalogObjectCreate(storeDir, storeName.c_str(),
                                      options) == COD_SUCCESS;
    }
    return true;
}

void MainWindow::createMenus()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(m_newAct);
    fileMenu->addAction(m_openAct);
    fileMenu->addAction(m_saveAct);
    fileMenu->addSeparator();
    fileMenu->addAction(m_exitAct);

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(m_undoEditAct);
    editMenu->addAction(m_redoEditAct);
    editMenu->addSeparator();
    editMenu->addAction(m_saveEditAct);
    editMenu->addAction(m_cancelEditAct);
    editMenu->addSeparator();
    editMenu->addAction(m_createNewGeometryAct);
    editMenu->addAction(m_createNewGeometryByWalkAct);
    editMenu->addAction(m_editSelectedGeometryAct);
    editMenu->addSeparator();
    editMenu->addAction(m_deleteGeometryAct);
    editMenu->addSeparator();
    editMenu->addAction(m_addPointAct);
    editMenu->addAction(m_deletePointAct);
    editMenu->addAction(m_addHoleAct);
    editMenu->addAction(m_deleteHoleAct);
    editMenu->addAction(m_addGeometryPartAct);
    editMenu->addAction(m_deleteGeometryPartAct);
    editMenu->addSeparator();
    editMenu->addAction(m_createOverviewsAct);

    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(m_statusBarAct);
//  refresh

    QMenu *dataMenu = menuBar()->addMenu(tr("&Data"));
    dataMenu->addAction(m_createTMS);
    dataMenu->addAction(m_createStore);
    dataMenu->addAction(m_loadAct);
    dataMenu->addAction(m_createOverviewsAct);
    dataMenu->addSeparator();
    dataMenu->addAction(m_createTracker);

    QMenu *mapMenu = menuBar()->addMenu(tr("&Map"));
    mapMenu->addAction(m_addLayerAct);
    mapMenu->addSeparator();
    mapMenu->addAction(m_identify);
    mapMenu->addAction(m_pan);
    mapMenu->addAction(m_zoomIn);
    mapMenu->addAction(m_zoomOut);
    // prev extent
    // next extent

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(m_aboutAct);
    helpMenu->addAction(m_aboutQtAct);
    helpMenu->addAction(m_loginMyNextGISCom);
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
    myMenu.addAction("Remove", this, &MainWindow::removeMapLayer);

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

void MainWindow::createStore()
{
    CatalogDialog dlg(CatalogDialog::SAVE, tr("Select path and name"),
                      ngsCatalogObjectType::CAT_CONTAINER_DIR, this);
    int result = dlg.exec();

    if(QDialog::Accepted == result) {
        std::string storePath = dlg.getCatalogPath();

        CatalogObjectH storeDir = ngsCatalogObjectGet(storePath.c_str());
        std::string newName = dlg.getNewName();

        char **options = nullptr;
        options = ngsListAddNameValue(options, "TYPE", std::to_string(CAT_CONTAINER_NGS).c_str());
        options = ngsListAddNameValue(options, "CREATE_UNIQUE", "ON");
        ngsCatalogObjectCreate(storeDir, newName.c_str(), options);
        ngsListFree(options);
    }
}

void MainWindow::createTMS()
{
    CreateTMSRasterWizard wizard(this);
    if(wizard.exec() == QDialog::Accepted) {
        QString name = wizard.name();
        if(!name.endsWith(".wconn")) {
            name.append(".wconn");
        }
        QString path = wizard.path();
        QString url = wizard.url();
        int epsg = wizard.epsg();
        int z_min = wizard.z_min();
        int z_max = wizard.z_max();

        CatalogObjectH catalog = ngsCatalogObjectGet(path.toStdString().c_str());

        char **options = nullptr;
        options = ngsListAddNameValue(options, "TYPE",
                                      std::to_string(ngsCatalogObjectType::CAT_RASTER_TMS).c_str());
        options = ngsListAddNameValue(options, "CREATE_UNIQUE", "ON");
        options = ngsListAddNameValue(options, "url", url.toStdString().c_str());
        options = ngsListAddNameValue(options, "epsg", std::to_string(epsg).c_str());
        options = ngsListAddNameValue(options, "z_min", std::to_string(z_min).c_str());
        options = ngsListAddNameValue(options, "z_max", std::to_string(z_max).c_str());
        options = ngsListAddNameValue(options, "cache_expires", "1209600");

        if(ngsCatalogObjectCreate(catalog, name.toStdString().c_str(),
                                  options) != COD_SUCCESS) {
            QMessageBox::critical(this, tr("Error"), tr("Failed to create TMS") +
                                  ": " + ngsGetLastErrorMessage());
        }
    }
}

void MainWindow::onOpenRecentFile()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (action) {
        if(!m_mapModel->open(action->data().toString().toStdString().c_str())) {
            QMessageBox::critical(this, tr("Error"), tr("Map load failed"));
        }
    }
}

void MainWindow::loginMyNextGISCom()
{
    LoginMyNextGISComDialog dlg(this);
    dlg.exec();
}

void MainWindow::createTracker()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::information(this, tr("Tracker ID"),
                                     tr("Your tracker ID is: <i>%1</i><p>"
        "Add this tracker ID to your Web GIS or send this ID to Web GIS administrator.<p>"
        "Do you want to create tracker in Web GIS?<p>"
        "<small>You must be Web GIS administrator or have permissions to create trackers.</small>")
          .arg(ngsGetDeviceId(false)), QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        const QVector<int> filter = {
            ngsCatalogObjectType::CAT_CONTAINER_GISCONNECTIONS,
            ngsCatalogObjectType::CAT_CONTAINER_NGW,
            ngsCatalogObjectType::CAT_CONTAINER_NGWGROUP,
            ngsCatalogObjectType::CAT_CONTAINER_NGWTRACKERGROUP };
        CatalogDialog dlg(CatalogDialog::SAVE, tr("Select location and tracker name"),
                          filter, this);
        int result = dlg.exec();
        if(QDialog::Accepted == result) {
            std::string trackersPath = dlg.getCatalogPath();

            CatalogObjectH storeDir = ngsCatalogObjectGet(trackersPath.c_str());
            std::string newName = dlg.getNewName();

            char **options = nullptr;
            options = ngsListAddNameValue(options, "TYPE", std::to_string(CAT_NGW_TRACKER).c_str());
            options = ngsListAddNameValue(options, "CREATE_UNIQUE", "ON");
            options = ngsListAddNameValue(options, "TRACKER_ID", ngsGetDeviceId(false));
            if(ngsCatalogObjectCreate(storeDir, newName.c_str(), options) !=
                    COD_SUCCESS) {
                QMessageBox::critical(this, tr("Error"),
                                      tr("Failed to create tracker.\nError: %1").arg(ngsGetDeviceId(false)));
            }
            else {
                QMessageBox::information(this, tr("Success"), tr("Tracker created successfully."));
            }
            ngsListFree(options);
        }
    }
}
