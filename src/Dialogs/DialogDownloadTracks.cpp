/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2013 - Christophe Thomas aka Oxygen77

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

#include <QMessageBox>
#include <QTextStream>
#include <QDebug>
#include <QDir>

#include "DialogDownloadTracks.h"
#include "mycentralwidget.h"
#include "Player.h"
#include "settings.h"
#include "Util.h"

#define VLM_RACE_INFO 2
#define VLM_GET_TRACK 3
#define VLM_GET_PARTIAL_TRACK 4
#define VLM_BOAT_INFO 5

DialogDownloadTracks::DialogDownloadTracks(MainWindow * ,myCentralWidget * parent,inetConnexion * inet) :
    QDialog(parent),
    inetClient(inet),
    ui(new Ui::DialogDownloadTracks)
{
    this->parent=parent;
    ui->setupUi(this);
    Util::setFontDialog(this);
    this->raceIsValid=false;
    this->setWhatsThis(tr("Permet de telecharger manuellement une trace pour une course VLM.\nLa boÃŪte Ã  cocher trace partielle s'active apres l'entree d'un numero de course valide, et permet de requÃĐrir une trace tronquÃĐe."));
    ui->raceIDEdit->setToolTip(tr("Numero de la course\n http://www.v-l-m.org/races.php?fulllist=1"));
    ui->boatIDEdit->setToolTip(tr("Numero du bateau"));
    ui->startTimeEdit->setToolTip(tr("Debut de la trace"));
    ui->startTimeEdit->setEnabled(false);
    ui->endTimeEdit->setToolTip(tr("Fin de la trace"));
}

DialogDownloadTracks::~DialogDownloadTracks()
{
    Settings::setSetting(this->objectName()+".height",this->height());
    Settings::setSetting(this->objectName()+".width",this->width());
    Settings::setSetting(this->objectName()+".positionx",this->pos().x());
    Settings::setSetting(this->objectName()+".positiony",this->pos().y());
    delete ui;
    //qWarning()<<"delete DialogDownLoadTracks completed";
}

void DialogDownloadTracks::init()
{
    ui->raceIDEdit->setValue(20120101);
    ui->labelDisplayRaceName->setText("N/A");
    ui->boatIDEdit->setValue(20000);
    ui->labelDisplayBoatName->setText("N/A");
    raceIsValid=false;
    boatIsValid=false;
    raceID=0;
    boatID=0;
    ui->endTimeEdit->setEnabled(false);
    ui->labelStartTime->setEnabled(false);
    ui->labelEndTime->setEnabled(false);
    ui->frameTrackCheckBox->setEnabled(false);
    ui->frameTrackCheckBox->setChecked(false);
    ui->boatIDEdit->selectAll();
    ui->editRouteName->setEnabled(false);
    ui->editRouteName->setText("");
    ui->labelPathName->setEnabled(false);
    ui->labelPathName->setText("N/A");
    ui->labelPath->setEnabled(false);
    ui->labelFileName->setText("N/A");
    ui->labelStatus->setEnabled(false);
    fileName="";
    fullFileName="";
    routeName="";
    qStartTime.setTimeSpec(Qt::UTC);
    qEndTime.setTimeSpec(Qt::UTC);
    filePath=appFolder.value("tracks");
    ui->labelPathName->setText(filePath);
    cached=false;
    connect(ui->fetchButton,SIGNAL(clicked()),this,SLOT(slot_fetch()));
    dlRunning = false;
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    ui->fetchButton->setEnabled(false);
    this->show();
}

void DialogDownloadTracks::accept()
{
    done(QDialog::Accepted);
}
void DialogDownloadTracks::done(int result)
{
    QDialog::done(result);
}

void DialogDownloadTracks::on_boatIDEdit_valueChanged(int)
{
    boatID=ui->boatIDEdit->value();
    doRequest(VLM_BOAT_INFO);
}

void DialogDownloadTracks::on_raceIDEdit_valueChanged(int)
{
    raceID=ui->raceIDEdit->value();
    doRequest(VLM_RACE_INFO);
}

void DialogDownloadTracks::on_startTimeEdit_dateTimeChanged(QDateTime)
{
    qStartTime=ui->startTimeEdit->dateTime();
    updateFileName(ui->frameTrackCheckBox->isChecked());
}

void DialogDownloadTracks::on_endTimeEdit_dateTimeChanged(QDateTime)
{
    qEndTime=ui->endTimeEdit->dateTime();
    updateFileName(ui->frameTrackCheckBox->isChecked());
}

