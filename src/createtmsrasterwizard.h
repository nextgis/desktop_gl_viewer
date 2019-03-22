/******************************************************************************
*  Project: NextGIS GL Viewer
*  Purpose: GUI viewer for spatial data.
*  Author:  Dmitry Baryshnikov, bishop.dev@gmail.com
*******************************************************************************
*  Copyright (C) 2016-2018 NextGIS, <info@nextgis.com>
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

#ifndef CREATETMSRASTERWIZARD_H
#define CREATETMSRASTERWIZARD_H

#include <QWizard>

class CreateTMSRasterWizard : public QWizard
{
    Q_OBJECT
public:
    enum { PAGE_QMS, PAGE_FINISH };
public:
    CreateTMSRasterWizard(QWidget *parent = Q_NULLPTR);
    QString name() const;
    QString path() const;
    QString url() const;
    int epsg() const;
    int z_min() const;
    int z_max() const;
};

#endif // CREATETMSRASTERWIZARD_H
