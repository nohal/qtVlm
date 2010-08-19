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
***********************************************************************/

#include <QDebug>
#include <QtGui>

#include "dataDef.h"
#include "boatAccount.h"
#include "Projection.h"
#include "Util.h"
#include "vlmLine.h"
#include "parser.h"
#include "settings.h"
#include "mycentralwidget.h"
#include "orthoSegment.h"
#include "MainWindow.h"
#include "Polar.h"

/**************************/
/* Init & Clean           */
/**************************/

boatAccount::boatAccount(QString login, QString pass, bool activated,Projection * proj,
                         MainWindow * main, myCentralWidget *parent,inetConnexion * inet) : QGraphicsWidget(), inetClient(inet)
{
    this->mainWindow=main;
    this->parent=parent;

    inetClient::setName(login);
    boat_name="NO NAME";
    boat_id=-1;
    race_id=0;
    isSync = false;
    selected = false;
    polarName="";
    polarData=NULL;
    changeLocked=false;
    pilototo.clear();
    hasPilototo=false;
    polarVlm="";
    forcePolar=false;
    alias="";
    useAlias=false;
    forceEstime=false;
    updating=false;
    doingValidation=false;
    vacLen=300;
    doingSync=false;
    gatesLoaded=false;
    width=height=0;
    nWP=0;
    this->porteHidden=parent->get_shPor_st();
    firstSynch=false;

    needAuth=true;

    setZValue(Z_VALUE_BOAT);
    setFont(QFont("Helvetica",9));
    setData(0,BOAT_WTYPE);

    fgcolor = QColor(0,0,0);
    int gr = 255;
    bgcolor = QColor(gr,gr,gr,150);

    estimeLine = new orthoSegment(proj,parent->getScene(),Z_VALUE_ESTIME,true);
    WPLine = new orthoSegment(proj,parent->getScene(),Z_VALUE_ESTIME);
    connect(parent, SIGNAL(shPor(bool)),this,SLOT(slot_shPor()));
    connect(parent, SIGNAL(showALL(bool)),this,SLOT(slot_shSall()));
    connect(parent, SIGNAL(hideALL(bool)),this,SLOT(slot_shHall()));

    trace.clear();
    trace_drawing = new vlmLine(proj,parent->getScene(),Z_VALUE_BOAT);

    this->proj = proj;
    this->labelHidden=parent->get_shLab_st();

    createPopUpMenu();

    connect(this,SIGNAL(boatUpdated(boatAccount*,bool,bool)),main,SIGNAL(updateRoute()));
    connect(parent, SIGNAL(shLab(bool)),this,SLOT(slot_shLab()));
    slot_paramChanged();

    if(activated)
        show();

    connect(ac_select,SIGNAL(triggered()),this,SLOT(slot_selectBoat()));
    connect(ac_estime,SIGNAL(triggered()),this,SLOT(slot_toggleEstime()));

    connect(this,SIGNAL(boatSelected(boatAccount*)),main,SLOT(slotSelectBoat(boatAccount*)));
    connect(this,SIGNAL(boatUpdated(boatAccount*,bool,bool)),main,SLOT(slotBoatUpdated(boatAccount*,bool,bool)));
    connect(this,SIGNAL(boatLockStatusChanged(boatAccount*,bool)),
            main,SLOT(slotBoatLockStatusChanged(boatAccount*,bool)));

    connect(main,SIGNAL(paramVLMChanged()),this,SLOT(slot_paramChanged()));

    connect(this,SIGNAL(getPolar(QString,Polar**)),main,SLOT(getPolar(QString,Polar**)));
    connect(this,SIGNAL(releasePolar(QString)),main,SLOT(releasePolar(QString)));

    connect(this,SIGNAL(validationDone(bool)),main,SLOT(slotValidationDone(bool)));

    /* timer used for validating modification to boat param (from BoardVLM)*/
    timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer,SIGNAL(timeout()),this, SLOT(slot_getDataTrue()));

    setParam(login,pass,activated);
}

