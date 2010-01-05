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

#include "boatAccount.h"
#include "Util.h"
#include "vlmLine.h"

#define VLM_NO_REQUEST  -1
#define VLM_REQUEST_IDU  0
#define VLM_REQUEST_BOAT 1
#define VLM_REQUEST_TRJ  2

/**************************/
/* Init & Clean           */
/**************************/

boatAccount::boatAccount(QString login, QString pass, bool activated,Projection * proj,
                         MainWindow * main, myCentralWidget *parent) : QGraphicsWidget()
{
    this->mainWindow=main;
    this->parent=parent;

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

    width=height=0;

    setZValue(Z_VALUE_BOAT);
    setFont(QFont("Helvetica",9));
    setData(0,BOAT_WTYPE);

    fgcolor = QColor(0,0,0);
    int gr = 255;
    bgcolor = QColor(gr,gr,gr,150);

    estimeLine = new orthoSegment(proj,parent->getScene(),Z_VALUE_ESTIME,true);
    WPLine = new orthoSegment(proj,parent->getScene(),Z_VALUE_ESTIME);

    trace.clear();
    trace_drawing = new vlmLine(proj,parent->getScene(),Z_VALUE_BOAT);

    this->proj = proj;

    createPopUpMenu();

    slot_paramChanged();

    if(activated)
        show();

    connect(ac_select,SIGNAL(triggered()),this,SLOT(slot_selectBoat()));
    connect(ac_estime,SIGNAL(triggered()),this,SLOT(slot_toggleEstime()));

    connect(this,SIGNAL(boatSelected(boatAccount*)),main,SLOT(slotSelectBoat(boatAccount*)));
    connect(this,SIGNAL(boatUpdated(boatAccount*,bool)),main,SLOT(slotBoatUpdated(boatAccount*,bool)));
    connect(this,SIGNAL(boatLockStatusChanged(boatAccount*,bool)),
            main,SLOT(slotBoatLockStatusChanged(boatAccount*,bool)));

    connect(main,SIGNAL(paramVLMChanged()),this,SLOT(slot_paramChanged()));

    connect(this,SIGNAL(getPolar(QString,Polar**)),main,SLOT(getPolar(QString,Polar**)));
    connect(this,SIGNAL(releasePolar(QString)),main,SLOT(releasePolar(QString)));

    connect(this,SIGNAL(validationDone(bool)),main,SLOT(slotValidationDone(bool)));

    /* timer used for validating modification to boat param (from BoardVLM)*/
    timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer,SIGNAL(timeout()),this, SLOT(slot_getData()));

    /* init http inetManager */
    conn=new inetConnexion(main,(QWidget*)this);

    setParam(login,pass,activated);
}

