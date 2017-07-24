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

#include <QDataStream>
#include <QMimeData>

constexpr const char* MIME = "application/vnd.map.layer";

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
    setOverlayVisible(MOT_EDIT, true); // FIXME: for test, remove it
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

int MapModel::rowCount(const QModelIndex &/*parent*/) const
{
    return ngsMapLayerCount(m_mapId);
}

int MapModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 1; // Only layer name
}

QVariant MapModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    LayerH layer = static_cast<LayerH>(index.internalPointer());
    return ngsLayerGetName(layer);
}

bool MapModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (data(index, role) != value) {
        LayerH layer = static_cast<LayerH>(index.internalPointer());
        if(ngsLayerSetName(layer, value.toString().toUtf8()) !=
                ngsCode::COD_SUCCESS)
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

unsigned char MapModel::mapId() const
{
    return m_mapId;
}

void MapModel::setSize(int w, int h, bool YAxisInverted)
{
    if(0 == m_mapId)
        return;
    ngsMapSetSize(m_mapId, w, h, YAxisInverted ? 1 : 0);
    ngsMapSetExtentLimits(m_mapId, -20037508.34, -20037508.34, 20037508.34, 20037508.34);
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

ngsCoordinate MapModel::getCenter() const
{
    if(0 == m_mapId)
        return {0, 0, 0};
    return ngsMapGetCenter(m_mapId);
}

bool MapModel::setCenter(const ngsCoordinate &newCenter)
{
    if(0 == m_mapId)
        return false;
    return ngsMapSetCenter(m_mapId, newCenter.X, newCenter.Y);
}

ngsCoordinate MapModel::getCoordinate(int x, int y) const
{
    if(0 == m_mapId)
        return {0, 0, 0};
    return ngsMapGetCoordinate(m_mapId, static_cast<double>(x),
                               static_cast<double>(y));
}

ngsCoordinate MapModel::getDistance(const QPoint &pt) const
{
    if(0 == m_mapId)
        return {0, 0, 0};
    return ngsMapGetDistance(m_mapId, pt.x(), pt.y());
}

double MapModel::getRotate(ngsDirection dir) const
{
    if(0 == m_mapId)
        return 0.0;
    return ngsMapGetRotate(m_mapId, dir);
}

bool MapModel::setRotate(ngsDirection dir, double value)
{
    if(0 == m_mapId)
        return false;
    return ngsMapSetRotate(m_mapId, dir, value) == ngsCode::COD_SUCCESS;
}

double MapModel::getScale() const
{
    if(0 == m_mapId)
        return 1.0;
    return ngsMapGetScale(m_mapId);
}

bool MapModel::setScale(double value)
{
    if(0 == m_mapId)
        return false;
    return ngsMapSetScale(m_mapId, value);
}

void MapModel::createLayer(const char *name, const char *path)
{
    if(0 == m_mapId)
        return;
    int result = ngsMapCreateLayer(m_mapId, name, path);
    if(-1 != result) {
        beginInsertRows(QModelIndex(), result, result);
        insertRow(result);
        endInsertRows();
    }
}

void MapModel::deleteLayer(const QModelIndex &index)
{
    if(0 == m_mapId)
        return;
    LayerH layer = static_cast<LayerH>(index.internalPointer());
    if(ngsMapLayerDelete(m_mapId, layer) == ngsCode::COD_SUCCESS) {
        beginRemoveRows(index.parent(), index.row(), index.row());
        removeRow(index.row());
        endRemoveRows();
    }
}

QStringList MapModel::mimeTypes() const
{
    QStringList types;
    types << MIME;
    return types;
}

bool MapModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                            int row, int /*column*/, const QModelIndex &parent)
{
    if(0 == m_mapId)
        return false;

    if (action == Qt::IgnoreAction)
        return true;

    if (!data->hasFormat(MIME))
        return false;

    LayerH beforeLayer = parent.internalPointer();

    QByteArray encodedData = data->data(MIME);
    QDataStream stream(&encodedData, QIODevice::ReadOnly);

     while (!stream.atEnd()) {
         qint64 pointer;
         stream >> pointer;
         LayerH movedLayer = reinterpret_cast<LayerH>(pointer);

         bool result;
         beginResetModel();
         result = ngsMapLayerReorder(m_mapId, beforeLayer, movedLayer) ==
                              ngsCode::COD_SUCCESS;
         endResetModel();
         return result;
     }

    return false;
}


QMimeData *MapModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

     foreach (QModelIndex index, indexes) {
         if (index.isValid()) {
             qint64 pointer = reinterpret_cast<qint64>(index.internalPointer());
             stream << pointer;
         }
     }

     mimeData->setData(MIME, encodedData);
     return mimeData;
}

void MapModel::setOverlayVisible(ngsMapOverlyType typeMask, char visible)
{
    if (0 == m_mapId)
        return;
    ngsOverlaySetVisible(m_mapId, typeMask, visible);
}