boatAccount::~boatAccount()
{
    disconnect();
    if(polarData)
        emit releasePolar(polarData->getName());
    if(gatesLoaded)
    {
        vlmLine * porte;
        QListIterator<vlmLine*> i (gates);
        while(i.hasNext())
        {
            porte=i.next();
            //parent->getScene()->removeItem(porte);
            if(porte)
            {
                if(!parent->getAboutToQuit())
                    delete porte;
            }
        }
    }
}

void boatAccount::createPopUpMenu(void)
{
    popup = new QMenu(parent);

    ac_select = new QAction("Selectionner",popup);
    popup->addAction(ac_select);

    ac_estime = new QAction("Afficher estime",popup);
    popup->addAction(ac_estime);

    ac_compassLine = new QAction(tr("Tirer un cap"),popup);
    popup->addAction(ac_compassLine);
    connect(ac_compassLine,SIGNAL(triggered()),this,SLOT(slotCompassLine()));
    connect (this,SIGNAL(compassLine(int,int)),mainWindow,SLOT(slotCompassLineForced(int,int)));

    ac_twaLine = new QAction(tr("Tracer une estime TWA"),popup);
    popup->addAction(ac_twaLine);
    connect(ac_twaLine,SIGNAL(triggered()),this,SLOT(slotTwaLine()));
}

/**************************/
/* Data request to VLM    */
/**************************/

void boatAccount::slot_getData(bool doingSync)
{    
    updating=true;
    trace_drawing->deleteAll();
    doRequest(VLM_REQUEST_BOAT);
    this->doingSync=doingSync;
}
void boatAccount::slot_getDataTrue()
{
    slot_getData(true);
}

void boatAccount::tryWs(void)
{
    QString page;

    QTextStream(&page) << "/ws/boatinfo.php?forcefmt=json";

    inetGet(VLM_REQUEST_WS,page);
}


void boatAccount::doRequest(int requestCmd)
{
    if(!activated)
    {
        updating=false;
        return;
    }

    if(hasInet())
    {
        if(hasRequest() )
        {
            qWarning() << "request already running for " << login;
            return;
        }
        else
            qWarning() << "Doing request " << requestCmd << " for " << login  ;

        QString page;

        switch(requestCmd)
        {
            case VLM_REQUEST_BOAT:
                QTextStream(&page) << "/ws/boatinfo.php?forcefmt=json";
                break;
            case VLM_REQUEST_TRJ:
                if(race_id==0)
                {
                    qWarning() << "boat Acc no request TRJ for:" << login << " id=" << boat_id;
                    return;
                }
                QTextStream(&page) << "/gmap/index.php?"
                            << "type=ajax&riq=trj"
                            << "&idusers="
                            << boat_id
                            << "&idraces="
                            << race_id;
                break;
            case VLM_REQUEST_GATE:
                QTextStream(&page) << "/ws/raceinfo.php?idrace="<<race_id;
                break;
             default:
                qWarning() << "[boatAccount-doRequest] error: unknown request: " << requestCmd;
                break;
        }

        inetGet(requestCmd,page);
    }
    else
    {
         qWarning("Creating dummy data");
         boat_id = 0;
         boat_name = "test";
         race_id = 0;
         lat = 0;
         lon = 0;
         vacLen=300;
         race_name = "Test race";
         updating=false;
         updateBoatData();
    }
}

void boatAccount::authFailed(void)
{
    QMessageBox::warning(0,QObject::tr("Paramètre bateau"),
                  QString("Erreur de parametrage du bateau '")+
                  login+"'.\n Verifier le login et mot de passe puis reactivez le bateau");
    setStatus(false);
    emit boatUpdated(this,false,doingSync);
    inetClient::authFailed();
}

#define printData(JSON,DATA) qWarning() << DATA << JSON[DATA].toString();

