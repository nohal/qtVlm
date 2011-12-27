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

#define VLM_DL_TRACK 1
#define VLM_RACE_INFO 2
#define VLM_GET_TRACK 3

DialogDownloadTracks::DialogDownloadTracks(MainWindow * main ,myCentralWidget * parent,inetConnexion * inet) :
    QDialog(parent),
    inetClient(inet),
    ui(new Ui::DialogDownloadTracks)
{
    this->parent=parent;
    ui->setupUi(this);
    this->raceIsValid=false;
    ui->raceIDEdit->setToolTip(tr("Numero de la course\n http://www.virtual-loup-de-mer.org/races.php?fulllist=1","",0));
    ui->boatIDEdit->setToolTip(tr("Numero du bateau","",0));
    ui->startTimeEdit->setToolTip(tr("Debut de la trace","",0));
    ui->endTimeEdit->setToolTip(tr("Fin de la trace","",0));
}

DialogDownloadTracks::~DialogDownloadTracks()
{
    delete ui;
}


void DialogDownloadTracks::accept()
{
        raceID=ui->raceIDEdit->value();
        boatID=ui->boatIDEdit->value();
        QDateTime qStartTime=ui->startTimeEdit->dateTime();
        startTime=qStartTime.toTime_t();
        QDateTime qEndTime=ui->endTimeEdit->dateTime();
        endTime=qEndTime.toTime_t();
        if (!raceIsValid)
        {
            QMessageBox msgBox;
            msgBox.setText(tr("Course inconnue"));
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.exec();
        }
        else
        {
        if (!doRequest(VLM_GET_TRACK))
            QDialog::done(QDialog::Rejected);
        else
            QDialog::done(QDialog::Accepted);
        }
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
    case VLM_DL_TRACK:
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
        inetGet(VLM_DL_TRACK,page);
        qWarning()<<"Sending Track Request: "<<page;
        break;
    case VLM_GET_TRACK:
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
    }
    return true;
}

void DialogDownloadTracks::requestFinished (QByteArray data)
{
    int nb;
    switch(getCurrentRequest())
    {
    case VLM_DL_TRACK:
        if(fileName.isEmpty())
        {
            qWarning() << "Empty file name in VLM track save";
        }
            if(!jsonFileReceived(&data))
            {
                qWarning() << "Error when receiving json";
                //showDialog();
             }
        break;
    case VLM_GET_TRACK:
    {
        QJson::Parser parser;
        bool ok;
        QVariantMap result=parser.parse (data, &ok).toMap();
        if (!ok) {
            qWarning() << "Error parsing json data " << data;
            qWarning() << "Error: " << parser.errorString() << " (line: " << parser.errorLine() << ")";
        }
        QVariant trackRaw=result["tracks"];
        QList<QVariant> details=trackRaw.toList();
        QString routeName;
        routeName=routeName.sprintf("%d_%d_",raceID,boatID);
        parent->withdrawRouteFromBank(routeName,details);
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
            qWarning() << "Error parsing json data " << data;
            qWarning() << "Error: " << parser.errorString() << " (line: " << parser.errorLine() << ")";
        }
        if (result.contains("racename"))
        {
             raceIsValid=true;
             ui->labelDisplayRaceName->setText(result["racename"].toString());
             QDateTime tempTime;
             tempTime.setTimeSpec(Qt::UTC);
             tempTime.setTime_t(result["deptime"].toInt());
             ui->startTimeEdit->setMinimumDateTime(tempTime);
             ui->startTimeEdit->setDateTime(tempTime);
             ui->endTimeEdit->setMinimumDateTime(tempTime);
             ui->endTimeEdit->setDateTime(tempTime);
        }
        else
        {
            raceIsValid=false;
            ui->labelDisplayRaceName->setText("N/A");
        }
        break;
    }
    }
}

bool DialogDownloadTracks::jsonFileReceived ( QByteArray * content)
{
    QString trackPath = Settings::getSetting("edtTrackFolder", "tracks").toString();
    QDir dirTrack (trackPath);
    if (!dirTrack.exists())
    {
        trackPath=QApplication::applicationDirPath()+"/tracks";
        Settings::setSetting("askTrackFolder",1);
        Settings::setSetting("edtTrackFolder",trackPath);
    }
    fileName=trackPath+"/"+fileName;
    if(Settings::getSetting("askTrackFolder",1)==1)
    {
        fileName = QFileDialog::getSaveFileName(this,
                         tr("Sauvegarde du fichier JSON"), fileName, "Tracks (*.json)");
    }

    if (fileName != "")
    {
        QFile *saveFile = new QFile(fileName);
        assert(saveFile);
        if (saveFile->open(QIODevice::WriteOnly))
        {
            int nb=saveFile->write(*content);
            if(nb>0)
                saveFile->close();
            //qWarning() << nb << " bytes saved in " << fileName;
            return true;
        }
        else
        {
            QMessageBox::critical (this,
                    tr("Erreur"),
                    tr("Ecriture du fichier impossible."));
        }
    }
    return false;
}
