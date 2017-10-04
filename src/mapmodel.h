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
#include <QPointF>
#include <QSet>
#include <QVector>

#include "ngstore/api.h"

#include "catalogmodel.h"

constexpr const char * DEFAULT_MAP_NAME = "default";
constexpr const char * DEFAULT_MAP_DESCRIPTION = "default map";
constexpr unsigned short DEFAULT_EPSG = 3857;
constexpr double DEFAULT_MAX_X = 20037508.34; // 180.0
constexpr double DEFAULT_MAX_Y = 20037508.34; // 90.0
constexpr double DEFAULT_MIN_X = -DEFAULT_MAX_X;
constexpr double DEFAULT_MIN_Y = -DEFAULT_MAX_Y;


class Layer
{
public:
    Layer() : m_handle(nullptr) {}
    explicit Layer(LayerH layerH) : m_handle(layerH) {}
    ~Layer() = default;
    LayerH handle() const { return  m_handle; }
    void setSelection(const QSet<long long> &ids);
    void emptyFeatureSet() { m_featureSet.empty(); }
    QVector<FeaturePtr> featureSet() const { return m_featureSet; }
    void addFeatureToSet(const FeaturePtr& feature) { m_featureSet.append(feature); }

private:
    LayerH m_handle;
    QVector<FeaturePtr> m_featureSet;
};

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

    Qt::DropActions supportedDropActions() const override {
        return Qt::MoveAction;
    }

    // Map functions
    unsigned char mapId() const;
    void setSize(int w, int h, bool YAxisInverted = true);
    void draw(enum ngsDrawState state, ngsProgressFunc callback,
                 void* callbackData);
    void invalidate(const ngsExtent& bounds);
    void setBackground(const ngsRGBA &color);
    ngsCoordinate getCenter() const;
    bool setCenter(const ngsCoordinate& newCenter);
    ngsCoordinate getCoordinate(int x, int y) const;
    ngsCoordinate getDistance(const QPoint& pt) const;
    double getRotate(enum ngsDirection dir) const;
    bool setRotate(enum ngsDirection dir, double value);
    double getScale() const;
    bool setScale(double value);
    void createLayer(const char *name, const char* path);
    void deleteLayer(const QModelIndex &index);
    void setOverlayVisible(int typeMask, char visible);
    void undoEdit();
    void redoEdit();
    bool canUndoEdit();
    bool canRedoEdit();
    void saveEdit();
    void cancelEdit();
    void createNewGeometry(const QModelIndex &index, bool walk = false);
    void editSelectedGeometry();
    void deleteGeometry();
    void addPoint(ngsCoordinate* coordinates = nullptr);
    void deletePoint();
    void addHole();
    void deleteHole();
    void addGeometryPart();
    void deleteGeometryPart();
    ngsPointId editOverlayTouch(double x, double y, const ngsMapTouchType type);
    void setSelectionStyle(const ngsRGBA &fillColor, const ngsRGBA &borderColor,
                           double width);
    QVector<Layer> identify(double minX, double minY,
                  double maxX, double maxY);
    bool isFeatureClass(enum ngsCatalogObjectType type) const;

signals:
    void undoEditFinished();
    void redoEditFinished();
    void editSaved();
    void editCanceled();
    void geometryCreated(const QModelIndex &index, bool walk);
    void geometryEditStarted();
    void geometryDeleted();
    void pointAdded();
    void pointDeleted();
    void holeAdded();
    void holeDeleted();
    void geometryPartAdded();
    void geometryPartDeleted();

private:
    unsigned char m_mapId;

    // QAbstractItemModel interface
public:
    virtual QStringList mimeTypes() const override;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                        int row, int column, const QModelIndex &parent) override;
    virtual QMimeData *mimeData(const QModelIndexList &indexes) const override;
};

#endif // MAPMODEL_H