void boatAccount::requestFinished (QByteArray res_byte)
{
    QStringList lsbuf;
    double latitude=0,longitude=0;

    QString res(res_byte);

    switch(getCurrentRequest())
    {             
        case VLM_REQUEST_WS:
            break;

        case VLM_REQUEST_BOAT:
            {
                QJson::Parser parser;
                bool ok;

                QVariantMap result = parser.parse (res_byte, &ok).toMap();
                if (!ok) {
                    qWarning() << "Error parsing json data " << res_byte;
                    qWarning() << "Error: " << parser.errorString() << " (line: " << parser.errorLine() << ")";
                }
                
                //qWarning() << "reqest res for " << login << ": " << result["IDU"].toInt() << ", " << result["IDB"].toString();

                hasPilototo=false;
                newRace=false;
                vacLen=300;
                pilototo.clear();
                for(int i=0;i<5;i++)
                    pilototo.append("none");

                if(result["RAC"].toInt() == 0)
                {
                    QMessageBox::warning(0,QObject::tr("Bateau au ponton"),
                                         QString("Le bateau '")+login+"' a ete desactive car hors course");
                    setStatus(false);
                    emit boatUpdated(this,false,doingSync);
                    emit hasFinishedUpdating();
                    break;
                }

                if(race_id != result["RAC"].toInt())
                    newRace=true;

                boat_id     = result["IDU"].toInt();
                race_id     = result["RAC"].toInt();
                boat_name   = result["IDB"].toString();
                latitude    = result["LAT"].toDouble();
                longitude   = result["LON"].toDouble();
                race_name   = result["RAN"].toString();
                speed       = (float) result["BSP"].toDouble();
                heading     = (float) result["HDG"].toDouble();
                avg         = (float) result["AVG"].toDouble();
                dnm         = (float) result["DNM"].toDouble();
                loch        = (float) result["LOC"].toDouble();
                ortho       = (float) result["ORT"].toDouble();
                loxo        = (float) result["LOX"].toDouble();
                vmg         = (float) result["VMG"].toDouble();
                windDir     = (float) result["TWD"].toDouble();
                windSpeed   = (float) result["TWS"].toDouble();
                WPLat       = result["WPLAT"].toDouble();
                WPLon       = result["WPLON"].toDouble();
                WPHd        = (float) result["H@WP"].toDouble();
                pilotType   = result["PIM"].toInt();
                pilotString = result["PIP"].toString();                
                TWA         = (float) result["TWA"].toDouble();
                ETA         = result["ETA"].toString();
                score       = result["POS"].toString();
                prevVac     = result["LUP"].toUInt();
                nextVac     = result["NUP"].toUInt();
                nWP         = result["NWP"].toInt();
                pilototo[0] = result["PIL1"].toString();
                pilototo[1] = result["PIL2"].toString();
                pilototo[2] = result["PIL3"].toString();
                pilototo[3] = result["PIL4"].toString();
                pilototo[4] = result["PIL5"].toString();
                polarVlm = result["POL"].toString();
                email = result["EML"].toString();
                vacLen = result["VAC"].toInt();

                //qWarning() << "WPL=" << result["WPL"].toString();

                hasPilototo=true;

                lat = latitude/1000;
                lon = longitude/1000;

                current_heading = heading;

                if(doingValidation)
                {
                    qWarning() << "Validating Board cmd for " << login;
                    if(chkResult())
                    {
                        doingValidation=false;
                        emit validationDone(true);
                    }
                    else
                    {
                        valid_nbRetry++;
                        if(valid_nbRetry>MAX_RETRY)
                        {
                            qWarning("Failed to synch");
                            doingValidation=false;
                            updateBoatData();
                            emit boatUpdated(this,newRace,doingSync);
                            emit validationDone(false);
                            emit hasFinishedUpdating();
                            updating=false;
                        }
                        else
                        {
                            qWarning("Retry...");
                            timer->start(1000);
                            updating=false;
                            break;
                        }
                    }
                }

                /* request trace points */
                if(race_id==0)
                {
                    /* clearing trace */
                    trace.clear();
                    trace_drawing->deleteAll();
                    /*updating everything*/
                    updateBoatData();
                    updating=false;
                    QMessageBox::warning(0,QObject::tr("Bateau au ponton"),
                                         QString("Le bateau '")+login+"' a ete desactive car hors course");
                    setStatus(false);
                    emit boatUpdated(this,newRace,doingSync);
                    emit hasFinishedUpdating();
                }
                else
                {
                    //updateBoatData();
                    //emit boatUpdated(this,newRace,doingSync);
                    //emit hasFinishedUpdating();
                    //updating=false;
                    doRequest(VLM_REQUEST_TRJ);
                }
            }
            break;
        case VLM_REQUEST_TRJ:
            emit getTrace(res,&trace);
            trace_drawing->setPoly(trace);

            /* we can now update everything */
            updateBoatData();
            updateTraceColor();            

            if(race_id!=0 && !gatesLoaded)
            {
                updating=false;
                drawEstime();
                updating=true;
                doRequest(VLM_REQUEST_GATE);
            }
            else
            {
                updating=false;
                drawEstime();
                emit hasFinishedUpdating();
                emit boatUpdated(this,newRace,doingSync);
            }
            showNextGates();
            break;
        case VLM_REQUEST_GATE:
            {
                QJson::Parser parser;
                bool ok;

                QVariantMap result = parser.parse (res_byte, &ok).toMap();
                if (!ok) {
                    qWarning() << "Error parsing json data " << res_byte;
                    qWarning() << "Error: " << parser.errorString() << " (line: " << parser.errorLine() << ")";
                }

                qWarning() << "id Race: " << result["idraces"].toString();
                qWarning() << "race name: " << result["racename"].toString();

                QVariantMap wps= result["races_waypoints"].toMap();
                for(int i=1;true;i++)
                {
                    QString str;
                    QVariantMap wp= wps[str.setNum(i)].toMap();
                    if(wp.isEmpty()) break;
                    qWarning()<<"wp: "<<wp["libelle"].toString();
                    double lonPorte1=wp["longitude1"].toDouble()/1000.000;
                    double latPorte1=wp["latitude1"].toDouble()/1000.000;
                    double lonPorte2=wp["longitude2"].toDouble()/1000.000;
                    double latPorte2=wp["latitude2"].toDouble()/1000.000;
                    vlmLine * porte=new vlmLine(proj,parent->getScene(),Z_VALUE_GATE);
                    connect(parent, SIGNAL(shLab(bool)),porte,SLOT(slot_shLab()));
                    porte->setGateMode("WP"+str+": "+wp["libelle"].toString());
                    porte->addPoint(latPorte1,lonPorte1);
                    if(qRound(lonPorte1*1000)==qRound(lonPorte2*1000) && qRound(latPorte1*1000)==qRound(latPorte2*1000))
                    {
                        porte->setPorteOnePoint();
                        Util::getCoordFromDistanceAngle(latPorte1, lonPorte1, 500,wp["laisser_au"].toDouble()+180,&latPorte2,&lonPorte2);
                    }
                    porte->addPoint(latPorte2,lonPorte2);
                    QPen penLine(Qt::black,1);
                    penLine.setWidthF(3);
                    porte->setLinePen(penLine);
                    porte->setHidden(true);
                    gates.append(porte);

                }
                gatesLoaded=true;
                updating=false;
                emit hasFinishedUpdating();
                emit boatUpdated(this,newRace,doingSync);
                showNextGates();
            }
            break;
        default:
            qWarning() << "[boatAccount-requestFinished] error: unknown request: " << getCurrentRequest();
            break;
    }
#if 0 //unflag to check b-vmg
    double res_h,res_wangle;
    if(this->polarData && lon != 0 && lat != 0 && WPLon != 0 && WPLat != 0)
    {
        this->polarData->bvmg(this->lon,this->lat,this->WPLon,this->WPLat,this->windDir,this->windSpeed,0,&res_h,&res_wangle);
        qWarning()<<"Cap BVMG: "<<res_h<<" TWA BVMG: "<<res_wangle;
    }
#endif
}

