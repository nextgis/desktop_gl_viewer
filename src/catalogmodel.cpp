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

#include <QtGlobal>

//------------------------------------------------------------------------------
// CatalogItem
//------------------------------------------------------------------------------

CatalogItem::CatalogItem(const std::string& name, enum ngsCatalogObjectType type,
                         int filter, CatalogItem *parent) :
    parentItem(parent),
    m_name(name),
    m_type(type),
    m_filter(filter)
{
}

int CatalogItem::childCount()
{
    if(m_type < ngsCatalogObjectType::CAT_CONTAINER_ANY ||
            m_type > ngsCatalogObjectType::CAT_CONTAINER_ALL) {
        return 0;
    }

    if(childItems.empty()) {
        // load children
        ngsCatalogObjectInfo* pathInfo;
        if(nullptr == parentItem) {
            pathInfo = ngsCatalogObjectQuery("ngc://", m_filter);
        }
        else {
            pathInfo = ngsCatalogObjectQuery(getPath().c_str(), m_filter);
        }
        if(nullptr != pathInfo) {
            int count = 0;
            while(pathInfo[count].name) {
                appendChild(new CatalogItem(pathInfo[count].name,
                                static_cast<enum ngsCatalogObjectType>(pathInfo[count].type),
                                m_filter, this));
                count++;
            }
            ngsFree(pathInfo);
        }
    }

    return childItems.count();
}

QVariant CatalogItem::data(int column) const
{
    if(nullptr == parentItem) {
        switch (column) {
        case 0:
            return QObject::tr("Name");
        case 1:
            return QObject::tr("Type");
        default:
            return QObject::tr("");
        }
    }
    switch (column) {
    case 0:
        return m_name.c_str();
    case 1:
        return CatalogItem::getTypeText(m_type).c_str();
    default:
        return QVariant();
    }
}

int CatalogItem::row() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<CatalogItem*>(this));

    return 0;
}

std::string CatalogItem::getPath() const
{
    if(nullptr == parentItem)
        return "ngc:/";
    return parentItem->getPath() + "/" + m_name;
}

