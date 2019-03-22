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
#include "loginmynextgiscomdialog.h"
#include "ui_loginmynextgiscomdialog.h"

#include "ngstore/api.h"

#include <QMessageBox>

constexpr const char *CLIENT_ID = "pldx0MSrHfvEDr2yq7tebb5AxCvnsiyBrAowZmli";
constexpr const char *AUTH_URL = "https://my.nextgis.com/oauth2/token/";
constexpr const char *API_ENDPOINT = "https://my.nextgis.com/api/v1";

LoginMyNextGISComDialog::LoginMyNextGISComDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginMyNextGISComDialog)
{
    ui->setupUi(this);
}

LoginMyNextGISComDialog::~LoginMyNextGISComDialog()
{
    delete ui;
}

void LoginMyNextGISComDialog::accept()
{
    QString login = ui->loginEdit->text();
    QString password = ui->passwordEdit->text();
    QString clientId(CLIENT_ID);

    QDialog::accept();

    QString payload = QString("grant_type=password&client_id=%1&username=%2&password=%3&scope=user_info.read").arg(clientId).arg(login).arg(password);

    char **options = nullptr;
    options = ngsListAddNameValue(options, "CUSTOMREQUEST", "POST");
    options = ngsListAddNameValue(options, "POSTFIELDS", payload.toStdString().c_str());

    JsonDocumentH doc = ngsJsonDocumentCreate();
    int result = ngsJsonDocumentLoadUrl(doc, AUTH_URL, options, nullptr, nullptr);
    ngsListFree(options);
    if(result != 200) {
        ngsJsonDocumentFree(doc);
        QMessageBox::critical(this, tr("Error"), tr("Login failed."));
        return;
    }

    JsonObjectH root = ngsJsonDocumentRoot(doc);
    char **authOptions = nullptr;
    authOptions = ngsListAddNameValue(authOptions, "type", "bearer");
    const char *expiresIn = ngsJsonObjectGetStringForKey(root, "expires_in", "120");
    authOptions = ngsListAddNameValue(authOptions, "expiresIn", expiresIn);
    authOptions = ngsListAddNameValue(authOptions, "clientId", CLIENT_ID);
    authOptions = ngsListAddNameValue(authOptions, "tokenServer", AUTH_URL);
    const char *accessToken = ngsJsonObjectGetStringForKey(root, "access_token", "");
    authOptions = ngsListAddNameValue(authOptions, "accessToken", accessToken);
    const char *updateToken = ngsJsonObjectGetStringForKey(root, "refresh_token", "");
    authOptions = ngsListAddNameValue(authOptions, "updateToken", updateToken);

    ngsJsonDocumentFree(doc);

    if(ngsURLAuthAdd(API_ENDPOINT, authOptions) != 200) {
        ngsListFree(authOptions);
        QMessageBox::critical(this, tr("Error"), tr("Add bearer failed."));
        return;
    }

    ngsListFree(authOptions);

    if(ngsAccountUpdateUserInfo() != 1) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to update user info."));
        return;
    }
    if(ngsAccountUpdateSupportInfo() != 1) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to update support info."));
        return;
    }

    QString userFirstName = QString::fromUtf8(ngsAccountGetFirstName());
    QString userLastName = QString::fromUtf8(ngsAccountGetLastName());
    QString usetEmail = QString::fromUtf8(ngsAccountGetEmail());

    QMessageBox::information(this, tr("Account"),
                             tr("User <b>%1 %2</b><p>"
                                "email: %3<p>"
                                "authorised: %4<p>"
                                "supported: %5")
                                  .arg(userFirstName)
                                  .arg(userLastName)
                                  .arg(usetEmail)
                                  .arg(ngsAccountIsAuthorized() == 1 ? "yes" : "no")
                                  .arg(ngsAccountSupported() == 1 ? "yes" : "no"));

}