void boatAccount::inetError()
{
    updating=false;
    //emit hasFinishedUpdating();
    //emit boatUpdated(this,newRace,doingSync);
}

void boatAccount::showNextGates()
{
    if(!gatesLoaded) return;
    vlmLine * porte;
    QListIterator<vlmLine*> i (gates);
    int j=0;
    while(i.hasNext())
    {
        porte=i.next();
        if(!selected || porteHidden)
        {
            porte->setHidden(true);
            porte->hide();
            continue;
        }
        porte->show();
        j++;
        if(j<nWP)
        {
            porte->setHidden(true);
            porte->slot_showMe();
        }
        else if (j==nWP)
        {
            QPen penLine(Qt::blue,1);
            penLine.setWidthF(3);
            porte->setLinePen(penLine);
            porte->setHidden(false);
            porte->slot_showMe();
        }
        else
        {
            QPen penLine(Qt::white,1);
            penLine.setWidthF(3);
            porte->setLinePen(penLine);
            porte->setHidden(false);
            porte->slot_showMe();
        }
    }

}
void boatAccount::validateChg(int currentCmdNum,float cmd_val1,float cmd_val2,float cmd_val3)
{
    timer->stop();
    valid_cmd=currentCmdNum;
    valid_val1=cmd_val1;
    valid_val2=cmd_val2;
    valid_val3=cmd_val3;
    valid_nbRetry=0;
    doingValidation=true;
    slot_getData(true);
}

