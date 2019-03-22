/******************************************************************************
*  Project: NextGIS GL viewer
*  Purpose: Application for prepare manuals and documantations using sphinx-doc
*           and rst fromat.
*  Author:  Dmitry Baryshnikov, bishop.dev@gmail.com
*******************************************************************************
*  Copyright (C) 2016 NextGIS, info@nextgis.ru
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

#ifndef VERSION_H
#define VERSION_H

#define APP_NAME "NG GL Viewer"
#define APP_COMMENT "Application for view geospatial data"
#define VENDOR "NextGIS"
#define VENDOR_DOMAIN "nextgis.com"

#define NGGLV_MAJOR_VERSION     0
#define NGGLV_MINOR_VERSION     5
#define NGGLV_PATCH_NUMBER      0

#define NGGLV_VERSION_NUMBER ( NGGLV_MAJOR_VERSION * 1000) + ( NGGLV_MINOR_VERSION \
    * 100) +  NGGLV_PATCH_NUMBER

#define STRINGIZE(x)  #x
#define MAKE_VERSION_DOT_STRING(x, y, z) STRINGIZE(x) "." STRINGIZE(y) "." \
    STRINGIZE(z)

#define NGGLV_VERSION_STRING MAKE_VERSION_DOT_STRING( NGGLV_MAJOR_VERSION, \
     NGGLV_MINOR_VERSION,  NGGLV_PATCH_NUMBER)

/*  check if the current version is at least major.minor.release */
#define NGGLV_CHECK_VERSION(major,minor,patch) \
    ( NGGLV_MAJOR_VERSION > (major) || \
    ( NGGLV_MAJOR_VERSION == (major) &&  NGGLV_MINOR_VERSION > (minor)) || \
    ( NGGLV_MAJOR_VERSION == (major) &&  NGGLV_MINOR_VERSION == (minor) && \
      NGGLV_PATCH_NUMBER >= (patch)))

#endif // VERSION_H
