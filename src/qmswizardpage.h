/******************************************************************************
*  Project: NextGIS GL Viewer
*  Purpose: GUI viewer for spatial data.
*  Author:  Dmitry Baryshnikov, bishop.dev@gmail.com
*******************************************************************************
*  Copyright (C) 2016-2018 NextGIS, <info@nextgis.com>
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 2 of the License, or
*   (at your option) any later version.
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#ifndef QMSWIZARDPAGE_H
#define QMSWIZARDPAGE_H

#include <QAbstractItemDelegate>
#include <QAbstractItemModel>
#include <QIcon>
#include <QWizardPage>

class QMSItemModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit QMSItemModel(QObject *parent = Q_NULLPTR);
    virtual ~QMSItemModel() override = default;
    void setFilter(const QString &value);

    // QAbstractItemModel interface
public:
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual void fetchMore(const QModelIndex &parent) override;
    virtual bool canFetchMore(const QModelIndex &parent) const override;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    virtual QModelIndex parent(const QModelIndex &child) const override;
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;

    struct QMSItem {
        int id;
        QString name;
        QString desc;
        QIcon icon;
    };

signals:
    void beginFetchMore();
    void endFetchMore();

private:
    std::vector<struct QMSItem> m_items;
    int m_currentOffset;
    bool m_canFetchMore;
    QIcon m_defaultIcon;
    QString m_prevSearch;
};

namespace Ui {
class QMSWizardPage;
}

class QMSWizardPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit QMSWizardPage(QWidget *parent = nullptr);
    ~QMSWizardPage() override;


    // QWizardPage interface
public:
    virtual bool validatePage() override;

private slots:
    void onBeginFetchMore();
    void onEndFetchMore();
    void onFilterChanged(const QString &value);
    void onTimer();

private:
    Ui::QMSWizardPage *ui;
    QMSItemModel *m_model;
    QMovie *m_movie;
    QTimer *m_timer;

};

class ListDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    ListDelegate(QObject *parent = Q_NULLPTR);

    void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
    QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const;

    virtual ~ListDelegate();

};

#endif // QMSWIZARDPAGE_H