bool boatAccount::chkResult(void)
{
    float data;
    float val;

    switch(valid_cmd)
    {
        case VLM_CMD_HD:
            data=getHeading();
            if(compFloat(data,valid_val1) && getPilotType() == 1)
                return true;
            break;
        case VLM_CMD_ANG:
            data = getTWA();
            val=getHeading()-getWindDir();
            calcAngleSign(val,data);

            if(((int)data) == ((int)valid_val1) && getPilotType() == 2)
                return true;
            break;
        case VLM_CMD_VMG:
            if(getPilotType() == 4)
                return true;
            break;
        case VLM_CMD_VBVMG:
            if(getPilotType() == 5)
                return true;
            break;
        case VLM_CMD_ORTHO:
            if(getPilotType() == 3)
                return true;
            break;
        case VLM_CMD_WP:
            
            if(compDouble(getWPLat(),valid_val1) &&
                compDouble(getWPLon(),valid_val2) &&
                compFloat(getWPHd(),valid_val3))
                return true;
            else
            {
                qWarning()<<"lat: "<<getWPLat()<<" against "<<valid_val1 << "diff " << getWPLat()-valid_val1;
                qWarning()<<"lon: "<<getWPLon()<<" against "<<valid_val2 << "diff " << getWPLon()-valid_val2;
                qWarning()<<"lon: "<<getWPHd()<<" against "<<valid_val3 << "diff " << getWPHd()-valid_val3;
            }
            break;
    }
    return false;
}

/**************************/
/* Select / unselect      */
/**************************/

void boatAccount::slot_selectBoat()
{
    selected = true;
    updateTraceColor();
    emit boatSelected(this);
    showNextGates();
}

void boatAccount::unSelectBoat(bool needUpdate)
{
    selected = false;
    if(needUpdate)
    {
        drawEstime();
        update();
        updateTraceColor();
    }
    showNextGates();
}

