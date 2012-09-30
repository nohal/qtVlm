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

#include "mycentralwidget.h"
#include "miniunz.h"
#include "inetConnexion.h"
#include "DialogInetProgess.h"
#include "settings.h"
#include "dataDef.h"

#include "GshhsDwnload.h"

GshhsDwnload::GshhsDwnload(myCentralWidget * centralWidget, inetConnexion *inet):
  QObject(centralWidget),
  inetClient(inet)
{
    this->centralWidget=centralWidget;

    needAuth=false;
}

#define MAP_FNAME  "qtVlmMaps.zip"
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
                              tr("Sauvegarde des cartes"),
                              tr("Le fichier ") + filename + tr(" ne peut etre ouvert"));
    }
    finished=true;
}


void GshhsDwnload::getMaps(void) {
    QString page="/~oxygen/qtvlmMaps/";
    page += MAP_FNAME;
    connect (this->getInet()->getProgressDialog(),SIGNAL(rejected()),this,SLOT(slot_abort()));
    finished=false;
    filename="";
    inetGetProgress(1,page,"http://www.virtual-winds.com");
    while(!finished) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
    }
    if(!errorDuringDownload) {
        /* asking for folder holding maps */
        QString dir = Settings::getSetting("mapsFolder",appFolder.value("maps")).toString();

        dir = QFileDialog::getExistingDirectory(centralWidget, tr("Select maps folder"),
                                                        dir,
                                                        QFileDialog::ShowDirsOnly);

        QProgressDialog * progress=centralWidget->getMainWindow()->get_progress();

        if(progress) {
            progress->setLabelText(tr("Decompression des cartes"));
            progress->setValue(progress->value()+5);
            progress->raise();
            progress->activateWindow();
        }


        if(miniunzip(UZ_OVERWRITE,(const char*)filename.toAscii().data(),dir.toAscii().data(),NULL,NULL)!=UNZ_OK) {
            QMessageBox::critical(centralWidget,
                                  tr("Sauvegarde des cartes"),
                                  tr("Le fichier zip ") + filename + tr(" ne peut etre dezippe"));
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


