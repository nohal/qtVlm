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

#include "boatAccount.h"
#include "MainWindow.h"
#include "Polar.h"
#include "Util.h"

#define VLM_NO_REQUEST  -1
#define VLM_REQUEST_IDU  0
#define VLM_REQUEST_BOAT 1
#define VLM_REQUEST_TRJ  2

boatAccount::boatAccount(QString login, QString pass, bool activated,Projection * proj,QWidget * main, QWidget *parentWindow)
    : QWidget(parentWindow)
{
    this->parent=parentWindow;
    this->mainWindow=main;

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

    trace.clear();

    this->proj = proj;
    connect(parentWindow, SIGNAL(projectionUpdated()), this,
            SLOT(projectionUpdated()) );

    createPopUpMenu();

    createWidget();
    paramChanged();

    if(activated)
        show();

    connect(ac_select,SIGNAL(triggered()),this,SLOT(selectBoat()));
    connect(ac_estime,SIGNAL(triggered()),this,SLOT(toggleEstime()));

    connect(this,SIGNAL(boatSelected(boatAccount*)),main,SLOT(slotSelectBoat(boatAccount*)));
    connect(this,SIGNAL(boatUpdated(boatAccount*,bool)),main,SLOT(slotBoatUpdated(boatAccount*,bool)));
    connect(this,SIGNAL(boatLockStatusChanged(boatAccount*,bool)),
            main,SLOT(slotBoatLockStatusChanged(boatAccount*,bool)));

    connect(main,SIGNAL(paramVLMChanged()),this,SLOT(paramChanged()));

    connect(this,SIGNAL(getTrace(QString,QList<position*> *)),
             main,SLOT(slotGetTrace(QString,QList<position*> *)));

    connect(this,SIGNAL(getPolar(QString,Polar**)),main,SLOT(getPolar(QString,Polar**)));
    connect(this,SIGNAL(releasePolar(QString)),main,SLOT(releasePolar(QString)));

    connect(this,SIGNAL(validationDone(bool)),main,SLOT(slotValidationDone(bool)));

    /* timer used for validating modification to boat param (from BoardVLM)*/
    timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer,SIGNAL(timeout()),this, SLOT(getData()));

    /* init http inetManager */
    conn=new inetConnexion(main,this);

    setParam(login,pass,activated);
}

boatAccount::~boatAccount()
{
    disconnect();
    if(polarData)
        emit releasePolar(polarData->getName());
}

void boatAccount::selectBoat()
{
    selected = true;
    emit   boatSelected(this);
}

void boatAccount::unSelectBoat(bool needUpdate)
{
    selected = false;
    if(needUpdate)
        update();
}

void boatAccount::toggleEstime()
{
    forceEstime=!forceEstime;
    update();
}

void boatAccount::getData(void)
{
    updating=true;
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
            qWarning() << "request already running for " << login  << "(" << this << ")";
            //updating=false;
            return;
        }
        else
            qWarning() << "Doing request " << requestCmd << " for " << login  << "(" << this << ")";

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

        conn->doRequestGet(requestCmd,page);
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

void boatAccount::requestFinished ( int currentRequest,QByteArray res_byte)
{
    //-------------------------------------------
    // Retour de l'étape 1 : préparation du fichier
    //-------------------------------------------
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
                //qWarning() << "Get boat id " << boat_id;
                doRequest(VLM_REQUEST_BOAT);
            }
            break;
        case VLM_REQUEST_BOAT:
            hasPilototo=false;
            newRace=false;
            vacLen=300;
            pilototo.clear();
            for(int i=0;i<5;i++)
                pilototo.append("none");
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
                while (!trace.isEmpty())
                    delete trace.takeFirst();
                /*updating everything*/
                updateBoatData();
                updating=false;
                emit boatUpdated(this,newRace);
            }
            else
                doRequest(VLM_REQUEST_TRJ);
            break;
        case VLM_REQUEST_TRJ:
            emit getTrace(res,&trace);
            /* we can now update everything */
            updateBoatData();
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
    getData();
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

void boatAccount::updateBoatData()
{
    updateBoatName();
    reloadPolar();
    updatePosition();
    updateHint();
    //update();
}

