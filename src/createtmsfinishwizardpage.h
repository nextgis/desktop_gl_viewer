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
#ifndef CREATETMSFINISHWIZARDPAGE_H
#define CREATETMSFINISHWIZARDPAGE_H

#include "catalogmodel.h"

#include <QWizardPage>

namespace Ui {
class CreateTMSFinishWizardPage;
}

class CreateTMSFinishWizardPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit CreateTMSFinishWizardPage(QWidget *parent = Q_NULLPTR);
    ~CreateTMSFinishWizardPage() override;

private:
    Ui::CreateTMSFinishWizardPage *ui;

    // QWizardPage interface
public:
    virtual void initializePage() override;
    virtual bool validatePage() override;

private:
    QString m_url;
    int m_EPSG;
    int m_zMin;
    int m_zMax;
    double m_minX, m_minY, m_maxX, m_maxY;
    CatalogModel *m_model;

};

#endif // CREATETMSFINISHWIZARDPAGE_H