void DialogDownloadTracks::updateFileName(bool truncTrack)
{
    if (!truncTrack)
        fileName=fileName.sprintf("%d_%d_",raceID,boatID);
    else
        fileName=fileName.sprintf("%d_%d_%d_%d_",raceID,boatID,qStartTime.toTime_t(),qEndTime.toTime_t());
    fileName=fileName+".json";
    fullFileName=filePath+fileName;
    ui->labelFileName->setText(fileName);
    jsonFile.setFileName(fullFileName);
    if (jsonFile.open(QIODevice::ReadOnly))
    {
        ui->labelStatus->setText(tr("En cache."));
        cached=true;
    }
    else
    {
        ui->labelStatus->setText("VLM.");
        cached=false;
    }
}

void DialogDownloadTracks::on_frameTrackCheckBox_clicked(bool checked)
{
    ui->startTimeEdit->setEnabled(checked);
    ui->endTimeEdit->setEnabled(checked);
    ui->labelStartTime->setEnabled(checked);
    ui->labelEndTime->setEnabled(checked);
    updateFileName(ui->frameTrackCheckBox->isChecked());
}

/*****************************************
* Inet request
****************************************/
QString DialogDownloadTracks::getAuthLogin(bool * ok)
{
    Player * cur_player=parent->getPlayer();
    if(cur_player)
        return cur_player->getAuthLogin(ok);
    else
    {
        if(ok)
            *ok=true;
        return QString();
    }
}

QString DialogDownloadTracks::getAuthPass(bool * ok)
{
    Player * cur_player=parent->getPlayer();
    if(cur_player)
        return cur_player->getAuthPass(ok);
    else
    {
        if(ok)
            *ok=true;
        return QString();
    }
}

void DialogDownloadTracks::authFailed(void)
{
    QMessageBox::warning(0,QObject::tr("Parametre bateau"),
                  "Erreur de parametrage du joueur.\n Verifier le login et mot de passe puis reactivez le bateau");
    inetClient::authFailed();
}

void DialogDownloadTracks::inetError()
{

}

bool DialogDownloadTracks::doRequest(int reqType)
{
    if(!hasInet() || hasRequest())
    {
        qWarning("VLM Tracks:  bad state in inet");
        return false;
    }

    QString page;

    switch(reqType)
    {
    case VLM_GET_TRACK:
        QTextStream(&page)
               << "/ws/boatinfo/tracks.php?"
               <<"idr="
               << raceID
               << "&idu="
               << boatID
               << "&starttime="
               << startTime;
//               << "&endtime="
//               << endTime;
        inetGet(VLM_GET_TRACK,page,false);
        qWarning()<<"Sending Track Request: "<<page;
        break;
    case VLM_GET_PARTIAL_TRACK:
        QTextStream(&page)
               << "/ws/boatinfo/tracks.php?"
               <<"idr="
               << raceID
               << "&idu="
               << boatID
               << "&starttime="
               << startTime
               << "&endtime="
               << endTime;
        inetGet(VLM_GET_PARTIAL_TRACK,page,false);
        qWarning()<<"Sending Track Request: "<<page;
        break;
    case VLM_RACE_INFO:
        QTextStream(&page)
               << "/ws/raceinfo.php?"
               <<"idrace="
               << raceID;
        inetGet(VLM_RACE_INFO,page,false);
        qWarning()<<"Sending Track Request: "<<page;
        break;
    case VLM_BOAT_INFO:
        QTextStream(&page)
               << "/ws/boatinfo/profile.php?"
               <<"idu="
               << boatID;
        inetGet(VLM_BOAT_INFO,page,false);
        qWarning()<<"Sending Track Request: "<<page;
        break;
    }
    return true;
}

