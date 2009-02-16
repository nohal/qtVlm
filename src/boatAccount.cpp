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

#define VLM_NO_REQUEST  -1
#define VLM_REQUEST_IDU  0
#define VLM_REQUEST_BOAT 1

boatAccount::boatAccount(QString login, QString pass, bool activated,Projection * proj,QWidget * main, QWidget *parentWindow)
    : QWidget(parentWindow)
{
    this->parent=parentWindow;
    this->mainWindow=main;

    boat_name="NO NAME";
    boat_id=race_id=-1;
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

    this->proj = proj;
    connect(parentWindow, SIGNAL(projectionUpdated()), this,
            SLOT(projectionUpdated()) );

    createPopUpMenu();

    currentRequest=VLM_NO_REQUEST;

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
    connect(this,SIGNAL(WPChanged(float,float)),main,SLOT(slotWPChanged(float,float)));
            
    /* init http inetManager */
    inetManager = new QNetworkAccessManager(this);
    if(inetManager)
    {
        host = Util::getHost();
        connect(inetManager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(requestFinished (QNetworkReply*)));
        Util::paramProxy(inetManager,host);
    }

    setParam(login,pass,activated);
}

boatAccount::~boatAccount()
{
    if(inetManager)
        delete inetManager;
    disconnect();
}

void boatAccount::updateProxy(void)
{
    /* update connection */
    Util::paramProxy(inetManager,host);
#warning bad fix: we are forcing a new request
    currentRequest=VLM_NO_REQUEST;

    doRequest(VLM_REQUEST_IDU);
}

void boatAccount::selectBoat()
{
    emit   boatSelected(this);
    selected = true;
    update();
}

void boatAccount::unSelectBoat()
{
    selected = false;
    update();
}

void boatAccount::toggleEstime()
{
    forceEstime=!forceEstime;
    update();
}

void boatAccount::getData(void)
{
    doRequest(VLM_REQUEST_BOAT);
}

void boatAccount::doRequest(int requestCmd)
{
    if(!activated)
        return;

    if(inetManager)
    {
        if(currentRequest != VLM_NO_REQUEST)
        {
            qWarning() << "request already running " << login << " "
                    << (currentRequest==VLM_REQUEST_BOAT?"boat":"idu");
            return;
        }

        QString page;

        switch(requestCmd)
        {
            case VLM_REQUEST_BOAT:
                if(boat_id==-1)
                    return;
                QTextStream(&page) << host
                            << "/getinfo.php?"
                            << "pseudo=" << login
                            << "&password=" << pass
                            << "&idu="<< boat_id
                            ;
                break;
            case VLM_REQUEST_IDU:
                QTextStream(&page) << host
                            << "/getinfo2.php?"
                            << "pseudo=" << login
                            << "&password=" << pass
                            ;
                break;
        }
        currentRequest = requestCmd;
        qWarning() << "boat Acc Doing request: " << page;

        QNetworkRequest request;
        request.setUrl(QUrl(page));
        Util::addAgent(request);   
        inetManager->get(request);
    }
    else
    {
         qWarning("Creating dummy data");
         boat_id = 0;
         boat_name = "test";
         race_id = 0;
         lat = 0;
         lon = 0;
         race_name = "Test race";
         updateBoatData();
    }
}

void boatAccount::requestFinished ( QNetworkReply* inetReply)
{
    hasPilototo=false;

    if (inetReply->error() != QNetworkReply::NoError) {
        qWarning() << "Error doing inetGet:" << inetReply->error();
        currentRequest=VLM_NO_REQUEST;
    }
    else
    {
        //-------------------------------------------
        // Retour de l'étape 1 : préparation du fichier
        //-------------------------------------------

        pilototo.clear();
        for(int i=0;i<5;i++)
            pilototo.append("none");
        QString strbuf = inetReply->readAll();
        QStringList lsbuf;
        float latitude=0,longitude=0;
        bool newRace=false;

        switch(currentRequest)
        {
            case VLM_REQUEST_IDU:
                lsbuf = strbuf.split(";");
                for (int i=0; i < lsbuf.size(); i++)
                {
                    QStringList lsval = lsbuf.at(i).split("=");
                    if (lsval.size() == 2) {
                        if (lsval.at(0) == "IDU")
                        {
                            boat_id = lsval.at(1).toInt();
                            qWarning() << "Get boat id " << boat_id;
                            currentRequest=VLM_NO_REQUEST;
                            doRequest(VLM_REQUEST_BOAT);
                            break;
                        }
                    }
                }
                break;
            case VLM_REQUEST_BOAT:
                lsbuf = strbuf.split("\n");

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
                                qWarning() << "RAC=0";
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
                    }
                }

                lat = latitude/1000;
                lon = longitude/1000;
                
                current_heading = heading;
                qWarning() << "Data for " << boat_id << " received";
                /* compute heading point */
                updateBoatData();
                currentRequest=VLM_NO_REQUEST;
                emit boatUpdated(this,newRace);
                break;
        }
    }
    //delete inetReply;
}

void boatAccount::updateBoatData()
{
    updateBoatName();
    reloadPolar();
    updatePosition();
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
    update();
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
    if(!selected)
    {
        if(forceEstime)
            ac_estime->setText(tr("Cacher estime"));
        else
            ac_estime->setText(tr("Afficher estime"));
        ac_estime->setEnabled(true);
    }
    else
        ac_estime->setEnabled(false);
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

     doRequest(VLM_REQUEST_IDU);
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
                delete polarData;
                polarData=NULL;
            }
            qWarning("No User polar to load");
            return;
        }        
        if(polarData != NULL && polarName == polarData->getName())
            return;        
        if(polarData!=NULL)
            delete polarData;        
        qWarning() << "Loading forced polar: " << polarName;        
        polarData=new Polar(polarName,mainWindow);
    }
    else
    {
        if(polarVlm.isEmpty())
        {
            if(polarData!=NULL)
            {
                delete polarData;
                polarData=NULL;
            }
            qWarning("No VLM polar to load");
            return;
        }
        if(polarData != NULL && polarVlm == polarData->getName())
            return;
        
        if(polarData!=NULL)
        {
            qWarning() << "Old polar:" << polarData->getName();            
            delete polarData;
        }
        qWarning() << "Loading polar: " << polarVlm;
        polarData=new Polar(polarVlm,mainWindow);
    }
}

void boatAccount::setPolar(bool state,QString polar)
{
    this->polarName=polar;
    forcePolar=state;

    reloadPolar();
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

