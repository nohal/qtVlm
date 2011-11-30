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

#include <QDebug>
#include <QLabel>
#include <QWidget>
#include <QMessageBox>

#include "opponentBoat.h"

#include "boatVLM.h"
#include "MainWindow.h"
#include "settings.h"
#include "mycentralwidget.h"
#include "Projection.h"
#include "vlmLine.h"
#include "Util.h"
#include "parser.h"
#include "Orthodromie.h"
#include "boat.h"

/****************************************
* Opponent methods
****************************************/

opponent::opponent(QColor color, QString idu,QString race, float lat, float lon, QString pseudo,
                            QString name,Projection * proj,MainWindow *main, myCentralWidget *parentWindow):QGraphicsWidget()
{
    init(color,false,idu,race,lat,lon,pseudo,name,proj,main,parentWindow);
}

opponent::opponent(QColor color, QString idu,QString race,Projection * proj,MainWindow *main, myCentralWidget *parentWindow):QGraphicsWidget()
{
    init(color,true,idu,race,0,0,"","",proj,main,parentWindow);
}

void opponent::init(QColor color,bool isQtBoat,QString idu,QString race, float lat, float lon, QString pseudo,
                            QString name,Projection * proj,MainWindow *main, myCentralWidget *parentWindow)
{
    this->idu=idu;
    this->idrace=race;
    this->lat=lat;
    this->lon=lon;
    this->pseudo=pseudo;
    this->name=name;
    this->proj=proj;
    this->main=main;
    this->parentWindow=parentWindow;
    this->desc="";
    this->longName="";
    this->isReal=false;
    this->lastUpdate=QDateTime().currentDateTimeUtc().toTime_t();

    this->opp_trace=1;
    this->labelHidden=parentWindow->get_shLab_st();
    connect(parentWindow, SIGNAL(showALL(bool)),this,SLOT(slot_shShow()));
    connect(parentWindow, SIGNAL(hideALL(bool)),this,SLOT(slot_shHidden()));
    connect(parentWindow, SIGNAL(shOpp(bool)),this,SLOT(slot_shOpp()));
    connect(parentWindow, SIGNAL(shLab(bool)),this,SLOT(slot_shLab(bool)));
    width=height=0;

    parentWindow->getScene()->addItem(this);

    setZValue(Z_VALUE_OPP);
    setFont(QFont(QApplication::font()));
    setData(0,OPP_WTYPE);

    fgcolor = QColor(0,0,0);
    int gr = 255;
    bgcolor = QColor(gr,gr,gr,150);

    this->isQtBoat = isQtBoat;
    trace.clear();
    trace_drawing = new vlmLine(proj,parentWindow->getScene(),Z_VALUE_OPP);
    QPen penTrace=QPen(color);
    penTrace.setWidthF(1);
    trace_drawing->setLinePen(penTrace);
    trace_drawing->setLineMode();
    connect(parentWindow,SIGNAL(startReplay(bool)),trace_drawing,SLOT(slot_startReplay(bool)));
    connect(parentWindow,SIGNAL(replay(int)),trace_drawing,SLOT(slot_replay(int)));
    this->flag=QImage();
    this->drawFlag=false;
    this->pavillon=QString();

    myColor = color;

    paramChanged();

    if(!isQtBoat&& !parentWindow->get_shOpp_st())
        show();
    else
        hide();

    connect(proj, SIGNAL(projectionUpdated()), this, SLOT(updateProjection()));
    connect(main,SIGNAL(paramVLMChanged()),this,SLOT(paramChanged()));

    this->statusVLM="";
    this->loch1h="";
    this->loch3h="";
    this->loch24h="";
    this->rank=0;
    updatePosition();
}

opponent::~opponent(void)
{
    if(trace_drawing)
    {
        if(!parentWindow->getAboutToQuit())
            delete trace_drawing;
    }
}

/**************************/
/* boundingRect, Paint    */
/**************************/

QRectF opponent::boundingRect() const
{
    if(!drawFlag)
        return QRectF(0,0,width,height);
    else
        return QRectF(-12,height/2-10,width+30,25);
}