/**************************/
/* data access            */
/**************************/
float boatAccount::getBvmgUp(float ws)
{
    if(polarData) return(polarData->getBvmgUp(ws));
    return -1;
}
float boatAccount::getBvmgDown(float ws)
{
    if(polarData) return(polarData->getBvmgDown(ws));
    return -1;
}

/**************************/
/* Paint & graphics       */
/**************************/

void boatAccount::slot_toggleEstime()
{
    forceEstime=!forceEstime;
    drawEstime();
    update();
}

void boatAccount::paint(QPainter * pnt, const QStyleOptionGraphicsItem * , QWidget * )
{
    int dy = height/2;
    QPen pen(selected?Qt::darkRed:Qt::darkBlue);
    pnt->setPen(pen);
    if(!labelHidden)
    {
        QFontMetrics fm(font());

        pnt->fillRect(9,0, width-10,height-1, QBrush(bgcolor));
        pnt->setFont(font());
        pnt->drawText(10,fm.height()-2,my_str);
    }
    pen.setColor(selected?selColor:myColor);
    pen.setWidth(4);
    pnt->setPen(pen);
    pnt->fillRect(0,dy-3,7,7, QBrush(selected?selColor:myColor));

    int g = 60;
    pen = QPen(QColor(g,g,g));
    pen.setWidth(1);
    pnt->setPen(pen);
    if(!labelHidden)
        pnt->drawRect(9,0,width-10,height-1);

    //drawEstime();
}

void boatAccount::updateTraceColor(void)
{
    if(selected)
    {
        QPen penLine(selColor,1);
        penLine.setWidthF(1);
        trace_drawing->setLinePen(penLine);
        trace_drawing->setPointMode(selColor);
    }
    else
    {
        QPen penLine(myColor,1);
        penLine.setWidthF(1);
        trace_drawing->setLinePen(penLine);
        trace_drawing->setPointMode(myColor);
    }
    trace_drawing->setNbVacPerHour(3600/this->vacLen);
    trace_drawing->slot_showMe();
}

void boatAccount::drawEstime(void)
{
    estimeLine->hideSegment();
    WPLine->hideSegment();
    /*should we draw something?*/
    if(isUpdating() || !getStatus())
        return;

    QPen penLine1(QColor(Settings::getSetting("estimeLineColor", QColor(Qt::darkMagenta)).value<QColor>()),1,Qt::SolidLine);
    penLine1.setWidthF(Settings::getSetting("estimeLineWidth", 1.6).toDouble());
    QPen penLine2(QColor(Qt::black),1,Qt::DotLine);
    penLine2.setWidthF(1.2);

    int estime_param_2;
    int i1,j1,i2,j2;
    float estime;

    /* draw estime */
    if(getIsSelected() || getForceEstime())
    {
        double tmp_lat,tmp_lon;

        switch(estime_type)
        {
            case 0: /* time */
                estime = (float)((float)(estime_param/float(60.0000000000)))*getSpeed();
                break;
            case 1: /* nb vac */
                estime_param_2=getVacLen();
                estime = (float)((((float)(estime_param*estime_param_2))/3660.000000000)*getSpeed());
                break;
            default: /* dist */
                estime = estime_param;
                break;
        }

        Util::getCoordFromDistanceAngle(lat,lon,estime,getHeading(),&tmp_lat,&tmp_lon);
        proj->map2screen(lon,lat,&i1,&j1);
        proj->map2screen(tmp_lon,tmp_lat,&i2,&j2);
        estimeLine->setLinePen(penLine1);
        estimeLine->initSegment(i1,j1,i2,j2);
        /* draw ortho to wp */
        if(WPLat != 0 && WPLon != 0)
        {
            WPLine->setLinePen(penLine2);
            proj->map2screen(WPLon,WPLat,&i2,&j2);
            WPLine->initSegment(i1,j1,i2,j2);
        }
    }
}

