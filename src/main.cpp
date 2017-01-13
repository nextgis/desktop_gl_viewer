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
#include <QApplication>
#include <QSurfaceFormat>
#include "version.h"

using namespace ngv;

int main(int argc, char *argv[])
{
    //Q_INIT_RESOURCE(glview);

    QApplication app(argc, argv);

    app.setOrganizationName("NextGIS");
    app.setApplicationDisplayName ("NextGIS GL Viewer");
    app.setApplicationName("glviewer");
    app.setApplicationVersion(NGGLV_VERSION_STRING);
    app.setOrganizationDomain("nextgis.com");

    // gl stuff
    QSurfaceFormat format;
#ifdef Q_OS_MACOS
    format.setRenderableType (QSurfaceFormat::OpenGL);
#else
    //    format.setDepthBufferSize(16);
    //    format.setStencilBufferSize(8);
        format.setVersion(2, 0);
    //    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setRenderableType (QSurfaceFormat::OpenGLES);
#endif
    QSurfaceFormat::setDefaultFormat(format);

    // create window
    MainWindow wnd;
    wnd.show();
    
    return app.exec ();
}

