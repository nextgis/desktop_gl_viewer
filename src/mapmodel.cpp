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
#include "mapmodel.h"

MapModel::MapModel(QObject *parent)
    : QAbstractItemModel(parent), m_mapId(0)
{
}

MapModel::~MapModel()
{
    if(isValid())
        ngsMapClose(m_mapId);
}

bool MapModel::create(const char *name, const char *description,
                      unsigned short epsg, double minX, double minY,
                      double maxX, double maxY)
{
    beginResetModel();
    if(isValid())
        ngsMapClose(m_mapId);
    m_mapId = ngsMapCreate(name, description, epsg, minX, minY, maxX, maxY);
    endResetModel();
    return isValid();
}

bool MapModel::open(const char *path)
{
    beginResetModel();
    if(isValid())
        ngsMapClose(m_mapId);
    m_mapId = ngsMapOpen(path);
    endResetModel();

    return isValid();
}


QModelIndex MapModel::index(int row, int column, const QModelIndex &parent) const
{
    if(!hasIndex(row, column, parent))
        return QModelIndex();

    LayerH childItem = ngsMapLayerGet(m_mapId, row);
    if(nullptr != childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex MapModel::parent(const QModelIndex &/*index*/) const
{
    return QModelIndex(); // Plain list now
}

int MapModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return 0;

    return ngsMapLayerCount(m_mapId);
}

int MapModel::columnCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return 0;

    return 1; // Only layer name
}

QVariant MapModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    LayerH layer = static_cast<LayerH>(index.internalPointer());
    return ngsLayerGetName(layer);
}

bool MapModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (data(index, role) != value) {
        LayerH layer = static_cast<LayerH>(index.internalPointer());
        if(ngsLayerSetName(layer, value.toString().toUtf8()) !=
                ngsErrorCode::EC_SUCCESS)
            return false;
        emit dataChanged(index, index, QVector<int>() << role);
        return true;
    }
    return false;
}

Qt::ItemFlags MapModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = Qt::ItemIsEditable | Qt::ItemIsEnabled |
            Qt::ItemIsSelectable;

    if (index.isValid())
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
    else
        return Qt::ItemIsDropEnabled | defaultFlags;
}

bool MapModel::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent, row, row + count - 1);
    // FIXME: Implement me!
    endInsertRows();

    return false;
}

bool MapModel::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row + count - 1);
    // FIXME: Implement me!
    endRemoveRows();

    return false;
}

unsigned char MapModel::mapId() const
{
    return m_mapId;
}

void MapModel::setSize(int w, int h, bool YAxisInverted)
{
    if(0 == m_mapId)
        return;
    ngsMapSetSize(m_mapId, w, h, YAxisInverted ? 1 : 0);
}

void MapModel::draw(ngsDrawState state, ngsProgressFunc callback,
                       void *callbackData)
{
    if(0 == m_mapId)
        return;
    ngsMapDraw(m_mapId, state, callback, callbackData);
}

void MapModel::setBackground(const ngsRGBA &color)
{
    if(0 == m_mapId)
        return;
    ngsMapSetBackgroundColor(m_mapId, color);
}

ngsCoordinate MapModel::getCenter()
{
    if(0 == m_mapId)
        return {0, 0, 0};
    return ngsMapGetCenter(m_mapId);
}
