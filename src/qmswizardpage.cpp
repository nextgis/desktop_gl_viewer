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
#include "qmswizardpage.h"
#include "ui_qmswizardpage.h"

#include "ngstore/api.h"

#include <QMessageBox>
#include <QMovie>
#include <QPainter>
#include <QTimer>

//------------------------------------------------------------------------------
// QMSWizardPage
//------------------------------------------------------------------------------
QMSWizardPage::QMSWizardPage(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::QMSWizardPage)
{
    ui->setupUi(this);

    m_timer = new QTimer(this);

    setTitle(tr("Select geoservice"));
    setSubTitle(tr("Select geoservice to save as raster."));

    m_model = new QMSItemModel;
    connect(m_model, &QMSItemModel::beginFetchMore,
            this, &QMSWizardPage::onBeginFetchMore);
    connect(m_model, &QMSItemModel::endFetchMore,
            this, &QMSWizardPage::onEndFetchMore);
    connect(ui->filterEdit, &QLineEdit::textChanged,
            this, &QMSWizardPage::onFilterChanged);
    connect(m_timer, &QTimer::timeout, this, &QMSWizardPage::onTimer);

    ui->listView->setModel(m_model);
    ui->listView->setItemDelegate(new ListDelegate(ui->listView));

    m_movie = new QMovie(":/animation/loader.gif");

    if(!m_movie->isValid()) {
        m_movie->setScaledSize(QSize(24,24));
    }
    else {
        delete m_movie;
        m_movie = Q_NULLPTR;
        ui->loadAnimation->setPixmap(QPixmap(":/images/refresh.svg"));
        ui->loadAnimation->hide();
    }
}

QMSWizardPage::~QMSWizardPage()
{
    delete ui;
}

bool QMSWizardPage::validatePage()
{
    QModelIndex index = ui->listView->currentIndex();
    if(!index.isValid()) {
        QMessageBox::critical(this, tr("Error"), tr("You mast select geoservice from the list."));
        return false;
    }

    QVariant id = m_model->data(index, Qt::UserRole + 2);
    wizard()->setProperty("qms_id", id);
    return true;
}

void QMSWizardPage::onBeginFetchMore()
{
    if(m_movie) {
        ui->loadAnimation->setMovie(m_movie);
        m_movie->start();
    }
    else {
        ui->loadAnimation->show();
    }
}

void QMSWizardPage::onEndFetchMore()
{
    if(m_movie) {
        m_movie->stop();
        ui->loadAnimation->setMovie(Q_NULLPTR);
    }
    else {
        ui->loadAnimation->hide();
    }
}

void QMSWizardPage::onFilterChanged(const QString &value)
{
    m_timer->setProperty("value", value);
    m_timer->start(900);
}

void QMSWizardPage::onTimer()
{
    m_model->setFilter(m_timer->property("value").toString());
    m_timer->stop();
}

//------------------------------------------------------------------------------
// QMSItemModel
//------------------------------------------------------------------------------

constexpr int FETCH_COUNT = 15;

static QIcon getIconForUrl(const QString &url, const QIcon &defaultIcon = QIcon())
{
    ngsURLRequestResult *result = ngsURLRequest(ngsURLRequestType::URT_GET,
                                                url.toStdString().c_str(),
                                                nullptr, nullptr, nullptr);
    QByteArray byteArray(reinterpret_cast<const char *>(result->data), result->dataLen);

    QPixmap img;
    img.loadFromData(byteArray);

    ngsURLRequestResultFree(result);
    QIcon ico(img);
    if(ico.isNull()) {
        return defaultIcon;
    }
    return ico;
}

static int fillItemsArray(int offset, const QString &filter, std::vector<QMSItemModel::QMSItem> &array, const QIcon &defaultIcon)
{
    char **options = nullptr;
    if(!filter.isEmpty()) {
        options = ngsListAddNameValue(options, "search", filter.toStdString().c_str());
    }
    options = ngsListAddNameValue(options, "type", "tms");
    options = ngsListAddNameValue(options, "epsg", "3857");
    options = ngsListAddNameValue(options, "limit", std::to_string(FETCH_COUNT).c_str());
    options = ngsListAddNameValue(options, "offset", std::to_string(offset).c_str());
    int counter(0);
    ngsQMSItem *items = ngsQMSQuery(options);
    ngsListFree(options);
    while(items[counter].id != -1) {
        QString iconUrlStr(items[counter].iconUrl);
        QIcon icon;
        if(iconUrlStr.isEmpty()) {
            icon = defaultIcon;
        }
        else {
            icon = getIconForUrl(iconUrlStr, defaultIcon);
        }
        array.push_back({items[counter].id, items[counter].name,
                         items[counter].desc, icon});
        counter++;
    }
    ngsFree(items);
    return offset + counter;
}