void boatAccount::updateBoatName()
{
    QString txt;
    if(getAliasState())
        txt=(alias.isEmpty()?login:alias);
    else
        txt=login;

    label->setText(txt);

     adjustSize();
}

void boatAccount::createWidget()
{
    fgcolor = QColor(0,0,0);
    int gr = 255;
    bgcolor = QColor(gr,gr,gr,150);

    QGridLayout *layout = new QGridLayout();
    layout->setContentsMargins(10,0,2,0);
    layout->setSpacing(0);

    label = new QLabel(boat_name, this);
    label->setFont(QFont("Helvetica",10));

    QPalette p;
    p.setBrush(QPalette::Active, QPalette::WindowText, fgcolor);
    p.setBrush(QPalette::Inactive, QPalette::WindowText, fgcolor);
    label->setPalette(p);
    label->setAlignment(Qt::AlignHCenter);

    layout->addWidget(label, 0,0, Qt::AlignHCenter|Qt::AlignVCenter);
    this->setLayout(layout);
    setAutoFillBackground (false);
    hide();
}

void boatAccount::updatePosition(void)
{
    int boat_i,boat_j;
    Util::computePos(proj,lat,lon,&boat_i,&boat_j);
    boat_i-=3;
    boat_j-=(height()/2);
    move(boat_i, boat_j);
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

void boatAccount::projectionUpdated()
{
    if(activated)
        updatePosition();
}

void  boatAccount::paintEvent(QPaintEvent *)
{
    QPainter pnt(this);
    int dy = height()/2;

    //qWarning() << "qtBoat " << login << " paint";

    pnt.fillRect(9,0, width()-10,height()-1, QBrush(bgcolor));

    QPen pen(selected?selColor:myColor);
    pen.setWidth(4);
    pnt.setPen(pen);
    pnt.fillRect(0,dy-3,7,7, QBrush(selected?selColor:myColor));

    int g = 60;
    pen = QPen(QColor(g,g,g));
    pen.setWidth(1);
    pnt.setPen(pen);
    pnt.drawRect(9,0,width()-10,height()-1);
}

void  boatAccount::enterEvent (QEvent *)
{
    enterCursor = cursor();
    setCursor(Qt::PointingHandCursor);
}

void  boatAccount::leaveEvent (QEvent *)
{
    setCursor(enterCursor);
}

void  boatAccount::mousePressEvent(QMouseEvent * e)
{
    if(e->button() == Qt::LeftButton)
    {
        if(!((MainWindow*)mainWindow)->get_selPOI_instruction())
            selectBoat();
    }
}

void  boatAccount::mouseDoubleClickEvent(QMouseEvent *)
{

}

void  boatAccount::mouseReleaseEvent(QMouseEvent *)
{

}

void boatAccount::contextMenuEvent(QContextMenuEvent *)
{
    int nb=0;
    if(!((MainWindow*)mainWindow)->get_selPOI_instruction())
    {
        ac_select->setEnabled(true);
        nb++;
    }
    else
        ac_select->setEnabled(false);

    if(!selected)
    {
        if(forceEstime)
            ac_estime->setText(tr("Cacher estime"));
        else
            ac_estime->setText(tr("Afficher estime"));
        ac_estime->setEnabled(true);
        nb++;
    }
    else
        ac_estime->setEnabled(false);
    if(nb)
        popup->exec(QCursor::pos());
}

void boatAccount::createPopUpMenu(void)
{
    popup = new QMenu(parent);

    ac_select = new QAction("Selectionner",popup);
    popup->addAction(ac_select);

    ac_estime = new QAction("Afficher estime",popup);
    popup->addAction(ac_estime);

    /*ac_change = new QAction("Changer",popup);
    popup->addAction(ac_change);*/
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
     //doRequest(VLM_REQUEST_IDU);
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
        emit getPolar(polarName,&polarData);
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

void boatAccount::paramChanged()
{
    myColor = QColor(Util::getSetting("qtBoat_color",QColor(Qt::blue).name()).toString());
    selColor = QColor(Util::getSetting("qtBoat_sel_color",QColor(Qt::red).name()).toString());
    if(activated)
        update();
}

