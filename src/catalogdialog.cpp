/******************************************************************************
 * Project: libngstore
 * Purpose: NextGIS store and visualization support library
 * Author:  Dmitry Baryshnikov, dmitry.baryshnikov@nextgis.com
 ******************************************************************************
 *   Copyright (c) 2016-2017 NextGIS, <info@nextgis.com>
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include "catalogdialog.h"
#include "ui_catalogdialog.h"

#include <QPushButton>

#include "catalogmodel.h"

CatalogDialog::CatalogDialog(const QString &title, int filter, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CatalogDialog)
{
    ui->setupUi(this);
    setWindowTitle(title);

    // set model
    ui->treeView->setModel(new CatalogModel(filter));
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    connect(ui->treeView->selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection&,const QItemSelection&)),
            this,
            SLOT(selectionChanged(const QItemSelection&,const QItemSelection&)));
}

CatalogDialog::~CatalogDialog()
{
    delete ui;
}

std::string CatalogDialog::getCatalogPath()
{
    QModelIndex index = ui->treeView->currentIndex();
    CatalogItem *item = static_cast<CatalogItem*>(index.internalPointer());
    if(nullptr != item) {
        return item->getPath();
    }
    return "";
}

std::string CatalogDialog::getNewName()
{
    return ui->lineEdit->text().toUtf8().constData();
}

void CatalogDialog::selectionChanged(const QItemSelection &/*selected*/,
                                     const QItemSelection &/*deselected*/)
{
    QModelIndex index = ui->treeView->currentIndex();
    CatalogItem *item = static_cast<CatalogItem*>(index.internalPointer());
    if(nullptr != item) {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
}