boatAccount::~boatAccount()
{
    disconnect();
    if(polarData)
        emit releasePolar(polarData->getName());
    if(estimeLine)
    {
        //estimeLine->hideSegment();
        //delete estimeLine;
    }

    if(WPLine)
    {
        //WPLine->hideSegment();
        //delete WPLine;
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
    connect(ac_compassLine,SIGNAL(triggered()),mainWindow,SLOT(slotCompassLine()));
}

/**************************/
/* Data request to VLM    */
/**************************/

void boatAccount::slot_getData(void)
{
    updating=true;
    trace_drawing->deleteAll();
    doRequest(VLM_REQUEST_BOAT);
}

void boatAccount::doRequest(int requestCmd)
{
    if(!activated)
    {
        updating=false;
        return;
    }

    if(conn)
    {
        if(!conn->isAvailable() )
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
                if(boat_id==-1)
                {
                    qWarning() << "boat Acc no std request : Boat Id = -1 for:" << login ;
                    doRequest(VLM_REQUEST_IDU);
                    return;
                }
                QTextStream(&page) << "/getinfo.php?"
                            << "pseudo=" << login
                            << "&password=" << pass
                            << "&idu="<< boat_id
                            ;
                break;
            case VLM_REQUEST_IDU:
                QTextStream(&page) << "/getinfo2.php?"
                            << "pseudo=" << login
                            << "&password=" << pass
                            ;
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
        }

        slot_requestFinished(requestCmd,conn->doRequestGet(requestCmd,page));
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

void boatAccount::slot_requestFinished ( int currentRequest,QByteArray res_byte)
{
    //-------------------------------------------
    // Retour de l'étape 1 : préparation du fichier
    //-------------------------------------------
//    if(res_byte=="error") {
//        qWarning()<<"error in received QByteArray";
//        return;}
    QStringList lsbuf;
    float latitude=0,longitude=0;

    QString res(res_byte);

    switch(currentRequest)
    {
        case VLM_REQUEST_IDU:
            lsbuf = res.split(";");
            boat_id=-1;
            for (int i=0; i < lsbuf.size(); i++)
            {
                QStringList lsval = lsbuf.at(i).split("=");
                if (lsval.size() == 2) {
                    if (lsval.at(0) == "IDU")
                    {
                        boat_id = lsval.at(1).toInt();
                        break;
                    }
                }
            }
            if(boat_id!=-1)
            {
                doRequest(VLM_REQUEST_BOAT);
            }
            else
            {
                QMessageBox::warning(0,QObject::tr("Paramètre bateau"),
                              QString("Erreur de parametrage du bateau '")+
                              login+"'.\n Verifier le login et mot de passe puis reactivez le bateau");
                setStatus(false);
                emit boatUpdated(this,0);
            }
            break;
        case VLM_REQUEST_BOAT:
            hasPilototo=false;
            newRace=false;
            vacLen=300;
            pilototo.clear();

            if(res.length()<5)
            {
                QMessageBox::warning(0,QObject::tr("Bateau au ponton"),
                              QString("Le bateau '")+login+"' a ete desactive car hors course");
                setStatus(false);
                emit boatUpdated(this,0);
                break;
            }

            for(int i=0;i<5;i++)
                pilototo.append("none");

            if(res.length()<5)
            {
                QMessageBox::warning(0,QObject::tr("Bateau au ponton"),
                              QString("Le bateau '")+login+"' a ete desactive car hors course");
                setStatus(false);
                emit boatUpdated(this,0);
                break;
            }

            lsbuf = res.split("\n");

            for (int i=0; i < lsbuf.size(); i++)
            {
                QStringList lsval = lsbuf.at(i).split("=");
                if (lsval.size() == 2) {
                    if (lsval.at(0) == "IDU")
                        boat_id = lsval.at(1).toInt();
                    else if (lsval.at(0) == "IDB")
                        boat_name = lsval.at(1);
                    else if (lsval.at(0) == "RAC")
                    {
                        if(race_id != lsval.at(1).toInt())
                            newRace=true;
                        race_id = lsval.at(1).toInt();
                        if(race_id==0)
                        {                            
                            latitude = longitude = speed = heading = avg = 0;
                            dnm = loch = ortho = loxo = vmg = windDir = 0;
                            windSpeed = WPLat = WPLon = TWA = prevVac = 0;
                            nextVac = 0;
                            race_name = "";
                            WPHd = -1;
                            pilotType = 1;
                            pilotString = "";
                            score = "";
                            hasPilototo=false;
                        }
                    }
                    else if (lsval.at(0) == "LAT")
                        latitude = lsval.at(1).toFloat();
                    else if (lsval.at(0) == "LON")
                        longitude = lsval.at(1).toFloat();
                    else if (lsval.at(0) == "RAN")
                        race_name = lsval.at(1);
                    else if (lsval.at(0) == "BSP")
                        speed = lsval.at(1).toFloat();
                    else if (lsval.at(0) == "HDG")
                        heading = lsval.at(1).toFloat();
                    else if (lsval.at(0) == "AVG")
                        avg = lsval.at(1).toFloat();
                    else if (lsval.at(0) == "DNM")
                        dnm = lsval.at(1).toFloat();
                    else if (lsval.at(0) == "LOC")
                        loch = lsval.at(1).toFloat();
                    else if (lsval.at(0) == "ORT")
                        ortho = lsval.at(1).toFloat();
                    else if (lsval.at(0) == "LOX")
                        loxo = lsval.at(1).toFloat();
                    else if (lsval.at(0) == "VMG")
                        vmg = lsval.at(1).toFloat();
                    else if (lsval.at(0) == "TWD")
                        windDir = lsval.at(1).toFloat();
                    else if (lsval.at(0) == "TWS")
                        windSpeed = lsval.at(1).toFloat();
                    else if (lsval.at(0) == "WPLAT")
                        WPLat = lsval.at(1).toFloat();
                    else if (lsval.at(0) == "WPLON")
                        WPLon = lsval.at(1).toFloat();
                    else if (lsval.at(0) == "H@WP")
                        WPHd = lsval.at(1).toFloat();
                    else if(lsval.at(0) == "PIM")
                        pilotType = lsval.at(1).toInt();
                    else if(lsval.at(0) == "PIP")
                        pilotString = lsval.at(1);
                        if(pilotString == "0.0000,0.0000")
                            pilotString=tr("Prochaine balise");
                    else if(lsval.at(0) == "TWA")
                        TWA = lsval.at(1).toFloat();
                    else if(lsval.at(0) == "ETA")
                        ETA = lsval.at(1);
                    else if(lsval.at(0) == "POS")
                        score = lsval.at(1);
                    else if(lsval.at(0) == "LUP")
                        prevVac = lsval.at(1).toInt();
                    else if(lsval.at(0) == "NUP")
                        nextVac = lsval.at(1).toInt();
                    else if(lsval.at(0) == "PIL1")
                    {
                        hasPilototo=true;
                        pilototo[0] = lsval.at(1);
                    }
                    else if(lsval.at(0) == "PIL2")
                    {
                        hasPilototo=true;
                        pilototo[1] = lsval.at(1);
                    }
                    else if(lsval.at(0) == "PIL3")
                    {
                        hasPilototo=true;
                        pilototo[2] = lsval.at(1);
                    }
                    else if(lsval.at(0) == "PIL4")
                    {
                        hasPilototo=true;
                        pilototo[3] = lsval.at(1);
                    }
                    else if(lsval.at(0) == "PIL5")
                    {
                        hasPilototo=true;
                        pilototo[4] = lsval.at(1);
                    }
                    else if(lsval.at(0) == "POL")
                        polarVlm = lsval.at(1);
                    else if(lsval.at(0) == "EML")
                        email = lsval.at(1);
                    else if(lsval.at(0) == "VAC")
                        vacLen = lsval.at(1).toInt();
                }
            }

            /*if(pilotType==5 && pilotString.isEmpty())
                pilotString=tr("Prochaine balise");*/

            lat = latitude/1000;
            lon = longitude/1000;

            current_heading = heading;
            //qWarning() << "Data for " << boat_id << " received" << " - vac len " << vacLen;

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
                        emit boatUpdated(this,newRace);
                        emit validationDone(false);
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
                emit boatUpdated(this,newRace);
            }
            else
                doRequest(VLM_REQUEST_TRJ);
            break;
        case VLM_REQUEST_TRJ:
            emit getTrace(res,&trace);
            trace_drawing->setPoly(trace);

            /* we can now update everything */
            updateBoatData();
            updateTraceColor();
            updating=false;
            emit boatUpdated(this,newRace);
            break;
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
    slot_getData();
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
            if(compFloat(getWPLat(),valid_val1) &&
                compFloat(getWPLon(),valid_val2) &&
                compFloat(getWPHd(),valid_val3))
                return true;
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

    QFontMetrics fm(font());

    pnt->fillRect(9,0, width-10,height-1, QBrush(bgcolor));
    pnt->setFont(font());
    pnt->drawText(10,fm.height()-2,my_str);

    QPen pen(selected?selColor:myColor);
    pen.setWidth(4);
    pnt->setPen(pen);
    pnt->fillRect(0,dy-3,7,7, QBrush(selected?selColor:myColor));

    int g = 60;
    pen = QPen(QColor(g,g,g));
    pen.setWidth(1);
    pnt->setPen(pen);
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
    trace_drawing->slot_showMe();
}

void boatAccount::drawEstime(void)
{
    estimeLine->hideSegment();
    WPLine->hideSegment();
    /*should we draw something?*/
    if(isUpdating() || !getStatus())
        return;

    QPen penLine1(QColor(Util::getSetting("estimeLineColor", QColor(Qt::darkMagenta)).value<QColor>()),1,Qt::SolidLine);
    penLine1.setWidthF(Util::getSetting("estimeLineWidth", 1.6).toDouble());
    QPen penLine2(QColor(Qt::black),1,Qt::DotLine);
    penLine2.setWidthF(1.2);

    int estime_param_2;
    int i1,j1,i2,j2;
    float estime;

    /* draw estime */
    if(getIsSelected() || getForceEstime())
    {
        float tmp_lat,tmp_lon;

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

    setToolTip(str);
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
    myColor = QColor(Util::getSetting("qtBoat_color",QColor(Qt::blue).name()).toString());
    selColor = QColor(Util::getSetting("qtBoat_sel_color",QColor(Qt::red).name()).toString());
    estime_type=Util::getSetting("estimeType",0).toInt();
    switch(estime_type)
    {
        case 0: /* time */
            estime_param = Util::getSetting("estimeTime",60).toInt();
            break;
        case 1: /* nb vac */
            estime_param = Util::getSetting("estimeVac",10).toInt();
            break;
        default: /* dist */
            estime_param = Util::getSetting("estimeLen",100).toInt();
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

void boatAccount::mousePressEvent(QGraphicsSceneMouseEvent * e)
{
    if (e->button() != Qt::LeftButton)
    {
         e->ignore();
    }
}

void  boatAccount::mouseReleaseEvent(QGraphicsSceneMouseEvent * e)
{
    if(e->button() == Qt::LeftButton)
    {
        emit clearSelection();
        if(!mainWindow->get_selPOI_instruction())
        {
            slot_selectBoat();
            return;
        }
    }
}

void boatAccount::contextMenuEvent(QGraphicsSceneContextMenuEvent * e)
{
    bool onlyLineOff = false;

    switch(parent->getCompassMode((float)e->scenePos().x(),(float)e->scenePos().y()))
    {
        case 0:
            /* not showing menu line, default text*/
            ac_compassLine->setText("Tirer un cap");
            ac_compassLine->setEnabled(false);
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