void opponent::paint(QPainter * pnt, const QStyleOptionGraphicsItem * , QWidget * )
{
    if(isQtBoat || parentWindow->get_shOpp_st())
    {
        hide();
        return;
    }
    if(name.isEmpty())
        return;

    if(Settings::getSetting("showFlag",0).toInt()==1)
    {
        if(flag.isNull())
        {
            if(flag.load("img/flags/"+this->pavillon+".png"))
            {
                flag=flag.scaled(30,20,Qt::KeepAspectRatio);
                drawFlag=true;
                prepareGeometryChange();
            }
            else
                drawFlag=false;
        }
        else
        {
            if(!drawFlag)
                prepareGeometryChange();
            drawFlag=true;
        }
    }
    else
    {
        if(drawFlag)
            prepareGeometryChange();
        drawFlag=false;
    }
    int dy = height/2;

    QFontMetrics fm(font());
    if(!labelHidden)
    {
        if(this->isReal)
            bgcolor=QColor(128,126,219,150);
        else if(this->statusVLM.toLower()=="on_coast" || this->statusVLM.toLower()=="locked")
            bgcolor=QColor(239,48,36,150);
        else
            bgcolor = QColor(255,255,255,150);
        if(!drawFlag)
        {
            pnt->fillRect(9,0, width-10,height-1, QBrush(bgcolor));
            pnt->setFont(font());
            pnt->drawText(10,fm.height()-2,my_str);
        }
        else
        {
            pnt->fillRect(21,0, width-10,height-1, QBrush(bgcolor));
            pnt->setFont(font());
            pnt->drawText(22,fm.height()-2,my_str);
        }
    }
    QPen pen(myColor);
    if(!drawFlag)
    {
        pen.setWidth(4);
        pnt->setPen(pen);
        pnt->fillRect(0,dy-3,7,7, QBrush(myColor));
    }
    else
    {
        pnt->drawImage(-11,dy-9,flag);
    }
    int g = 60;
    pen = QPen(QColor(g,g,g));
    pen.setWidth(1);
    pnt->setPen(pen);
    if(!labelHidden)
    {
        if(!drawFlag)
            pnt->drawRect(9,0,width-10,height-1);
        else
            pnt->drawRect(21,0,width-10,height-1);
    }
}
void opponent::updateProjection()
{
    updatePosition();
    update();
}

void opponent::updatePosition()
{
    int boat_i,boat_j;

    Util::computePos(proj,lat,lon,&boat_i,&boat_j);

    boat_i-=3;
    boat_j-=(height/2);

    setPos(boat_i, boat_j);
    if(!parentWindow->get_shOpp_st())
        drawTrace();
}
void opponent::drawTrace()
{
    trace_drawing->setNbVacPerHour(3600/main->get_selectedBoatVacLen());
    if(!trace.isEmpty() && (trace.last().lat!=lat || trace.last().lon!=lon))
        trace.append(vlmPoint(lon,lat));
    if(opp_trace==1)
    {
        trace_drawing->setPoly(trace);
        trace_drawing->slot_showMe();
    }
    else
    {
        trace_drawing->hide();
    }
}

