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
#include "progressdialog.h"
#include "ui_progressdialog.h"

ProgressDialog::ProgressDialog(const QString &title, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProgressDialog),
    m_cancel(false)
{
    ui->setupUi(this);
    ui->progressBar->setRange(0, 100);
    ui->progressBar->setValue(0);
    setWindowTitle(title);
}

ProgressDialog::~ProgressDialog()
{
    delete ui;
}

bool ProgressDialog::isCancel()
{
    return m_cancel;
}

void ProgressDialog::setProgress(int progress)
{
    ui->progressBar->setValue(progress);
}

void ProgressDialog::onCancelClicked()
{
   m_cancel = true;
}
