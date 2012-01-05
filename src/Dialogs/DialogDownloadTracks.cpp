#include <QMessageBox>
#include <QTextStream>
#include <QDebug>
#include <QFileDialog>
#include "DialogDownloadTracks.h"
#include "ui_DialogDownloadTracks.h"
#include "mycentralwidget.h"
#include "Player.h"
#include "parser.h"
#include "settings.h"

#define VLM_RACE_INFO 2
#define VLM_GET_TRACK 3
#define VLM_BOAT_INFO 5

DialogDownloadTracks::DialogDownloadTracks(MainWindow * main ,myCentralWidget * parent,inetConnexion * inet) :
    QDialog(parent),
    inetClient(inet),
    ui(new Ui::DialogDownloadTracks)
{
    this->parent=parent;
    ui->setupUi(this);
    this->raceIsValid=false;
    this->setWhatsThis(tr("Permet de telecharger manuellement une trace pour une course VLM.\nNecessite d'entrer un numero de bateau et un numero de courses valides."));
    ui->raceIDEdit->setToolTip(tr("Numero de la course\n http://www.virtual-loup-de-mer.org/races.php?fulllist=1"));
    ui->boatIDEdit->setToolTip(tr("Numero du bateau"));
    ui->boatIDEdit->selectAll();
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    fileName="";
    routeName="";
}

DialogDownloadTracks::~DialogDownloadTracks()
{
    delete ui;
}

void DialogDownloadTracks::init()
{
    ui->raceIDEdit->setValue(20120101);
    ui->labelDisplayRaceName->setText("N/A");
    ui->boatIDEdit->setValue(20000);
    ui->labelDisplayBoatName->setText("N/A");
    raceIsValid=false;
    boatIsValid=false;
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    raceID=0;
    boatID=0;
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    this->show();
}

