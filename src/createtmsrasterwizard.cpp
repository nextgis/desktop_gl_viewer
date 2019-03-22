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

#include "createtmsfinishwizardpage.h"
#include "createtmsrasterwizard.h"
#include "qmswizardpage.h"

#include <QIcon>

CreateTMSRasterWizard::CreateTMSRasterWizard(QWidget *parent) : QWizard(parent)
{
    QPixmap pixmap = QIcon(":/images/raster.svg").pixmap(QSize(48, 48));
    setPixmap(QWizard::LogoPixmap, pixmap);
    setWindowTitle(tr("Create TMS raster"));
    setWizardStyle(WizardStyle::ModernStyle);

    setPage(PAGE_QMS, new QMSWizardPage);
    setPage(PAGE_FINISH, new CreateTMSFinishWizardPage);

    setStartId(PAGE_QMS);
}

QString CreateTMSRasterWizard::name() const
{
    return property("name").toString();
}

QString CreateTMSRasterWizard::path() const
{
    return property("path").toString();
}

QString CreateTMSRasterWizard::url() const
{
    return property("url").toString();
}

int CreateTMSRasterWizard::epsg() const
{
    return property("epsg").toInt();
}

int CreateTMSRasterWizard::z_min() const
{
    return property("z_min").toInt();
}

int CreateTMSRasterWizard::z_max() const
{
    return property("z_max").toInt();
}
