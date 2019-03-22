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
#include "createngwconnectiondialog.h"
#include "ui_createngwconnectiondialog.h"

#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>
#include <QMessageBox>
#include <QProgressDialog>

#include "ngstore/api.h"

CreateNGWConnectionDialog::CreateNGWConnectionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateNGWConnectionDialog)
{
    ui->setupUi(this);
}

CreateNGWConnectionDialog::~CreateNGWConnectionDialog()
{
    delete ui;
}

void CreateNGWConnectionDialog::on_checkConnectionButton_clicked()
{
    QProgressDialog dialog;

    QFutureWatcher<char> futureWatcher;
    connect(&futureWatcher, &QFutureWatcher<void>::finished, &dialog, &QProgressDialog::reset);
    connect(&dialog, &QProgressDialog::canceled, &futureWatcher, &QFutureWatcher<void>::cancel);
//    connect(&futureWatcher, &QFutureWatcher<void>::progressRangeChanged, &dialog, &QProgressDialog::setRange);
//    connect(&futureWatcher, &QFutureWatcher<void>::progressValueChanged, &dialog, &QProgressDialog::setValue);

    char **options = nullptr;
    options = ngsListAddNameValue(options, "url", url().toStdString().c_str());
    options = ngsListAddNameValue(options, "login", login().toStdString().c_str());
    options = ngsListAddNameValue(options, "password", password().toStdString().c_str());

    QFuture<char> future = QtConcurrent::run(ngsCatalogCheckConnection,
                                                        CAT_CONTAINER_NGW,
                                                        options);

    futureWatcher.setFuture(future);

    dialog.exec();

    futureWatcher.waitForFinished();
    ngsListFree(options);

    char result = futureWatcher.result();
    if(result != 1) {
        QString message = QString(tr("Connect failed.\nError: %1")).arg(
                    ngsGetLastErrorMessage());
        QMessageBox::critical(this, tr("Error"), message);
    }
    else {
        QMessageBox::information(this, tr("Information"), tr("Connection successful."));
    }
}

void CreateNGWConnectionDialog::on_loginAsGuest_stateChanged(int arg1)
{
    Q_UNUSED(arg1)
    if(ui->loginAsGuest->isChecked()) {
        m_login = ui->loginEdit->text();
        m_password = ui->passwordEdit->text();
        ui->loginEdit->setText(tr("guest"));
        ui->passwordEdit->setText("");
        ui->loginEdit->setEnabled(false);
        ui->passwordEdit->setEnabled(false);
    }
    else {
        ui->loginEdit->setText(m_login);
        ui->passwordEdit->setText(m_password);
        ui->loginEdit->setEnabled(true);
        ui->passwordEdit->setEnabled(true);
    }
}

void CreateNGWConnectionDialog::accept()
{
    if(name().isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Connection name should be set."));
        return;
    }

    if(url().isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("NextGIS Web connection url should be set."));
        return;
    }

    if(login().isEmpty() && !isGuest()) {
        QMessageBox::warning(this, tr("Warning"), tr("NextGIS Web login should be set."));
        return;
    }

    if(password().isEmpty() && !isGuest()) {
        QMessageBox::warning(this, tr("Warning"), tr("NextGIS Web login should be set."));
        return;
    }

    return QDialog::accept();
}

QString CreateNGWConnectionDialog::name() const
{
    return ui->connNameEdit->text();
}

QString CreateNGWConnectionDialog::url() const
{
    return ui->connectionUrlEdit->text();
}

QString CreateNGWConnectionDialog::login() const
{
    return ui->loginEdit->text();
}

QString CreateNGWConnectionDialog::password() const
{
    return ui->passwordEdit->text();
}

bool CreateNGWConnectionDialog::isGuest() const
{
    return ui->loginAsGuest->isChecked();
}
