/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2014 - Christophe Thomas aka Oxygen77

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


/**********************************************************************************
 * Controle class for DialogVlmGrib View
 *
 * Uses child class of DialogVlmGriv_view
 * => call updateList(LIST) to send new list
 * => call set_dialogVisibility(BOOL) to show/hide dialog
 * => call set_waitBoxVisibility(BOOL) to show/hide a waitBox during inet access
 *
 * View should call: downloadGrib(INT) to start a download / exitDialog() to close dialog
 **************************************************************************************/

#include <QNetworkRequest>
#include <QFileDialog>
#include <QMessageBox>

#include "settings.h"
#include "mycentralwidget.h"
#include "Util.h"
#include "inetConnexion.h"
#include "DialogInetProgess.h"
#include "MainWindow.h"

#define VLM_REQUEST_GET_FOLDER 0
#define VLM_REQUEST_GET_FILE   1

#include "DialogVlmGrib_view_pc.h"
#include "DialogVlmGrib_ctrl.h"

DialogVlmGrib_ctrl::DialogVlmGrib_ctrl(MainWindow *mainWindow, myCentralWidget * centralWidget, inetConnexion * inet):  inetClient(inet)
{

    view = new DialogVlmGrib_view_pc(centralWidget,this);
    if(!view) return;
    this->centralWidget=centralWidget;

    connect(this,SIGNAL(signalGribFileReceived(QString)),mainWindow,SLOT(slot_gribFileReceived(QString)));
    filename="";

    updateList();
}

void DialogVlmGrib_ctrl::updateList(void) {
    if(doRequest(VLM_REQUEST_GET_FOLDER)) {
        view->set_waitBoxVisibility(true);
    }
    else
        exitDialog();
}

void DialogVlmGrib_ctrl::downloadGrib(int itemIndex) {
    if(itemIndex<0 || itemIndex>=lst_fname.size())
        updateList();
    if(!doRequest(VLM_REQUEST_GET_FILE,itemIndex))
        updateList();
}

void DialogVlmGrib_ctrl::exitDialog(void) {
    if(view) view->set_dialogVisibility(false);
    deleteLater();
}

QStringList DialogVlmGrib_ctrl::parseFolderListing(QString data)
{
    int pos=0;

    QString gribName_str;
    QString date_str;
    QString size_str;

    lst_fname.clear();
    QStringList lst;

    while(1) {
        /* grib file name */
        int previousPos=pos;
        pos = data.indexOf("gfs_NOAA",pos);
        if(pos==-1) {
            pos=previousPos;
            pos = data.indexOf("gfs_interim",pos);
            if(pos==-1)
                break;
            gribName_str = data.mid(pos,18);
            pos+=18;
        }
        else {
            gribName_str = data.mid(pos,23);
            pos+=23;
        }
        /* grib date */
        pos = data.indexOf("<td align=\"right\">",pos);
        if(pos==-1) break;
        date_str = data.mid(pos+18,17);
        pos=pos+20;

        /* file size */
        pos = data.indexOf("<td align=\"right\">",pos);
        if(pos==-1) break;
        size_str = data.mid(pos+18,4);
        pos=pos+20;

        lst_fname.append(gribName_str);
        lst.append(gribName_str + ", modified " + date_str + ", size " + size_str);
    }
    return lst;
}

bool DialogVlmGrib_ctrl::gribFileReceived(QByteArray * content) {
    QString gribPath=Settings::getSetting(edtGribFolder).toString();
    QDir dirGrib(gribPath);
    if(!dirGrib.exists()) {
        gribPath=appFolder.value("grib");
        Settings::setSetting(askGribFolder,1);
        Settings::setSetting(edtGribFolder,gribPath);
    }
    filename=gribPath+"/"+filename;
    if(Settings::getSetting(askGribFolder)==1) {
        filename = QFileDialog::getSaveFileName(centralWidget,
                         tr("Sauvegarde du fichier GRIB"), filename, "Grib (*.grb)");
    }

    if (filename != "") {
        QFile *saveFile = new QFile(filename);
        assert(saveFile);
        if (saveFile->open(QIODevice::WriteOnly)) {
            int nb=saveFile->write(*content);
            saveFile->close();
            qWarning() << nb << " bytes saved in " << filename;
            return true;
        }
        else {
            QMessageBox::critical (centralWidget,
                    tr("Erreur"),
                    tr("Ecriture du fichier impossible."));
        }
    }
    return false;
}

/*****************************************
* Inet request
****************************************/

bool DialogVlmGrib_ctrl::doRequest(int reqType,int param)
{
    if(!hasInet() || hasRequest())
    {
        qWarning("VLM dvnld grib:  bad state in inet");
        return false;
    }

    QString page;

    switch(reqType)
    {
        case VLM_REQUEST_GET_FOLDER:
            page="/";
            inetGet(VLM_REQUEST_GET_FOLDER,page,"http://grib.v-l-m.org",false);
            break;
        case VLM_REQUEST_GET_FILE:
            /*search selected file*/
            if(!view) return false;
            if(param < 0 || param >= lst_fname.size()) return false;
            filename=lst_fname.at(param);
            if(filename.contains("interim"))
                filename=filename.mid(0,18);
            else
                filename=filename.mid(0,23);
            page="/"+filename;
            view->set_dialogVisibility(false);
            inetGetProgress(VLM_REQUEST_GET_FILE,page,"http://grib.v-l-m.org",false);
            connect (this->getInet()->getProgressDialog(),SIGNAL(rejected()),this,SLOT(slot_abort()));
            break;
    }
    return true;
}

void DialogVlmGrib_ctrl::slot_abort()
{
    qWarning()<<"aborting VLM grib donwload";
    this->inetAbort();
    filename.clear();
    updateList();
}

void DialogVlmGrib_ctrl::requestFinished (QByteArray data)
{
    if(data.isEmpty() || data.isNull()) return;
    switch(getCurrentRequest())
    {
        case VLM_REQUEST_GET_FOLDER:{
            if(!view) return;
            view->set_waitBoxVisibility(false);
            QStringList lst=parseFolderListing(QString(data));

            if(lst.size()==0)
            {
                if(view) delete view;
                view=NULL;
                return;
            }
            view->updateList(lst);
            view->set_dialogVisibility(true);
            break;
        }
        case VLM_REQUEST_GET_FILE:
            if(!gribFileReceived(&data) && view)
                updateList();
            else
            {
                emit signalGribFileReceived(filename);                
                exitDialog();
            }
            break;
    }
}

