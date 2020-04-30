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
#include "createngwconnectiondialog.h"
#include "ui_catalogdialog.h"

#include <QInputDialog>
#include <QMenu>
#include <QPushButton>

CatalogDialog::CatalogDialog(Type type, const QString &title, int filter,
                             QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CatalogDialog),
    m_type(type)
{
    // set model
    m_model = new CatalogModel(filter);
    init(title);
}

CatalogDialog::CatalogDialog(CatalogDialog::Type type, const QString& title,
                             const QVector<int>& filter, QWidget* parent) :
    QDialog(parent),
    ui(new Ui::CatalogDialog),
    m_type(type)
{
    // set model
    m_model = new CatalogModel(filter);
    init(title);
}

CatalogDialog::~CatalogDialog()
{
    delete m_model;
    delete ui;
}

void CatalogDialog::init(const QString &title)
{
    ui->setupUi(this);
    setWindowTitle(title);
    ui->treeView->setModel(m_model);
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    connect(ui->treeView->selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection&,const QItemSelection&)),
            this,
            SLOT(selectionChanged(const QItemSelection&,const QItemSelection&)));
    connect(ui->treeView, SIGNAL(customContextMenuRequested(QPoint)), this,
            SLOT(showContextMenu(QPoint)));
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
    return ui->lineEdit->text().toStdString();
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

void CatalogDialog::showContextMenu(const QPoint &pos)
{
    QModelIndex selected = ui->treeView->indexAt(pos);

    // Create menu and insert some actions
    QMenu contextMenu;

    QMenu contextNewMenu(tr("New"));

    if(m_model->canCreateObject(selected, CAT_NGW_GROUP)) {
        QAction *action = contextNewMenu.addAction("NextGIS Web group", this,
                                                   &CatalogDialog::createNGWGroup);
        action->setData(selected);
    }

    if(m_model->canCreateObject(selected, CAT_NGW_TRACKERGROUP)) {
        QAction *action = contextNewMenu.addAction("NextGIS Web tracker group",
                                                   this,
                                                   &CatalogDialog::createNGWTrackerGroup);
        action->setData(selected);
    }

    if(m_model->canCreateObject(selected, CAT_CONTAINER_NGW)) {
        QAction *action = contextNewMenu.addAction("NextGIS Web connection",
                                                   this,
                                                   &CatalogDialog::createNGWConnection);
        action->setData(selected);
    }

    contextMenu.addMenu(&contextNewMenu);

    // Handle global position
    QPoint globalPos = ui->treeView->mapToGlobal(pos);

    // Show context menu at handling position
    contextMenu.exec(globalPos);
}

void CatalogDialog::createNGWGroup()
{
    QObject *obj = sender();
    QAction *action = qobject_cast<QAction*>(obj);
    if(action) {
        QModelIndex index = action->data().toModelIndex();
        if(index.isValid()) {
            bool ok;
            QString name = QInputDialog::getText(this, tr("Set new resource group name"),
                                                     tr("Resource group name:"), QLineEdit::Normal,
                                                     tr("New resource group"), &ok);
            if (ok && !name.isEmpty()) {
                QMap<std::string, std::string> options;
                options["CREATE_UNIQUE"] = "ON";
                if(!m_model->createObject(index, name, CAT_NGW_GROUP, options)) {
                    QString message = QString(tr("Create resource group failed.\nError: %1")).arg(
                                ngsGetLastErrorMessage());
                }
            }
        }
    }
}

void CatalogDialog::createNGWTrackerGroup()
{
    QObject *obj = sender();
    QAction *action = qobject_cast<QAction*>(obj);
    if(action) {
        QModelIndex index = action->data().toModelIndex();
        if(index.isValid()) {
            bool ok;
            QString name = QInputDialog::getText(this, tr("Set new tracker group name"),
                                                     tr("Tracker group name:"), QLineEdit::Normal,
                                                     tr("New tracker group"), &ok);
            if (ok && !name.isEmpty()) {
                QMap<std::string, std::string> options;
                options["CREATE_UNIQUE"] = "ON";
                if(!m_model->createObject(index, name, CAT_NGW_TRACKERGROUP, options)) {
                    QString message = QString(tr("Create tracker group failed.\nError: %1")).arg(
                                ngsGetLastErrorMessage());
                }
            }
        }
    }
}

void CatalogDialog::createNGWConnection()
{
    QObject *obj = sender();
    QAction *action = qobject_cast<QAction*>(obj);
    if(action) {
        QModelIndex index = action->data().toModelIndex();
        if(index.isValid()) {
            CreateNGWConnectionDialog dlg(this);
            if(dlg.exec() == QDialog::Accepted) {
                QMap<std::string, std::string> options;
                options["login"] = dlg.login().toStdString();
                options["password"] = dlg.password().toStdString();
                options["url"] = dlg.url().toStdString();
                options["is_guest"] = dlg.isGuest() ? "ON" : "OFF";
                options["CREATE_UNIQUE"] = "ON";
                if(!m_model->createObject(index, dlg.name(), CAT_CONTAINER_NGW, options)) {
                    QString message = QString(tr("Create connection failed.\nError: %1")).arg(
                                ngsGetLastErrorMessage());
                }
            }
        }
    }
}
