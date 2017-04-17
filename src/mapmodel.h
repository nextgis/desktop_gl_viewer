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
#ifndef MAPMODEL_H
#define MAPMODEL_H

#include <QAbstractItemModel>

constexpr const char * DEFAULT_MAP_NAME = "default";
constexpr const char * DEFAULT_MAP_DESCRIPTION = "default map";
constexpr unsigned short DEFAULT_EPSG = 3857;
constexpr double DEFAULT_MAX_X = 20037508.34; // 180.0
constexpr double DEFAULT_MAX_Y = 20037508.34; // 90.0
constexpr double DEFAULT_MIN_X = -DEFAULT_MAX_X;
constexpr double DEFAULT_MIN_Y = -DEFAULT_MAX_Y;

//#define MIN_OFF_PX 2
//#define TM_ZOOMING 200
//#define YORIENT 0

class MapModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit MapModel(QObject *parent = 0);
    virtual ~MapModel();

    bool create(const char* name = DEFAULT_MAP_NAME,
                const char* description = DEFAULT_MAP_DESCRIPTION,
                unsigned short epsg = DEFAULT_EPSG,
                double minX = DEFAULT_MIN_X,
                double minY = DEFAULT_MIN_Y,
                double maxX = DEFAULT_MAX_X,
                double maxY = DEFAULT_MAX_Y);
    bool open(const char* path);
    bool isValid() const { return m_mapId != 0; }

    // Basic functionality:
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // Add data:
    bool insertRows(int row, int count,
                    const QModelIndex &parent = QModelIndex()) override;

    // Remove data:
    bool removeRows(int row, int count,
                    const QModelIndex &parent = QModelIndex()) override;

    unsigned char mapId() const;

private:
    unsigned char m_mapId;
};

#endif // MAPMODEL_H
