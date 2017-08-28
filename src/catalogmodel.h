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

#include "ngstore/api.h"

#include "QSharedPointer"

#include <QAbstractItemModel>

constexpr double BIG_VALUE = 100000000;

static bool isExtentInit(const ngsExtent &ext) {
    return ext.maxX < BIG_VALUE && ext.maxY < BIG_VALUE &&
            ext.minX > -BIG_VALUE && ext.minY > -BIG_VALUE;
}

static ngsExtent mergeExtent(const ngsExtent &ext1, const ngsExtent &ext2) {
    ngsExtent out;
    if(isExtentInit(ext1)) {
        out.minX = qMin(ext1.minX, ext2.minX);
        out.maxX = qMax(ext1.maxX, ext2.maxX);
        out.minY = qMin(ext1.minY, ext2.minY);
        out.maxY = qMax(ext1.maxY, ext2.maxY);
    }
    else {
        out.minX = ext2.minX;
        out.maxX = ext2.maxX;
        out.minY = ext2.minY;
        out.maxY = ext2.maxY;
    }
    return out;
}

class Geometry
{
public:
    Geometry(GeometryH handle, bool owns) : m_handle(handle), m_owns(owns) {}
    ~Geometry() { if(m_owns) ngsGeometryFree(m_handle); }
    ngsExtent envelope() const { return ngsGeometryGetEnvelope(m_handle); }
private:
    GeometryH m_handle;
    bool m_owns;
};
typedef QSharedPointer<Geometry> GeometryPtr;

class Feature
{
public:
    Feature(FeatureH handle) : m_handle(handle) {}
    ~Feature() { ngsFeatureFree(m_handle); }
    long long id() const { return ngsFeatureGetId(m_handle); }
    GeometryPtr geometry() const { return GeometryPtr(
                    new Geometry(ngsFeatureGetGeometry(m_handle), false));
                                 }
private:
    FeatureH m_handle;
};
typedef QSharedPointer<Feature> FeaturePtr;

class CatalogItem
{
public:
    CatalogItem(const std::string& name, enum ngsCatalogObjectType type,
                int filter = 0,
                CatalogItem *parent = 0);
    ~CatalogItem() { qDeleteAll(childItems); }

    void appendChild(CatalogItem *child) { childItems.append(child); }

    CatalogItem *child(int row) { return childItems.value(row); }
    int childCount();
    int columnCount() const { return 2; }
    QVariant data(int column) const;
    int row() const;
    CatalogItem *parent() { return parentItem; }
    std::string getPath() const;

    // static
public:
    static std::string getTypeText(enum ngsCatalogObjectType type);

private:
    QList<CatalogItem*> childItems;
    CatalogItem *parentItem;
    std::string m_name;
    enum ngsCatalogObjectType m_type;
    int m_filter;
};

class CatalogModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit CatalogModel(int filter = ngsCatalogObjectType::CAT_UNKNOWN,
                          QObject *parent = 0);
    ~CatalogModel() { delete m_rootItem; }

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
    CatalogItem *m_rootItem;
};

#endif // CATALOGMODEL_H