void DialogDownloadTracks::accept()
{
    if (raceIsValid&&boatIsValid)
    {
        raceID=ui->raceIDEdit->value();
        boatID=ui->boatIDEdit->value();
        routeName=routeName.sprintf("%d_%d_",raceID,boatID);
        QString appExeFolder=QApplication::applicationDirPath();
        fileName=appExeFolder+"/tracks/"+routeName+".json";
        QFile jsonFile(fileName);
        if ( !jsonFile.open(QIODevice::ReadOnly) )
            doRequest(VLM_GET_TRACK);
        else
        {
            QTextStream stream(&jsonFile);
            QJson::Parser parser;
            bool ok;
            QByteArray data;
            stream>>data;
            QVariantMap result=parser.parse (data, &ok).toMap();
            if (!ok) {
                qWarning() << "Error parsing json data " << data;
                qWarning() << "Error: " << parser.errorString() << " (line: " << parser.errorLine() << ")";
            }
            if (result["nb_tracks"]!=0)
            {
                QVariant trackRaw=result["tracks"];
                QList<QVariant> details=trackRaw.toList();
                parent->withdrawRouteFromBank(routeName,details);
            }
        }

        QDialog::done(QDialog::Accepted);
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setText(tr("Course ou bateau inconnu"));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        QDialog::done(QDialog::Rejected);
    }
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
        inetGet(VLM_GET_TRACK,page);
        qWarning()<<"Sending Track Request: "<<page;
        break;
    case VLM_RACE_INFO:
        QTextStream(&page)
               << "/ws/raceinfo.php?"
               <<"idrace="
               << raceID;
        inetGet(VLM_RACE_INFO,page);
        qWarning()<<"Sending Track Request: "<<page;
        break;
    case VLM_BOAT_INFO:
        QTextStream(&page)
               << "/ws/boatinfo/profile.php?"
               <<"idu="
               << boatID;
        inetGet(VLM_BOAT_INFO,page);
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
        QJson::Parser parser;
        bool ok;
        QVariantMap result=parser.parse (data, &ok).toMap();
        if (routeName.isEmpty())
        {
            QMessageBox msgBox;
            msgBox.setText(tr("Ce nom est deja utilise ou invalide"));
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.exec();
            return;
        }
        if (!ok) {
            qWarning() << "Error parsing json data " << data;
            qWarning() << "Error: " << parser.errorString() << " (line: " << parser.errorLine() << ")";
        }
        if (result["nb_tracks"]!=0)
        {
            if(fileName.isEmpty())
            {
                qWarning() << "Empty file name in VLM track save";
            }
            else
            {
                QFile *saveFile = new QFile(fileName);
                assert(saveFile);
                if (saveFile->open(QIODevice::WriteOnly))
                {
                    int nb=saveFile->write(data);
                    if(nb>0)
                        saveFile->close();
                    //qWarning() << nb << " bytes saved in " << fileName;
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
                errMsgList<< tr(QString("Course: %1").arg(raceID).toAscii());
                errMsgList<< tr(QString("Bateau: %1").arg(boatID).toAscii());
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
                errMsgList<< tr(QString("Course: %1").arg(raceID).toAscii());
                errMsgList<< tr(QString("Bateau: %1").arg(boatID).toAscii());
                errMsgList<< tr(QString("Heure debut: %1").arg(startTime).toAscii());
                errMsg=errMsgList.join("\n");
                QMessageBox::warning(this,
                                     tr("Requete incorrecte"),
                                     errMsg);
            }
        }
    }
    break;
    case VLM_RACE_INFO:
    {
        //http://virtual-loup-de-mer.org/ws/raceinfo.php?idrace=20110524
       // {"idraces":"20110524","racename":"Transatlantic NY - Lizard","started":"1","deptime":"1306263600","startlong":"-73837","startlat":"40458","boattype":"boat_VLM70","closetime":"1337820526","racetype":"1","firstpcttime":"200","depend_on":"0","qualifying_races":"","idchallenge":"","coastpenalty":"900","bobegin":"0","boend":"0","maxboats":"0","theme":"","vacfreq":"5","races_waypoints":{"1":{"idwaypoint":"2011052401","wpformat":"0","wporder":"1","wptype":"Finish","latitude1":"49960","longitude1":"-5201","latitude2":"49900","longitude2":"-5201","libelle":"Point Lizard","maparea":"12"}},"races_instructions":[{"idraces":"20110524","instructions":"http:\/\/www.virtual-winds.com\/forum\/index.php?s=&showtopic=6853&view=findpost&p=225544","flag":"13","autoid":"271"}]}

        //qWarning()<<"inside VLM_RACE_INFO";
        QJson::Parser parser;
        bool ok;

        QVariantMap result=parser.parse (data, &ok).toMap();
        if (!ok) {
            if (data.startsWith("0"))
                qWarning()<<"No race such as: "<<raceID;
            else
            {
                qWarning() << "Error parsing json data " << data;
                qWarning() << "Error: " << parser.errorString() << " (line: " << parser.errorLine() << ")";
            }
            raceIsValid=false;
            ui->labelDisplayRaceName->setText("N/A");
            ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
            return;
        }
        else
        {
             raceIsValid=true;
             startTime=result["deptime"].toInt();
             qStartTime.setTimeSpec(Qt::UTC);
             qStartTime.setTime_t(startTime);
             ui->labelDisplayRaceName->setText(result["racename"].toString());
             if (boatIsValid)
                ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
//             ui->startTimeEdit->setEnabled(true);
//             ui->endTimeEdit->setEnabled(true);
        }
        break;
    }
    case VLM_BOAT_INFO:
    {
        //qWarning()<<"inside VLM_BOAT_INFO";
        QJson::Parser parser;
        bool ok;

        QVariantMap result=parser.parse (data, &ok).toMap();
        if (!ok)
        {
            qWarning() << "Error parsing json data " << data;
            qWarning() << "Error: " << parser.errorString() << " (line: " << parser.errorLine() << ")";

            boatIsValid=false;
            ui->labelDisplayBoatName->setText("N/A");
            ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
            return;
        }
        else
            if (result["success"]=="true")
            {
                boatIsValid=true;
                QVariantMap profile=result["profile"].toMap();
                ui->labelDisplayBoatName->setText(profile["OWN"].toString());
                if (raceIsValid)
                   ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
            }
            else
            {
                boatIsValid=false;
                ui->labelDisplayBoatName->setText("N/A");
            }
        break;
    }
    }
}