std::string CatalogItem::getTypeText(enum ngsCatalogObjectType type)
{
    switch (type) {
    case ngsCatalogObjectType::CAT_CONTAINER_ROOT:
        return "Catalog";
    case ngsCatalogObjectType::CAT_CONTAINER_LOCALCONNECTION:
    case ngsCatalogObjectType::CAT_CONTAINER_WFS:
    case ngsCatalogObjectType::CAT_CONTAINER_POSTGRES:
    case ngsCatalogObjectType::CAT_CONTAINER_WMS:
    case ngsCatalogObjectType::CAT_CONTAINER_NGW:
        return "Connection";
    case ngsCatalogObjectType::CAT_CONTAINER_DIR:
    case ngsCatalogObjectType::CAT_CONTAINER_ARCHIVE_DIR:
        return "Directory";
    case ngsCatalogObjectType::CAT_CONTAINER_ARCHIVE:
    case ngsCatalogObjectType::CAT_CONTAINER_ARCHIVE_ZIP:
        return "Archive";
    case ngsCatalogObjectType::CAT_CONTAINER_GDB:
        return "GeoDataBase";
    case ngsCatalogObjectType::CAT_CONTAINER_GDB_SET:
        return "Dataset";
    case ngsCatalogObjectType::CAT_CONTAINER_POSTGRES_SCHEMA:
        return "Schema";
    case ngsCatalogObjectType::CAT_CONTAINER_NGS:
        return "NextGIS Datastore";
    case ngsCatalogObjectType::CAT_CONTAINER_KML:
    case ngsCatalogObjectType::CAT_CONTAINER_KMZ:
        return "KML File";
    case ngsCatalogObjectType::CAT_CONTAINER_SXF:
        return "SXF File";
    case ngsCatalogObjectType::CAT_CONTAINER_GPKG:
        return "GeoPackage";
    case ngsCatalogObjectType::CAT_FC_ESRI_SHAPEFILE:
        return "ESRI Shapefile";
    case ngsCatalogObjectType::CAT_FC_MAPINFO_TAB:
    case ngsCatalogObjectType::CAT_FC_MAPINFO_MIF:
        return "MapInfo file";
    case ngsCatalogObjectType::CAT_FC_DXF:
        return "AutoCAD File";
    case ngsCatalogObjectType::CAT_FC_POSTGIS:
    case ngsCatalogObjectType::CAT_FC_WFS:
    case ngsCatalogObjectType::CAT_FC_KMLKMZ:
    case ngsCatalogObjectType::CAT_FC_SXF:
    case ngsCatalogObjectType::CAT_FC_GDB:
    case ngsCatalogObjectType::CAT_FC_GPKG:
        return "Feature class";
    case ngsCatalogObjectType::CAT_FC_GML:
        return "GML File";
    case ngsCatalogObjectType::CAT_FC_GEOJSON:
        return "GeoJSON File";
    case ngsCatalogObjectType::CAT_FC_CSV:
        return "CSV File";
   case ngsCatalogObjectType::CAT_RASTER_BMP:
        return "Bitmap";
    case ngsCatalogObjectType::CAT_RASTER_TIFF:
        return "GeoTIFF";
    case ngsCatalogObjectType::CAT_RASTER_TIL:
        return "TIL File";
    case ngsCatalogObjectType::CAT_RASTER_IMG:
        return "Erdas imagine";
    case ngsCatalogObjectType::CAT_RASTER_JPEG:
        return "Jpeg File";
    case ngsCatalogObjectType::CAT_RASTER_PNG:
        return "PNG File";
    case ngsCatalogObjectType::CAT_RASTER_GIF:
        return "GIF File";
    case ngsCatalogObjectType::CAT_RASTER_SAGA:
        return "SAGA File";
    case ngsCatalogObjectType::CAT_RASTER_VRT:
        return "Virtual raster";
    case ngsCatalogObjectType::CAT_RASTER_WMS:
        return "WMS Layer";
    case ngsCatalogObjectType::CAT_RASTER_TMS:
        return "TMS raster";
    case ngsCatalogObjectType::CAT_RASTER_POSTGIS:
    case ngsCatalogObjectType::CAT_RASTER_GDB:
    case ngsCatalogObjectType::CAT_RASTER_GPKG:
        return "Raster";
    case ngsCatalogObjectType::CAT_TABLE_POSTGRES:
    case ngsCatalogObjectType::CAT_TABLE_MAPINFO_TAB:
    case ngsCatalogObjectType::CAT_TABLE_MAPINFO_MIF:
    case ngsCatalogObjectType::CAT_TABLE_CSV:
    case ngsCatalogObjectType::CAT_TABLE_GDB:
    case ngsCatalogObjectType::CAT_TABLE_DBF:
    case ngsCatalogObjectType::CAT_TABLE_GPKG:
        return "Table";
    case ngsCatalogObjectType::CAT_TABLE_XLS:
    case ngsCatalogObjectType::CAT_TABLE_XLSX:
    case ngsCatalogObjectType::CAT_TABLE_ODS:
        return "Spread sheet";
    case ngsCatalogObjectType::CAT_FILE_NGMAPDOCUMENT:
        return "NextGIS Map document";
    default:
        return "";
    }
}

//------------------------------------------------------------------------------
// CatalogModel
//------------------------------------------------------------------------------

CatalogModel::CatalogModel(int filter, QObject *parent)
    : QAbstractItemModel(parent)
{
    m_rootItem = new CatalogItem("", ngsCatalogObjectType::CAT_CONTAINER_ROOT, filter);
}

QModelIndex CatalogModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    CatalogItem *parentItem;

    if (!parent.isValid())
        parentItem = m_rootItem;
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

    if (parentItem == m_rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int CatalogModel::rowCount(const QModelIndex &parent) const
{
    CatalogItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<CatalogItem*>(parent.internalPointer());

    return parentItem->childCount();
}

int CatalogModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<CatalogItem*>(parent.internalPointer())->columnCount();
    else
        return m_rootItem->columnCount();
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
        return Qt::NoItemFlags;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant CatalogModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return m_rootItem->data(section);

    return QVariant();
}