void DialogDownloadTracks::requestFinished (QByteArray data)
{
    switch(getCurrentRequest())
    {
    case VLM_GET_TRACK:
    {        
        QVariantMap result;
        if (routeName.isEmpty())
        {
            QMessageBox msgBox;
            msgBox.setText(tr("Trace entiere: pas de nom de route."));
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.exec();
            return;
        }
        if (!inetClient::JSON_to_map(data,&result)) {
            return;
        }
        if (result["nb_tracks"]!=0)
        {
            if(fullFileName.isEmpty())
            {
                qWarning() << "Empty file name in VLM track save";
            }
            else
            {
                QFile *saveFile = new QFile(fullFileName);
                assert(saveFile);
                if (saveFile->open(QIODevice::WriteOnly))
                {
                    int nb=saveFile->write(data);
                    if(nb>0)
                        saveFile->close();
                    //qWarning() << nb << " bytes saved in " << fullFileName;
                }
                else
                {
                    QMessageBox::critical (this,
                                           tr("Erreur"),
                                           tr("Ecriture du fichier impossible."));
                }
            }
            QVariant trackRaw=result["tracks"];
            QList<QVariant> details=trackRaw.toList();
            parent->withdrawRouteFromBank(routeName,details);
        }
        else
        {
            if (result["tracks_hidden"]=="true") //maybe consider BO case here.
            {
                QString errMsg;
                QStringList errMsgList;
                errMsgList<< tr("Trace cachee pour:");
                errMsgList<< tr(QString("Course: %1").arg(raceID).toLatin1());
                errMsgList<< tr(QString("Bateau: %1").arg(boatID).toLatin1());
                errMsg=errMsgList.join("\n");
                QMessageBox::warning(this,
                                     tr("Pas de trace"),
                                     errMsg);
            }
            else
            {
                QString errMsg;
                QStringList errMsgList;
                errMsgList<< tr("Pas de trace correspondant a la requete:");
                errMsgList<< tr(QString("Course: %1").arg(raceID).toLatin1());
                errMsgList<< tr(QString("Bateau: %1").arg(boatID).toLatin1());
                errMsgList<< tr(QString("Heure debut: %1").arg(startTime).toLatin1());
                errMsg=errMsgList.join("\n");
                QMessageBox::warning(this,
                                     tr("Requete incorrecte"),
                                     errMsg);
            }
        }
        dlRunning = false;
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
    break;
    case VLM_GET_PARTIAL_TRACK:
    {
        QVariantMap result;
        if (routeName.isEmpty())
        {
            QMessageBox msgBox;
            msgBox.setText(tr("Trace partielle: pas de nom de route."));
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.exec();
            return;
        }
        if (!inetClient::JSON_to_map(data,&result)) {
            return;
        }
        if (result["nb_tracks"]!=0)
        {
            if(fullFileName.isEmpty())
            {
                qWarning() << "Empty file name in VLM track save";
            }
            else
            {
                QFile *saveFile = new QFile(fullFileName);
                assert(saveFile);
                if (saveFile->open(QIODevice::WriteOnly))
                {
                    int nb=saveFile->write(data);
                    if(nb>0)
                        saveFile->close();
                    //qWarning() << nb << " bytes saved in " << fullFileName;
                }
                else
                {
                    QMessageBox::critical (this,
                                           tr("Erreur"),
                                           tr("Ecriture du fichier impossible."));
                }
            }
            QVariant trackRaw=result["tracks"];
            QList<QVariant> details=trackRaw.toList();
            parent->withdrawRouteFromBank(routeName,details);
        }
        else
        {
            if (result["tracks_hidden"]=="true")
            {
                QString errMsg;
                QStringList errMsgList;
                errMsgList<< tr("Trace cachee pour:");
                errMsgList<< tr(QString("Course: %1").arg(raceID).toLatin1());
                errMsgList<< tr(QString("Bateau: %1").arg(boatID).toLatin1());
                errMsgList<< tr(QString("Heure debut: %1").arg(qStartTime.toString("yyyy/MM/dd hh:mm:ss UTC")).toLatin1());
                errMsgList<< tr(QString("Heure fin: %1").arg(qEndTime.toString("yyyy/MM/dd hh:mm:ss UTC")).toLatin1());
                errMsg=errMsgList.join("\n");
                QMessageBox::warning(this,
                                     tr("Pas de trace"),
                                     errMsg);
            }
            QString errMsg;
            QStringList errMsgList;
            errMsgList<< tr("Pas de trace correspondant a la requete:");
            errMsgList<< tr(QString("Course: %1").arg(raceID).toLatin1());
            errMsgList<< tr(QString("Bateau: %1").arg(boatID).toLatin1());
            errMsgList<< tr(QString("Heure debut: %1").arg(qStartTime.toString("yyyy/MM/dd hh:mm:ss UTC")).toLatin1());
            errMsgList<< tr(QString("Heure fin: %1").arg(qEndTime.toString("yyyy/MM/dd hh:mm:ss UTC")).toLatin1());
            errMsg=errMsgList.join("\n");
            QMessageBox::warning(this,
                                 tr("Requete incorrecte"),
                                 errMsg);
        }
        dlRunning = false;
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
    break;
    case VLM_RACE_INFO:
    {
        //http://v-l-m.org/ws/raceinfo.php?idrace=20110524
       // {"idraces":"20110524","racename":"Transatlantic NY - Lizard","started":"1","deptime":"1306263600","startlong":"-73837","startlat":"40458","boattype":"boat_VLM70","closetime":"1337820526","racetype":"1","firstpcttime":"200","depend_on":"0","qualifying_races":"","idchallenge":"","coastpenalty":"900","bobegin":"0","boend":"0","maxboats":"0","theme":"","vacfreq":"5","races_waypoints":{"1":{"idwaypoint":"2011052401","wpformat":"0","wporder":"1","wptype":"Finish","latitude1":"49960","longitude1":"-5201","latitude2":"49900","longitude2":"-5201","libelle":"Point Lizard","maparea":"12"}},"races_instructions":[{"idraces":"20110524","instructions":"http:\/\/www.virtual-winds.org\/forum\/index.php?s=&showtopic=6853&view=findpost&p=225544","flag":"13","autoid":"271"}]}

        //qWarning()<<"inside VLM_RACE_INFO";

        QVariantMap result;
        if (!inetClient::JSON_to_map(data,&result)) {
            if (data.startsWith("0"))
                qWarning()<<"No race such as: "<<raceID;
            else
            {
                return;
            }
            raceIsValid=false;
            ui->labelDisplayRaceName->setText("N/A");
            ui->startTimeEdit->setEnabled(false);
            ui->labelStartTime->setEnabled(false);
            ui->labelEndTime->setEnabled(false);
            ui->endTimeEdit->setEnabled(false);
            ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
            return;
        }
        else
        {
             raceIsValid=true;
             ui->labelDisplayRaceName->setText(result["racename"].toString());
             startTime=result["deptime"].toInt();
             qStartTime.setTime_t(startTime);
             ui->startTimeEdit->setMinimumDateTime(qStartTime);
             ui->startTimeEdit->setDateTime(qStartTime);
             ui->endTimeEdit->setMinimumDateTime(qStartTime);
             ui->endTimeEdit->setDateTime(qStartTime);
             ui->labelDisplayRaceName->setText(result["racename"].toString());
             if (boatIsValid)
             {
                 ui->editRouteName->setEnabled(true);
                 ui->frameTrackCheckBox->setEnabled(true);
                 ui->frameTrackCheckBox->setChecked(false);
                 updateFileName(ui->frameTrackCheckBox->isChecked());
                 ui->fetchButton->setEnabled(true);
             }
//             ui->startTimeEdit->setEnabled(true);
//             ui->endTimeEdit->setEnabled(true);
        }
        break;
    }
    case VLM_BOAT_INFO:
    {
        //qWarning()<<"inside VLM_BOAT_INFO";
        QVariantMap result;
        if (!inetClient::JSON_to_map(data,&result)) {
            boatIsValid=false;
            ui->labelDisplayBoatName->setText("N/A");
            ui->fetchButton->setEnabled(false);
            return;
        }
        else
            if (result["success"]=="true")
            {
                boatIsValid=true;
                QVariantMap profile=result["profile"].toMap();
                playerName=profile["OWN"].toString();
                routeName=playerName;
                ui->editRouteName->setText(routeName);
                ui->labelDisplayBoatName->setText(playerName);
                if (raceIsValid)
                    ui->fetchButton->setEnabled(true);
            }
            else
            {
                boatIsValid=false;
                playerName="";
                ui->labelDisplayBoatName->setText("N/A");
                routeName="";
                ui->editRouteName->setText(routeName);
                ui->editRouteName->setEnabled(false);
                ui->fetchButton->setEnabled(false);
            }
        break;
    }
    }
}

void DialogDownloadTracks::slot_fetch (void)
{
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    if (raceIsValid&&boatIsValid)
    {
        raceID=ui->raceIDEdit->value();
        boatID=ui->boatIDEdit->value();
        routeName=ui->editRouteName->text();
        if (ui->frameTrackCheckBox->isChecked())
        {
            qStartTime=ui->startTimeEdit->dateTime();
            startTime=qStartTime.toTime_t();
            qEndTime=ui->endTimeEdit->dateTime();
            endTime=qEndTime.toTime_t();
            updateFileName(ui->frameTrackCheckBox->isChecked());
            if ( !cached ) {
                dlRunning=true;
                doRequest(VLM_GET_PARTIAL_TRACK);
            }
            else
            {
                QTextStream stream(&jsonFile);
                QByteArray data;
                stream>>data;
                QVariantMap result;
                if (!inetClient::JSON_to_map(data,&result)) {
                    return;
                }
                if (result["nb_tracks"]!=0)
                {
                    QVariant trackRaw=result["tracks"];
                    QList<QVariant> details=trackRaw.toList();
                    parent->withdrawRouteFromBank(routeName,details);
                    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
                }
             }
        }
        else
        {
            updateFileName(ui->frameTrackCheckBox->isChecked());
            if ( !cached ) {
                dlRunning=true;
                doRequest(VLM_GET_TRACK);
            }
            else
            {
                QTextStream stream(&jsonFile);
                QByteArray data;
                stream>>data;
                QVariantMap result;
                if (!inetClient::JSON_to_map(data,&result)) {
                    return;
                }
                if (result["nb_tracks"]!=0)
                {
                    QVariant trackRaw=result["tracks"];
                    QList<QVariant> details=trackRaw.toList();
                    parent->withdrawRouteFromBank(routeName,details);
                    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
                }
            }
        }
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setText(tr("Course ou bateau inconnu"));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
    }
}
