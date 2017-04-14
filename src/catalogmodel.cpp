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
#include "catalogmodel.h"

//------------------------------------------------------------------------------
// CatalogItem
//------------------------------------------------------------------------------

CatalogItem::CatalogItem(const QList<QVariant> &data, CatalogItem *parent)
{
    parentItem = parent;
    itemData = data;
}

int CatalogItem::childCount() const
{
    if(childItems.empty()) {

    }

    return childItems.count();
}

int CatalogItem::row() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<CatalogItem*>(this));

    return 0;
}

//------------------------------------------------------------------------------
// CatalogModel
//------------------------------------------------------------------------------

CatalogModel::CatalogModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    QList<QVariant> rootData;
    rootData << "Name" << "Type";
    rootItem = new CatalogItem(rootData);
//    catalorRootItem = new CatalogItem();
}

QModelIndex CatalogModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
            return QModelIndex();

    CatalogItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<CatalogItem*>(parent.internalPointer());

    CatalogItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex CatalogModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    CatalogItem *childItem = static_cast<CatalogItem*>(index.internalPointer());
    CatalogItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int CatalogModel::rowCount(const QModelIndex &parent) const
{
    CatalogItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<CatalogItem*>(parent.internalPointer());

    return parentItem->childCount();
}

int CatalogModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<CatalogItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}

QVariant CatalogModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
            return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    CatalogItem *item = static_cast<CatalogItem*>(index.internalPointer());

    return item->data(index.column());
}

Qt::ItemFlags CatalogModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant CatalogModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}
