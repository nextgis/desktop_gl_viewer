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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace ngv {

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

protected:
    void closeEvent(QCloseEvent *event);
    void writeSettings();
    void readSettings();
    void createMenus();
    void createActions();

private:
    QAction *m_pNewAct;
    QAction *m_pOpenAct;
    QAction *m_pSaveAct;
    QAction *m_pAboutAct;
    QAction *m_pUploadAct;
    QAction *m_pAboutQtAct;
    QAction *m_pExitAct;
};

}

#endif // MAINWINDOW_H
