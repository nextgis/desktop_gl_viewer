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
#include "ui_createtmsfinishwizardpage.h"

#include "ngstore/api.h"

#include <QMessageBox>

CreateTMSFinishWizardPage::CreateTMSFinishWizardPage(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::CreateTMSFinishWizardPage)
{
    ui->setupUi(this);

    setTitle(tr("Input save path"));
    setSubTitle(tr("Select directory to save raster."));

    setFinalPage(true);

    m_model = new CatalogModel(ngsCatalogObjectType::CAT_CONTAINER_DIR);
    ui->catalogTreeView->setModel(m_model);

}

CreateTMSFinishWizardPage::~CreateTMSFinishWizardPage()
{
    delete ui;
}

void CreateTMSFinishWizardPage::initializePage()
{
    int qmsId = wizard()->property("qms_id").toInt();
    ngsQMSItemProperties properties = ngsQMSQueryProperties(qmsId);

    m_url = properties.url;
    m_EPSG = properties.EPSG;
    m_zMin = properties.z_min;
    m_zMax = properties.z_max;
    m_minX = properties.extent.minX;
    m_minY = properties.extent.minY;
    m_maxX = properties.extent.maxX;
    m_maxY = properties.extent.maxY;

    ui->rasterNameEdit->setText(QString(properties.name) + ".wconn");
}

bool CreateTMSFinishWizardPage::validatePage()
{
    QModelIndex index = ui->catalogTreeView->currentIndex();
    if(!index.isValid()) {
        QMessageBox::critical(this, tr("Error"), tr("You mast select output folder."));
        return false;
    }

    QString name = ui->rasterNameEdit->text();
    if(name.isEmpty()) {
        QMessageBox::critical(this, tr("Error"), tr("You mast enter a name."));
        return false;
    }

    CatalogItem *item = static_cast<CatalogItem*>(index.internalPointer());
    if(nullptr == item) {
        QMessageBox::critical(this, tr("Error"), tr("Invalid output folder."));
        return false;
    }

    std::string path = item->getPath();

    wizard()->setProperty("name", name);
    wizard()->setProperty("path", QString(path.c_str()));

    wizard()->setProperty("url", m_url);
    wizard()->setProperty("epsg", m_EPSG);
    wizard()->setProperty("z_min", m_zMin);
    wizard()->setProperty("z_max", m_zMax);
    wizard()->setProperty("min_x", m_minX);
    wizard()->setProperty("min_y", m_minY);
    wizard()->setProperty("max_x", m_maxX);
    wizard()->setProperty("max_y", m_maxY);

    return true;
}