QMSItemModel::QMSItemModel(QObject *parent) : QAbstractItemModel(parent),
    m_currentOffset(0),
    m_canFetchMore(true)
{
}

void QMSItemModel::setFilter(const QString &value)
{
    QString filter;

    if(value.size() < 2 && m_prevSearch.size() > 2) {
        // empty search
        filter = "";
        m_canFetchMore = true;
        m_currentOffset = 0;
        m_items.clear();
    }
    else if(value.size() > 2) {
        filter = value;
        m_canFetchMore = true;
        m_currentOffset = 0;
        m_items.clear();
    }
    else {
        return;
    }

    emit beginFetchMore();
    beginResetModel();
    int newOffset = fillItemsArray(m_currentOffset, filter, m_items, m_defaultIcon);

    if(newOffset == m_currentOffset) {
        m_canFetchMore = false;
    }
    else {
        m_currentOffset = newOffset;
    }

    endResetModel();
    emit endFetchMore();

    m_prevSearch = value;
}

QVariant QMSItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if(role == Qt::UserRole + 1) {
        return m_items[reinterpret_cast<size_t>(index.internalPointer())].desc;
    }

    if(role == Qt::UserRole + 2) {
        return m_items[reinterpret_cast<size_t>(index.internalPointer())].id;
    }

    if(role == Qt::DecorationRole) {
        return m_items[reinterpret_cast<size_t>(index.internalPointer())].icon;
    }

    if (role == Qt::DisplayRole) {
        return m_items[reinterpret_cast<size_t>(index.internalPointer())].name;
    }

    return QVariant();
}

void QMSItemModel::fetchMore(const QModelIndex &parent)
{
    Q_UNUSED(parent);
    emit beginFetchMore();

    if(m_defaultIcon.isNull()) {
        m_defaultIcon = getIconForUrl("https://qms.nextgis.com/api/v1/icons/default");
    }

    QString filter = m_prevSearch.size() > 2 ? m_prevSearch : "";

    int newOffset = fillItemsArray(m_currentOffset, filter, m_items, m_defaultIcon);
    if(newOffset == m_currentOffset) {
        m_canFetchMore = false;
    }
    else {
        beginInsertRows(QModelIndex(), m_currentOffset, newOffset);
        m_currentOffset = newOffset;
        endInsertRows();
    }
    emit endFetchMore();
}

bool QMSItemModel::canFetchMore(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_canFetchMore;
}

QModelIndex QMSItemModel::index(int row, int column, const QModelIndex &parent) const
{
    if(!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, row);
}

QModelIndex QMSItemModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child);
    return QModelIndex();
}

int QMSItemModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return static_cast<int>(m_items.size());
}

int QMSItemModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

//------------------------------------------------------------------------------
// ListDelegate
//------------------------------------------------------------------------------

ListDelegate::ListDelegate(QObject *parent) : QAbstractItemDelegate(parent)
{

}

void ListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const {
    if(option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.color(QPalette::Highlight));
    }

    QString title = index.data(Qt::DisplayRole).toString();
    QString description = index.data(Qt::UserRole + 1).toString();
    QIcon ic = index.data(Qt::DecorationRole).value<QIcon>();

    QRect r = option.rect;

    int imageSpace = 10;
    if (!ic.isNull()) {
        r = option.rect.adjusted(5, 10, -5, -10);
        ic.paint(painter, r, Qt::AlignTop|Qt::AlignLeft);
        imageSpace = 55;
    }

    QFont font = painter->font();
    font.setBold(true);
    painter->setFont(font);
    r = option.rect.adjusted(imageSpace, 5, -10, -35);
    painter->drawText(r.left(), r.top(), r.width(), r.height(), Qt::AlignTop|Qt::AlignLeft|Qt::TextWordWrap, title, &r);

    font.setBold(false);
    painter->setFont(font);
    r = option.rect.adjusted(imageSpace, 35, -10, -5);
    painter->drawText(r.left(), r.top(), r.width(), r.height(), Qt::AlignTop|Qt::AlignLeft|Qt::TextWordWrap, description, &r);
}

QSize ListDelegate::sizeHint(const QStyleOptionViewItem & option,
                             const QModelIndex & index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(200, 70); // very dumb value
}

ListDelegate::~ListDelegate()
{

}