void opponent::updateName()
{
    QString str2;
    if(!isReal)
    {
        switch(label_type)
        {
            case SHOW_PSEUDO:
                my_str = pseudo;
                str2 = idu + " - " + name;
                break;
            case SHOW_NAME:
                my_str = name;
                str2 = idu + " - " + pseudo;
                break;
            case SHOW_IDU:
                my_str = idu;
                str2 = pseudo + " - " + name;
                break;
        }
    }
    else
        my_str=name;
    QString tt;
    if(!isReal)
    {
        str2=str2+"<br>"+tr("Classement: ")+tt.sprintf("%d",this->rank);
        str2=str2+"<br>"+tr("Loch 1h: ")+this->loch1h;
        str2=str2+"<br>"+tr("Loch 3h: ")+this->loch3h;
        str2=str2+"<br>"+tr("Loch 24h: ")+this->loch24h;
        str2=str2+"<br>"+tr("Status VLM: ")+this->statusVLM;
        str2.replace(" ","&nbsp;");
    }
    else
    {
        str2=str2+this->longName;
        str2=str2+"<br>"+tr("Date de la position: ")+
                QDateTime().fromTime_t(this->lastUpdate).toUTC().toString("dd MMM-hh:mm");
        str2+="<br>"+tr("Latitude:  ")+Util::pos2String(TYPE_LAT,this->lat);
        str2+="<br>"+tr("Longitude: ")+Util::pos2String(TYPE_LON,this->lon);
        str2.replace(" ","&nbsp;");
        if(!desc.isEmpty() && !desc.contains("arazzia"))
            str2=str2+"<br>"+desc;
    }
    setToolTip(str2);
    /* compute size */
    /* computing widget size */
    QFontMetrics fm(font());
    prepareGeometryChange();
    width = fm.width(my_str) + 10 + 2;
    height = qMax(fm.height()+2,10);

}
void opponent::setRealData(QString shortName, QString longName, QString desc, QString pavillon)
{
    this->name=shortName;
    this->longName=longName;
    this->desc=desc;
    this->pavillon=pavillon;
    updateName();
    update();
}

void opponent::setNewData(float lat, float lon,QString name)
{
    bool needUpdate = false;

    if(lat != this->lat || lon != this->lon)
    {
       this->lat=lat;
       this->lon=lon;
       updatePosition();
       needUpdate = true;
    }
    if(name.isEmpty()) name=this->pseudo;
    if(name != this->name)
    {
        this->name=name;
        updateName();
        needUpdate = true;
    }

    /* new data => we are not a qtVlm boat */
    if(isQtBoat)
    {
        setIsQtBoat(false);
        /* why doing this ? */
    }
    else
    {
        if(needUpdate)
            update();
    }
}

void opponent::setIsQtBoat(bool status)
{
    if(status == isQtBoat)
        return;
    isQtBoat=status;
    if(isQtBoat || parentWindow->get_shOpp_st())
        hide();
    else
        show();
}

void opponent::paramChanged()
{
    //myColor = QColor(Settings::getSetting("opp_color",QColor(Qt::green).name()).toString());
    label_type = Settings::getSetting("opp_labelType",0).toInt();
    opp_trace = Settings::getSetting("opp_trace","1").toInt();

    updatePosition();
    updateName();
    if(!isQtBoat)
        update();
}

void opponent::slot_shShow()
{
    show();
    if(trace_drawing)
    {
        drawTrace();
        trace_drawing->show();
    }
}

void opponent::slot_shHidden()
{
    hide();
    if(trace_drawing)
    {
        trace_drawing->hide();
    }
}

/****************************************
* Opponent list methods
****************************************/
#define OPP_TYPE_POSITION  0
#define OPP_TYPE_NAME      1

#define OPP_NO_REQUEST     0
#define OPP_BOAT_DATA      1
#define OPP_BOAT_TRJ       2
#define OPP_LIST           3
#define OPP_BOAT_REAL      4
#define OPP_INFO_REAL      5

#define OPP_MODE_REFRESH   0
#define OPP_MODE_NEWLIST   1

opponentList::opponentList(Projection * proj,MainWindow * main,myCentralWidget * parent, inetConnexion * inet) :
        QWidget(parent),
        inetClient(inet)
{
    inetClient::setName("OpponentList");

    needAuth=true;

    this->parent=parent;
    this->main=main;
    this->proj=proj;
    currentRace="";
    colorTable[0] = QColor(170,0,0);
    colorTable[1] = QColor(100,100,100);
    colorTable[2] = QColor(170,0,128);
    colorTable[3] = QColor(0,85,127);
    colorTable[4] = QColor(255,85,0);
    colorTable[5] = QColor(255,0,255);
    colorTable[6] = QColor(85,85,0);
    colorTable[7] = QColor(225,150,112);
    colorTable[8] = QColor(0,85,0);
    colorTable[9] = QColor(85,0,0);
    colorTable[10] = QColor(255,0,255);
    colorTable[11] = QColor(255,255,0);
    colorTable[12] = QColor(255,85,127);
    colorTable[13] = QColor(170,170,255);
    colorTable[14] = QColor(170,0,255);
    showWhat=SHOW_MY_LIST;
    showReal=false;
}

