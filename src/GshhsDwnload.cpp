/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2008 - Christophe Thomas aka Oxygen77

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

Original code: zyGrib: meteorological GRIB file viewer
Copyright (C) 2008 - Jacques Zaninetti - http://zygrib.free.fr

***********************************************************************/

#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QDebug>
#include <QFileDialog>
#include <QThread>

#include "mycentralwidget.h"
#include "miniunz.h"
#include "inetConnexion.h"
#include "DialogInetProgess.h"
#include "settings.h"
#include "dataDef.h"
#include "Progress.h"

#include "GshhsDwnload.h"

GshhsDwnload::GshhsDwnload(myCentralWidget * centralWidget, inetConnexion *inet):
  QObject(centralWidget),
  inetClient(inet)
{
    this->centralWidget=centralWidget;
}

#define MAP_FNAME  "qtVlmMaps.zip"
//"win_exe-3.1-1.zip"
//"qtVlmMaps.zip"

void GshhsDwnload::requestFinished(QByteArray res) {
    QString tmpPath = QDir::tempPath();
    if(!tmpPath.endsWith("/") && !tmpPath.endsWith("\\"))
        tmpPath.append('/');

    filename = tmpPath + MAP_FNAME;

    errorDuringDownload=false;

    QDir::temp().remove(MAP_FNAME);

    QFile *saveFile = new QFile(filename);
    if(saveFile && saveFile->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        int nb=saveFile->write(res);
        saveFile->close();
        qWarning() << "gshhs zip, " << nb << " bytes saved in " << filename;
        if(nb<=0) {
            errorDuringDownload = true;
            QDir::temp().remove(MAP_FNAME);
        }


    }
    else {
        errorDuringDownload = true;
    }

    if(errorDuringDownload) {
        QMessageBox::critical(centralWidget,
                              tr("Saving maps"),
                              tr("Zip file ") + filename + tr(" can't be opened"));
    }
    finished=true;
}


void GshhsDwnload::getMaps(void) {
    QString page="/oxygen/qtvlmMaps/";

    //QString page = "/oxygen/";

    page+= MAP_FNAME;

    connect (this->getInet()->getProgressDialog(),SIGNAL(rejected()),this,SLOT(slot_abort()));
    finished=false;
    filename="";
    inetGetProgress(1,page,"http://www.virtual-winds.org",false);
    while(!finished) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
    }
    if(!errorDuringDownload) {
        /* asking for folder holding maps */
        QString dir = Settings::getSetting("mapsFolder",appFolder.value("maps")).toString();

        Progress * progress=centralWidget->getMainWindow()->get_progress();

        if(progress) {
            progress->newStep(progress->value()+5,tr("Decompressing maps"));
            QCoreApplication::processEvents();
        }

        int res = miniunzip(UZ_OVERWRITE,(const char*)filename.toLatin1().data(),dir.toLatin1().data(),NULL,NULL);

        if(res!=UNZ_OK) {
            QMessageBox::critical(centralWidget,
                                  tr("Saving maps"),
                                  tr("Zip file ") + filename + tr(" can't be unzip")+" (error="+QString().setNum(res)+")");
        }
        else {
            centralWidget->loadGshhs();
        }
        QDir::temp().remove(MAP_FNAME);

    }
}

void GshhsDwnload::slot_abort(void) {
    this->inetAbort();
    errorDuringDownload=true;
    finished=true;
}


