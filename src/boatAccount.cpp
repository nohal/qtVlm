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


#include "boatAccount.h"
#include "MainWindow.h"

#define VLM_NO_REQUEST  -1
#define VLM_REQUEST_IDU  0
#define VLM_REQUEST_BOAT 1

boatAccount::boatAccount(QString login, QString pass, bool activated,Projection * proj,QWidget * main, QWidget *parentWindow)
    : QWidget(parentWindow)
{
    this->parent=parentWindow;

    boat_name="NO NAME";
    boat_id=race_id=-1;
    isSync = false;
    selected = false;

    this->proj = proj;
    connect(proj, SIGNAL(projectionUpdated(Projection *)), this,
            SLOT(projectionUpdated(Projection *)) );
    connect(this,SIGNAL(showMessage(QString)),main, SLOT(slotShowMessage(QString)));

    createPopUpMenu();

    currentRequest=VLM_NO_REQUEST;

    createWidget();

    if(activated)
        show();

    connect(ac_select,SIGNAL(triggered()),this,SLOT(selectBoat()));

    connect(this,SIGNAL(boatSelected(boatAccount*)),main,SLOT(slotSelectBoat(boatAccount*)));

    connect(this,SIGNAL(boatUpdated(boatAccount*)),main,SLOT(slotBoatUpdated(boatAccount*)));

    /* init http inetManager */

#if 1
    inetManager = new QNetworkAccessManager(this);
    if(inetManager)
    {
        host = "http://www.virtual-loup-de-mer.org";
        connect(inetManager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(requestFinished (QNetworkReply*)));
        connect(inetManager, SIGNAL(proxyAuthenticationRequired(QNetworkProxy , QAuthenticator * )),
            this,SLOT(requestNeedProxy(QNetworkProxy  ,QAuthenticator * )));
        connect(inetManager, SIGNAL(authenticationRequired(QNetworkReply* ,QAuthenticator* )),
            this,SLOT(requestNeedAuth(QNetworkReply*,QAuthenticator*)));

        updateProxy();
    }
#else
     inetManager = NULL;
#endif

    setParam(login,pass,activated);
}

boatAccount::~boatAccount()
{
    if(inetManager)
        delete inetManager;
}

void boatAccount::updateProxy(void)
{
    /* update connection */
    Util::paramProxy(inetManager,host);
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
            emit showMessage("request already running " + login + " "
                    + (currentRequest==VLM_REQUEST_BOAT?"boat":"idu"));
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
        showMessage("Doing inet request: " + page);

        inetManager->get(QNetworkRequest(QUrl(page)));
    }
    else
    {
         emit showMessage("Creating dummy data");
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
    if (inetReply->error() != QNetworkReply::NoError) {
        emit showMessage("Error doing inetGet:" + QString().setNum(inetReply->error()));
        currentRequest=VLM_NO_REQUEST;
    }
    else
    {
        //-------------------------------------------
        // Retour de l'étape 1 : préparation du fichier
        //-------------------------------------------


        QString strbuf = inetReply->readAll();
        QStringList lsbuf;
        float latitude=0,longitude=0;

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
                            emit showMessage("Get boat id " + QString().setNum(boat_id));
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
                            race_id = lsval.at(1).toInt();
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
                    }
                }

                lat = latitude/1000;
                lon = longitude/1000;
                updateBoatData();
                currentRequest=VLM_NO_REQUEST;
                emit boatUpdated(this);
                break;
        }

    }
}

void boatAccount::requestNeedProxy(QNetworkProxy ,QAuthenticator * )
{
    showMessage("Need proxy");
}

void boatAccount::requestNeedAuth(QNetworkReply* ,QAuthenticator* )
{
    showMessage("Need auth");
}

void boatAccount::updateBoatData()
{
    setLabelText(login);
    updatePosition();
    update();
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
    if (proj->isPointVisible(lon, lat)) {      // tour du monde ?
        proj->map2screen(lon, lat, &pi, &pj);
    }
    else if (proj->isPointVisible(lon-360, lat)) {
        proj->map2screen(lon-360, lat, &pi, &pj);
    }
    else {
        proj->map2screen(lon+360, lat, &pi, &pj);
    }

    int dy = height()/2;
    move(pi-3, pj-dy);
}

void boatAccount::projectionUpdated(Projection * proj)
{
    //emit showMessage("Projection update");
    this->proj=proj;
    updatePosition();
}

void boatAccount::setLabelText(QString name)
{
     label->setText(name);
     adjustSize();
}

void  boatAccount::paintEvent(QPaintEvent *)
{
    QPainter pnt(this);
    int dy = height()/2;

    pnt.fillRect(9,0, width()-10,height()-1, QBrush(bgcolor));

    QPen pen(selected?Qt::red:Qt::blue);
    pen.setWidth(4);
    pnt.setPen(pen);
    pnt.fillRect(0,dy-3,7,7, QBrush(selected?Qt::red:Qt::blue));

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
    if (e->button() == Qt::RightButton)
    {
        // Right clic : Edit this Point Of Interest
        popup->exec(QCursor::pos());
    }
}

void  boatAccount::mouseDoubleClickEvent(QMouseEvent *)
{
}

void  boatAccount::mouseReleaseEvent(QMouseEvent *)
{

}

void boatAccount::createPopUpMenu(void)
{
    popup = new QMenu(parent);

    ac_select = new QAction("Selectionner",popup);
    popup->addAction(ac_select);

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