QString opponentList::getRaceId()
{
    if(opponent_list.size()<=0)
        return "";

    return opponent_list[0]->getRace();
}

void opponentList::setBoatList(QString list_txt,QString race,int showWhat, bool force, bool showReal)
{
    this->showWhat=showWhat;
    this->showReal=showReal;
    currentRace = race;
    //qWarning() << "SetBoatList - race " << race << " - " << list_txt;
    if(!hasInet() || hasRequest())
    {
        if(!hasInet())
        {
            qWarning() << "getOpponents bad state in inet - setBoatList: " << hasInet() << " " << hasRequest();
            return;
        }
        qWarning()<<"Cancelling previous request";
        this->inetAbort();
    }
    if(showWhat==SHOW_NONE && !showReal)
    {
        clear();
        return;
    }

    /* is a list defined ? */
    if(opponent_list.size()>0)
    {
        if(!force && opponent_list[0]->getRace() == race && showWhat==SHOW_MY_LIST) /* it is the same race */
        { /* compare if same opp list */
            /* for now it is the same => refresh*/
            //qWarning() << "Refresh 2";
            refreshData();
            return;
        }
        /* clear current list */
        clear();
    }

    currentOpponent = 0;
    if(!list_txt.isEmpty())
        currentList = list_txt.split(";");
    else
        currentList.clear();
    currentMode = OPP_MODE_NEWLIST;
    if(currentList.size() > 0 || showWhat!=SHOW_MY_LIST || showReal)
        getGenData();


}

void opponentList::clear(void)
{
    while (!opponent_list.isEmpty())
        delete opponent_list.takeFirst();
}

void opponentList::refreshData(void)
{    
    //qWarning() << "refreshData";
    if(!hasInet() || hasRequest())
    {
        if(!hasInet())
        {
            qWarning() << "getOpponents bad state in inet - refreshData: " << hasInet() << " " << hasRequest();
            return;
        }
        qWarning()<<"Cancelling previous request";
        this->inetAbort();
    }

    if(opponent_list.size()<=0 && !this->showReal)
        return;

    //currentRace = opponent_list[0]->getRace();
    currentOpponent = 0;
    currentMode=OPP_MODE_REFRESH;
    getGenData();
}

QString opponentList::getAuthLogin(bool * ok)
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

QString opponentList::getAuthPass(bool * ok)
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

void opponentList::getGenData()
{
    QString page;
    QTextStream(&page)
                        << "/ws/raceinfo/ranking.php?idr="
                        << currentRace;
    if(showWhat==SHOW_TEN_FIRST)
        QTextStream(&page)<<"&limit=10";
    //qWarning() << "OPP, clearReq 2";
    clearCurrentRequest();
    inetGet(OPP_BOAT_DATA,page);
}

void opponentList::getNxtOppData()
{
    int listSize = opponent_list.size();
    QString idu;
    if(currentOpponent>=listSize)
    {
        parent->update();
        return;
    }

    idu = opponent_list[currentOpponent]->getIduser();

    if(main->isBoat(idu) && !opponent_list[currentOpponent]->getIsReal())
    {
        opponent_list[currentOpponent]->setIsQtBoat(true);
        ++currentOpponent;
        getNxtOppData();
        return;
    }
    ++currentOpponent;

    //qWarning() << "OPP, clearReq 1";
    clearCurrentRequest();

    time_t endtime=QDateTime::currentDateTime().toUTC().toTime_t();
    time_t starttime=endtime-(Settings::getSetting("trace_length",12).toInt()*60*60);
    QString page;
    QList<vlmPoint> * previousTrace=opponent_list[currentOpponent-1]->getTrace();
    if(!previousTrace->isEmpty())
    {
        for(int i=previousTrace->count()-1;i>=0;--i)
        {
            if(previousTrace->at(i).timeStamp<starttime)
                previousTrace->removeAt(i);
        }
        if(!previousTrace->isEmpty())
            starttime=previousTrace->last().timeStamp+10;
    }
    if(!opponent_list[currentOpponent-1]->getIsReal())
    {
        QTextStream(&page)
            << "/ws/boatinfo/tracks.php?"
            << "idu="
            << idu
            << "&starttime="
            << starttime;

    }
    else
    {
        QTextStream(&page)
            << "/ws/realinfo/tracks.php?"
            << "idreals="
            << idu
            << "&idr="
            << currentRace
            << "&starttime="
            << starttime;
    }
    inetGet(OPP_BOAT_TRJ,page);
}

