/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2011 - Christophe Thomas aka Oxygen77

http://qtvlm.sf.net

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
***********************************************************************/

#include <QtWidgets/QDialog>
#include <QDebug>

#include "DialogProxy.h"
#include "settings.h"
#include "Util.h"

DialogProxy::DialogProxy() : QDialog()
{
    qWarning() << "Init dialogProxy";
    setupUi(this);
    Util::setFontDialog(this);

    lineProxyHostname->setText(Settings::getSetting("httpProxyHostname", "").toString());
    lineProxyPort->setText(Settings::getSetting("httpProxyPort", "").toString());
    lineProxyUsername->setText(Settings::getSetting("httpProxyUsername", "").toString());
    lineProxyUserPassword->setText(Settings::getSetting("httpProxyUserPassword", "").toString());

    int usep = Settings::getSetting("httpUseProxy", 0).toInt();

    useProxy_btn->setChecked(usep!=0);
    noProxy_btn->setChecked(usep==0);

    if(usep!=0)
        proxyType->setCurrentIndex(usep-1);

    slot_useProxy_changed();
}

void DialogProxy::slot_useProxy_changed()
{
    bool usep = useProxy_btn->isChecked();

    int type = proxyType->currentIndex();

    proxyType->setEnabled(usep);
    lineProxyHostname->setEnabled(usep && (type != 0));
    lineProxyPort->setEnabled(usep && (type != 0));
    lineProxyUsername->setEnabled(usep);
    lineProxyUserPassword->setEnabled(usep);
}

void DialogProxy::slot_proxyType_changed(int /*type*/)
{
    slot_useProxy_changed();
}

void DialogProxy::done(int result)
{
    Settings::setSetting(this->objectName()+".height",this->height());
    Settings::setSetting(this->objectName()+".width",this->width());
    if(result == QDialog::Accepted)
    {
        int type;
        if(useProxy_btn->isChecked())
            type=proxyType->currentIndex()+1;
        else
            type=0;
        Settings::setSetting("httpUseProxy",type);
        Settings::setSetting("httpProxyHostname", lineProxyHostname->text());
        Settings::setSetting("httpProxyPort", lineProxyPort->text());
        Settings::setSetting("httpProxyUsername", lineProxyUsername->text());
        Settings::setSetting("httpProxyUserPassword", lineProxyUserPassword->text());
        emit proxyUpdated();
    }
    QDialog::done(result);
}