/**************************/
/* shape & boundingRect   */
/**************************/
void boatAccount::slotCompassLine()
{
    int i1,j1;
    proj->map2screen(this->lon,this->lat,&i1,&j1);
    emit compassLine(i1,j1);
}
QPainterPath boatAccount::shape() const
{
    QPainterPath path;
    path.addRect(0,0,width,height);
    return path;
}

QRectF boatAccount::boundingRect() const
{
    return QRectF(0,0,width,height);
}

/**************************/
/* Data update            */
/**************************/

void boatAccount::updateBoatData()
{
    updateBoatName();
    reloadPolar();
    updatePosition();
    updateHint();
}

void boatAccount::updateBoatName()
{    
    if(getAliasState())
        my_str=(alias.isEmpty()?login:alias);
    else
        my_str=login;

    //qWarning() << "Updating Boat name: " << login;

     /* computing widget size */
    QFontMetrics fm(font());
    prepareGeometryChange();
    width = fm.width(my_str) + 10 + 2;
    height = qMax(fm.height()+2,10);
}

void boatAccount::updatePosition(void)
{
    int boat_i,boat_j;
    Util::computePos(proj,lat,lon,&boat_i,&boat_j);
    boat_i-=3;
    boat_j-=(height/2);
    setPos(boat_i, boat_j);
    drawEstime();
}

void boatAccount::updateHint(void)
{
    QString str;
    /* adding score */
    str = getScore() + " - Spd: " + QString().setNum(getSpeed()) + " - ";
    switch(getPilotType())
    {
        case 1: /*heading*/
            str += tr("Hdg") + ": " + getPilotString() + tr("°");
            break;
        case 2: /*constant angle*/
            str += tr("Angle") + ": " + getPilotString()+ tr("°");
            break;
        case 3: /*pilotortho*/
            str += tr("Ortho" ) + "-> " + getPilotString();
            break;
        case 4: /*VMG*/
            str += tr("BVMG") + "-> " + getPilotString();
            break;
       case 5: /*VBVMG*/
            str += tr("VBVMG") /*+ "-> " + getPilotString()*/;
            break;
    }
    QString desc;
    if(!polarData) desc=tr(" (pas de polaire charg�e)");
    else if (polarData->getIsCsv()) desc=polarData->getName() + tr(" (format CSV)");
    else desc=polarData->getName() + tr(" (format POL)");
    str=str.replace(" ","&nbsp;");
    desc=desc.replace(" ","&nbsp;");
    setToolTip(desc+"<br>"+str);
}

void boatAccount::slot_projectionUpdated()
{
    if(activated)
        updatePosition();
}

void boatAccount::setStatus(bool activated)
{
     this->activated=activated;
     setVisible(activated);
     if(!activated)
     {
        WPLine->hide();
        estimeLine->hide();
        if(gatesLoaded)
        {
            vlmLine * porte;
            QListIterator<vlmLine*> i (gates);
            while(i.hasNext())
            {
                porte=i.next();
                delete porte;
            }
        }
        gatesLoaded=false;
     }

}

void boatAccount::setParam(QString login, QString pass)
{
     this->login=login;
     this->pass=pass;
     this->boat_id=-1;
     this->race_id=0;
}

void boatAccount::setParam(QString login, QString pass, bool activated)
{
    setStatus(activated);
    setParam(login,pass);
}