void opponentList::authFailed(void)
{
    QMessageBox::warning(0,QObject::tr("Parametre bateau"),
                  "Erreur de parametrage du joueur.\n Verifier le login et mot de passe puis reactivez le bateau");
    inetClient::authFailed();
}

void opponentList::inetError()
{

}

void opponentList::requestFinished (QByteArray res_byte)
{
    switch(getCurrentRequest())
    {        
        case OPP_BOAT_REAL:
        {
            QJson::Parser parser;
            bool ok;

            QVariantMap result = parser.parse (res_byte, &ok).toMap();
            if (!ok) {
                qWarning() << "Error parsing json data " << res_byte;
                qWarning() << "Error: " << parser.errorString() << " (line: " << parser.errorLine() << ")";
            }
            QVariantList vv=result["reals"].toList();
            QStringList idReals;
            foreach(QVariant v,vv)
            {
                QVariantMap data = v.toMap();
                bool foundIt=false;
                int foundNb=-1;
                for(int o=opponent_list.count()-1;o>=0;--o)
                {
                    if(!opponent_list.at(o)->getIsReal()) continue;
                    if(opponent_list.at(o)->getIduser()==data["idreals"].toString())
                    {
                        foundNb=o;
                        foundIt=true;
                        break;
                    }
                }
                if(!foundIt)
                {
                    opponent_list.append(new opponent(Qt::black,data["idreals"].toString(), currentRace,
                                         0,0,QString(), QString(), proj, main, parent));
                    opponent_list.last()->setIsReal(true);
                    opponent_list.last()->setLastUpdate(data["tracks_updated"].toInt());
                }
                else
                {
                    opponent_list[foundNb]->setLastUpdate(data["tracks_updated"].toInt());
                }
                idReals.append(data["idreals"].toString());
            }
            for (int i=opponent_list.count()-1;i>=0;--i)
            {
                if(!opponent_list.at(i)->getIsReal()) continue;
                if(!idReals.contains(opponent_list.at(i)->getIduser()))
                    delete opponent_list.takeAt(i);
            }
#if 0
            for (int i=opponent_list.count()-1;i>=0;--i)
            {
                qWarning()<<"opponent->"<<opponent_list.at(i)->getIduser();
            }
#endif
            if(opponent_list.count()>0)
            {
                currentOpponent = 0;
                currentMode=OPP_MODE_REFRESH;
                getNxtOppData();
            }
            break;
        }
        case OPP_BOAT_DATA:
            {
                QJson::Parser parser;
                bool ok;

                QVariantMap result = parser.parse (res_byte, &ok).toMap();
                if (!ok) {
                    qWarning() << "Error parsing json data " << res_byte;
                    qWarning() << "Error: " << parser.errorString() << " (line: " << parser.errorLine() << ")";
                }

                if(checkWSResult(res_byte,"OppList_getOppData2",main))
                {
                    QVariantMap ranking = result["ranking"].toMap();
                    if(currentMode==OPP_MODE_REFRESH && showWhat==SHOW_MY_LIST)
                    {                        
                        for (int o=opponent_list.count()-1;o>=0;o--)
                        {
                            opponent * ptr=opponent_list[o];
                            if(opponent_list[o]->getIsReal())
                            {
//                                delete opponent_list.takeAt(o);
                                continue;
                            }
                            QVariantMap data = ranking[ptr->getIduser()].toMap();
                            if(!data.isEmpty())
                            {
                                //qWarning() << "Found " << ptr->getIduser() ;
                                ptr->setNewData(data["latitude"].toFloat(),
                                                 data["longitude"].toFloat(),
                                                 data["boatname"].toString());
                                QString h1,h3,h24,pavillon;
                                h1=h1.sprintf("%.2f",data["last1h"].toFloat());
                                h3=h3.sprintf("%.2f",data["last3h"].toFloat());
                                h24=h24.sprintf("%.2f",data["last24h"].toFloat());
                                pavillon=data["country"].toString();
                                ptr->setOtherData(data["rank"].toInt(),h1,h3,h24,data["status"].toString(),pavillon);
                            }
                            else
                            {
                                qWarning()<<"removing arrived or abandonned opponent"<<ptr->getIduser();
                                parent->removeOpponent(ptr->getIduser(),ptr->getRace());
                                delete ptr;
                                opponent_list.removeAt(o);
                            }
                        }
                    }
                    else
                    {
                        int pos=0;
                        switch (this->showWhat)
                        {
                            case SHOW_MY_LIST:
                            {
                                for (int o=currentList.count()-1;o>=0;o--)
                                {
                                    QString str=currentList.at(o);
                                    QVariantMap data = ranking[str].toMap();
                                    if(!data.isEmpty())
                                    {
                                        if(isBoatVLM(data["idusers"].toString())) continue;
                                        //qWarning() << "(setList) Found " << str << " " << data["boatpseudo"].toString();
                                        opponent_list.append(new opponent(colorTable[pos],str,currentRace,
                                                                    data["latitude"].toFloat(),data["longitude"].toFloat(),
                                                                    data["boatpseudo"].toString(), data["boatname"].toString(),proj,main,parent));
                                        QString h1,h3,h24,pavillon;
                                        h1=h1.sprintf("%.2f",data["last1h"].toFloat());
                                        h3=h3.sprintf("%.2f",data["last3h"].toFloat());
                                        h24=h24.sprintf("%.2f",data["last24h"].toFloat());
                                        pavillon=data["country"].toString();
                                        opponent_list.last()->setOtherData(data["rank"].toInt(),h1,h3,h24,data["status"].toString(),pavillon);

                                        pos++;
                                    }
                                    else
                                    {
                                        qWarning()<<"removing arrived or abandonned opponent"<<str;
                                        parent->removeOpponent(str,currentRace);
                                        currentList.removeAt(o);
                                    }
                                }
                                break;
                            }
                            case SHOW_TEN_FIRST:
                            {
                                QMapIterator<QString,QVariant> it(ranking);
                                while (it.hasNext())
                                {
                                    QVariantMap data=it.next().value().toMap();
                                    if(isBoatVLM(data["idusers"].toString())) continue;
                                    opponent_list.append(new opponent(colorTable[pos],data["idusers"].toString(),currentRace,
                                                                data["latitude"].toFloat(),data["longitude"].toFloat(),
                                                                data["boatpseudo"].toString(), data["boatname"].toString(),proj,main,parent));
                                    QString h1,h3,h24,pavillon;
                                    h1=h1.sprintf("%.2f",data["last1h"].toFloat());
                                    h3=h3.sprintf("%.2f",data["last3h"].toFloat());
                                    h24=h24.sprintf("%.2f",data["last24h"].toFloat());
                                    pavillon=data["country"].toString();
                                    opponent_list.last()->setOtherData(data["rank"].toInt(),h1,h3,h24,data["status"].toString(),pavillon);
                                    pos++;
                                }
                                break;
                            }
                            case SHOW_TEN_CLOSEST_DISTANCE:
                            {
                                Orthodromie orth(parent->getSelectedBoat()->getLon(),
                                                 parent->getSelectedBoat()->getLat(),
                                                 0,0);
                                QMultiMap<float,QVariantMap> sorted;
                                QMapIterator<QString,QVariant> it(ranking);
                                while (it.hasNext())
                                {
                                    QVariantMap data=it.next().value().toMap();
                                    if(isBoatVLM(data["idusers"].toString())) continue;
                                    orth.setEndPoint(data["longitude"].toFloat(),
                                                     data["latitude"].toFloat());
                                    if(orth.getDistance()==0) continue;
                                    sorted.insert(orth.getDistance(),data);
                                }
                                QMapIterator<float,QVariantMap> s(sorted);
                                while (s.hasNext())
                                {
                                    QVariantMap data=s.next().value();
                                    opponent_list.append(new opponent(colorTable[pos],data["idusers"].toString(),currentRace,
                                                                data["latitude"].toFloat(),data["longitude"].toFloat(),
                                                                data["boatpseudo"].toString(), data["boatname"].toString(),proj,main,parent));
                                    QString h1,h3,h24,pavillon;
                                    h1=h1.sprintf("%.2f",data["last1h"].toFloat());
                                    h3=h3.sprintf("%.2f",data["last3h"].toFloat());
                                    h24=h24.sprintf("%.2f",data["last24h"].toFloat());
                                    pavillon=data["country"].toString();
                                    opponent_list.last()->setOtherData(data["rank"].toInt(),h1,h3,h24,data["status"].toString(),pavillon);
                                    pos++;
                                    if (pos>=10) break;
                                }
                                break;
                            }
                            case SHOW_TEN_CLOSEST_RANKING:
                            {
                                QMultiMap<int,QVariantMap> sorted;
                                QMapIterator<QString,QVariant> it(ranking);
                                QString score=parent->getSelectedBoat()->getScore();
                                QStringList ranks=score.split("/");
                                int playerRank=ranks.at(0).toInt();
                                while (it.hasNext())
                                {
                                    QVariantMap data=it.next().value().toMap();
                                    if(isBoatVLM(data["idusers"].toString())) continue;
                                    int diffRank=qAbs(data["rank"].toInt()-playerRank);
                                    if(diffRank==0) continue;
                                    sorted.insert(diffRank,data);
                                }
                                QMapIterator<int,QVariantMap> s(sorted);
                                while (s.hasNext())
                                {
                                    QVariantMap data=s.next().value();
                                    opponent_list.append(new opponent(colorTable[pos],data["idusers"].toString(),currentRace,
                                                                data["latitude"].toFloat(),data["longitude"].toFloat(),
                                                                data["boatpseudo"].toString(), data["boatname"].toString(),proj,main,parent));
                                    QString h1,h3,h24,pavillon;
                                    h1=h1.sprintf("%.2f",data["last1h"].toFloat());
                                    h3=h3.sprintf("%.2f",data["last3h"].toFloat());
                                    h24=h24.sprintf("%.2f",data["last24h"].toFloat());
                                    pavillon=data["country"].toString();
                                    opponent_list.last()->setOtherData(data["rank"].toInt(),h1,h3,h24,data["status"].toString(),pavillon);
                                    pos++;
                                    if (pos>=10) break;
                                }
                                break;
                            }
                        }
                    }
                    if(this->showReal)
                    {
                        QString page;
                        QTextStream(&page)
                                            << "/ws/raceinfo/reals.php?idr="
                                            << currentRace;
                        clearCurrentRequest();
                        inetGet(OPP_BOAT_REAL,page);
                    }
                    else if(opponent_list.count()>0)
                    {
                        //currentRace = opponent_list[0]->getRace();
                        currentOpponent = 0;
                        currentMode=OPP_MODE_REFRESH;
                        getNxtOppData();
                    }
                }
                else
                {
                    qWarning()<<"erreur is in"<<getCurrentRequest();
                }
            }
            break;


        case OPP_BOAT_TRJ:
            opp=NULL;
            if(currentMode==OPP_MODE_REFRESH)
            {
                //qWarning() << "Refresh";
                if(currentOpponent > opponent_list.size())
                {
                    //qWarning() << currentOpponent << " - " << opponent_list.size();
                    break;
                }
                opp=opponent_list[currentOpponent-1];
                getTrace(res_byte,opp->getTrace());
                if(opp->getIsReal())
                {
                    if(!opp->getTrace()->isEmpty())
                    {
                        //qWarning()<<"last point in trace:"<<opp->getTrace()->last().lon<<opp->getTrace()->last().lat;
                        opp->setNewData(opp->getTrace()->last().lat,
                                        opp->getTrace()->last().lon,
                                        QString());
                    }
                    else
                        qWarning()<<"trace is empty??";
                }
                if(!parent->get_shOpp_st())
                    opp->drawTrace();
            }
            else
            {
                //qWarning() << "Not Refresh";
                if(!opponent_list.isEmpty())
                {
                    opp=opponent_list.last();
                    getTrace(res_byte,opp->getTrace());
                    if(opp->getIsReal())
                    {
                        if(!opp->getTrace()->isEmpty())
                            opp->setNewData(opp->getTrace()->last().lat,
                                            opp->getTrace()->last().lon,
                                            QString());
                    }
                    if(!parent->get_shOpp_st())
                        opp->drawTrace();
                }
            }
            if(!opponent_list.isEmpty() && opp && opp->getIsReal())
            {
                QString page;
                QTextStream(&page)
                    << "/ws/realinfo/profile.php?"
                    << "idreals="
                    << opp->getIduser()
                    <<"&idr="
                    <<opp->getRace();
                inetGet(OPP_INFO_REAL,page);
            }
            else
                getNxtOppData();
            break;
        case OPP_INFO_REAL:
        {
            //qWarning()<<"inside OPP_INFO_REAL";
            QJson::Parser parser;
            bool ok;

            QVariantMap result=parser.parse (res_byte, &ok).toMap();
            if (!ok) {
                qWarning() << "Error parsing json data " << res_byte;
                qWarning() << "Error: " << parser.errorString() << " (line: " << parser.errorLine() << ")";
            }
            QVariantMap real = result["profile"].toMap();
            opp->setRealData(real["shortname"].toString(),
                             real["boatname"].toString(),
                             real["description"].toString(),
                             real["flag"].toString());
            getNxtOppData();
            break;
        }
    }
}
bool opponentList::isBoatVLM(QString id)
{
    QList<boatVLM*> * listBoats=parent->getBoats();
    for (int n=0;n<listBoats->count();++n)
    {
        if(listBoats->at(n)->getId()==id.toInt() && listBoats->at(n)->isActive())
            return true;
    }
    return false;
}

