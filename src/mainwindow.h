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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QActionGroup>
#include <QtConcurrent/QtConcurrent>
#include <QListView>
#include <QMainWindow>
#include <QSplitter>

#include "eventsstatus.h"
#include "locationstatus.h"
#include "mapmodel.h"
#include "glmapview.h"
#include "progressdialog.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);

public slots:
    void about();
    void save();
    void open();
    void newFile();
    void load();
    void createOverviews();
    void undoEdit();
    void redoEdit();
    void saveEdit();
    void cancelEdit();
    void createNewGeometry();
    void editSelectedGeometry();
    void deleteGeometry();
    void addPoint();
    void deletePoint();
    void addHole();
    void deleteHole();
    void addGeometryPart();
    void deleteGeometryPart();
    void addMapLayer();
    void removeMapLayer();
    void loadFinished();
    void showContextMenu(const QPoint &pos);
    void setStatusText(const QString &text, int timeout = 0);
    void statusBarShowHide();
    void identifyMode();
    void panMode();
    void zoomInMode();
    void zoomOutMode();
    void createStore();

protected:
    virtual void closeEvent(QCloseEvent *event) override;
    void writeSettings();
    void readSettings();
    void createMenus();
    void createActions();
    void createDockWindows();
    bool createDatastore();

private:
    QAction* m_newAct;
    QAction* m_openAct;
    QAction* m_saveAct;
    QAction* m_aboutAct;
    QAction* m_loadAct;
    QAction* m_createStore;
    QAction* m_createOverviewsAct;
    QAction* m_undoEditAct;
    QAction* m_redoEditAct;
    QAction* m_saveEditAct;
    QAction* m_cancelEditAct;
    QAction* m_createNewGeometryAct;
    QAction* m_editSelectedGeometryAct;
    QAction* m_deleteGeometryAct;
    QAction* m_addPointAct;
    QAction* m_deletePointAct;
    QAction* m_addHoleAct;
    QAction* m_deleteHoleAct;
    QAction* m_addGeometryPartAct;
    QAction* m_deleteGeometryPartAct;
    QAction* m_aboutQtAct;
    QAction* m_exitAct;
    QAction* m_addLayerAct;
    QAction* m_pDeleteLayerAct;
    QAction* m_statusBarAct;
    QAction* m_identify;
    QAction* m_pan;
    QAction* m_zoomIn;
    QAction* m_zoomOut;
    QActionGroup* m_mapGroup;

    ProgressDialog *m_progressDlg;
    QSplitter *m_splitter;

private:
    QFutureWatcher<int> m_watcher;
    EventsStatus *m_eventsStatus;
    LocationStatus *m_locationStatus;
    GlMapView *m_mapView;
    QListView* m_mapLayersView;
    MapModel *m_mapModel;
};

#endif // MAINWINDOW_H
