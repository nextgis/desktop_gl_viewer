/******************************************************************************
*  Project: NextGIS GL Viewer
*  Purpose: GUI viewer for spatial data.
*  Author:  Dmitry Baryshnikov, bishop.dev@gmail.com
*******************************************************************************
*  Copyright (C) 2018-2019 NextGIS, <info@nextgis.com>
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
#ifndef CREATENGWCONNECTIONDIALOG_H
#define CREATENGWCONNECTIONDIALOG_H

#include <QDialog>

namespace Ui {
class CreateNGWConnectionDialog;
}

class CreateNGWConnectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateNGWConnectionDialog(QWidget *parent = nullptr);
    ~CreateNGWConnectionDialog();
    QString name() const;
    QString url() const;
    QString login() const;
    QString password() const;
    bool isGuest() const;

private slots:
    void on_checkConnectionButton_clicked();

    void on_loginAsGuest_stateChanged(int arg1);

private:
    Ui::CreateNGWConnectionDialog *ui;
    QString m_login, m_password;

    // QDialog interface
public slots:
    virtual void accept() override;
};

#endif // CREATENGWCONNECTIONDIALOG_H
