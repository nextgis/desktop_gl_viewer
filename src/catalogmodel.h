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
#ifndef CATALOGMODEL_H
#define CATALOGMODEL_H

#include <QAbstractItemModel>

class CatalogItem
{
public:
    CatalogItem(const QList<QVariant> &data, CatalogItem *parent = 0);
    ~CatalogItem() { qDeleteAll(childItems); }

    void appendChild(CatalogItem *child) { childItems.append(child); }

    CatalogItem *child(int row) { return childItems.value(row); }
    int childCount() const;
    int columnCount() const { return itemData.count(); }
    QVariant data(int column) const { return itemData.value(column); }
    int row() const;
    CatalogItem *parent() { return parentItem; }

private:
    QList<CatalogItem*> childItems;
    QList<QVariant> itemData;
    CatalogItem *parentItem;
};

class CatalogModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit CatalogModel(QObject *parent = 0);
    ~CatalogModel() { delete rootItem; }

    // Basic functionality:
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

private:
    CatalogItem *rootItem;
};

#endif // CATALOGMODEL_H