QStringList opponentList::readData(QString in_data,int type)
{
    QString begin_str;
    QString end_str = ")";
    if(type==OPP_TYPE_POSITION)
        begin_str = "GLatLng(";
    else
        begin_str = "openInfoWindowHtml(";
    QString sub_str;
    QStringList lst;
    bool stop=false;
    int pos=0;
    int end;
    while(!stop)
    {
        pos = in_data.indexOf(begin_str,pos);
        if(pos==-1)
            stop=true;
        else
        {
            pos+=begin_str.size();
            end = in_data.indexOf(end_str,pos);
            if(end==-1)
                continue;
            lst.append(QString(in_data.mid(pos,end-pos)));
            pos=end;
        }
    }
    return lst;
}

void opponentList::getTrace(QByteArray buff, QList<vlmPoint> * trace)
{
    QJson::Parser parser;
    bool ok;

    /* clear current trace*/
    //trace->clear();

    QVariantMap result = parser.parse (buff, &ok).toMap();
    if (!ok) {
        qWarning() << "Error parsing json data " << buff;
        qWarning() << "Error: " << parser.errorString() << " (line: " << parser.errorLine() << ")";
    }

    if(checkWSResult(buff,"OppList_getTrack",main))
    {
        int nbPoints= result["nb_tracks"].toInt();
        //qWarning() << "Nb point in track: " << nbPoints;
        if(nbPoints > 0)
        {
            //int i=0;
            foreach (QVariant pos, result["tracks"].toList())
            {
                QList<QVariant> pos_list = pos.toList();
                if(!trace->isEmpty() && trace->last().timeStamp>=pos_list[0].toInt())
                    continue;
                double lon = pos_list[1].toDouble()/1000;
                double lat = pos_list[2].toDouble()/1000;
                //qWarning() << i << ": " << QDateTime::fromTime_t(pos_list[0].toInt()) << " - " << lon << "," << lat;
                vlmPoint pt(lon,lat);
                pt.timeStamp=pos_list[0].toInt();
                //trace->prepend(pt);
                trace->append(pt);
                //i++;
            }
        }
    }
}