void boatAccount::reloadPolar(void)
{
    if(forcePolar)
    {
        if(polarName.isEmpty())
        {
            if(polarData!=NULL)
            {
                emit releasePolar(polarData->getName());
                polarData=NULL;
            }
            //qWarning() << login << " No User polar to load";
            return;
        }
        if(polarData != NULL && polarName == polarData->getName())
            return;
        if(polarData!=NULL)
            emit releasePolar(polarData->getName());
        //qWarning() << login << " Loading forced polar: " << polarName;
        if(!polarName.isEmpty())
            emit getPolar(polarName,&polarData);
        else
            polarData=NULL;
    }
    else
    {
        if(polarVlm.isEmpty())
        {
            if(polarData!=NULL)
            {
                emit releasePolar(polarData->getName());
                polarData=NULL;
            }
            //qWarning() << login << " No VLM polar to load";
            return;
        }
        if(polarData != NULL && polarVlm == polarData->getName())
            return;

        if(polarData!=NULL)
        {
            //qWarning() << login << " Old polar:" << polarData->getName();
            emit releasePolar(polarData->getName());
            polarData=NULL;
        }
        //qWarning() << login << " Loading polar: " << polarVlm;
        emit getPolar(polarVlm,&polarData);
    }
}

void boatAccount::setPolar(bool state,QString polar)
{
    this->polarName=polar;
    forcePolar=state;
    if(activated)
        reloadPolar();
    else if(polarData)
    {
        emit releasePolar(polarData->getName());
        polarData=NULL;
    }

}

void boatAccount::setLockStatus(bool status)
{
    if(status!=changeLocked)
    {
        changeLocked=status;
        emit boatLockStatusChanged(this,status);
    }
}

void boatAccount::setAlias(bool state,QString alias)
{
    useAlias=state;
    this->alias=alias;
}

void boatAccount::slot_paramChanged()
{
    myColor = QColor(Settings::getSetting("qtBoat_color",QColor(Qt::blue).name()).toString());
    selColor = QColor(Settings::getSetting("qtBoat_sel_color",QColor(Qt::red).name()).toString());
    estime_type=Settings::getSetting("estimeType",0).toInt();
    switch(estime_type)
    {
        case 0: /* time */
            estime_param = Settings::getSetting("estimeTime",60).toInt();
            break;
        case 1: /* nb vac */
            estime_param = Settings::getSetting("estimeVac",10).toInt();
            break;
        default: /* dist */
            estime_param = Settings::getSetting("estimeLen",100).toInt();
            break;
    }
    if(activated)
    {
        drawEstime();
        update();
    }
}
void boatAccount::slot_updateGraphicsParameters()
{
    if(activated)
        drawEstime();
}
/**************************/
/* Events                 */
/**************************/

//void boatAccount::mousePressEvent(QGraphicsSceneMouseEvent * e)
//{
//    if (e->button() != Qt::LeftButton)
//    {
//         e->ignore();
//    }
//}
//
//void  boatAccount::mouseReleaseEvent(QGraphicsSceneMouseEvent * e)
//{
//    if(e->button() == Qt::LeftButton)
//    {
//        emit clearSelection();
//        if(!mainWindow->get_selPOI_instruction())
//        {
//            slot_selectBoat();
//            return;
//        }
//    }
//}

void boatAccount::contextMenuEvent(QGraphicsSceneContextMenuEvent * e)
{
    bool onlyLineOff = false;

    switch(parent->getCompassMode((float)e->scenePos().x(),(float)e->scenePos().y()))
    {
        case 0:
            /* not showing menu line, default text*/
            ac_compassLine->setText("Tirer un cap");
            ac_compassLine->setEnabled(true);
            break;
        case 1:
            /* showing text for compass line off*/
            ac_compassLine->setText("Arret du cap");
            ac_compassLine->setEnabled(true);
            onlyLineOff=true;
            break;
        case 2:
        case 3:
            ac_compassLine->setText("Tirer un cap");
            ac_compassLine->setEnabled(true);
            break;
        }

    if(forceEstime)
        ac_estime->setText(tr("Cacher estime"));
    else
        ac_estime->setText(tr("Afficher estime"));

    if(onlyLineOff)
    {
        ac_select->setEnabled(false);
        ac_estime->setEnabled(false);
    }
    else
    {
        ac_select->setEnabled(!mainWindow->get_selPOI_instruction());
        ac_estime->setEnabled(!selected);
    }

    popup->exec(QCursor::pos());
}
