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

#include "opponentBoat.h"

#include "boatAccount.h"
#include "MainWindow.h"
#include "settings.h"
#include "mycentralwidget.h"
#include "Projection.h"
#include "vlmLine.h"
#include "Util.h"

/****************************************
* Opponent methods
****************************************/

opponent::opponent(QColor color, QString idu,QString race, float lat, float lon, QString login,
                            QString name,Projection * proj,MainWindow *main, myCentralWidget *parentWindow):QGraphicsWidget()
{
    init(color,false,idu,race,lat,lon,login,name,proj,main,parentWindow);
}

opponent::opponent(QColor color, QString idu,QString race,Projection * proj,MainWindow *main, myCentralWidget *parentWindow):QGraphicsWidget()
{
    init(color,true,idu,race,0,0,"","",proj,main,parentWindow);
}

void opponent::init(QColor color,bool isQtBoat,QString idu,QString race, float lat, float lon, QString login,
                            QString name,Projection * proj,MainWindow *main, myCentralWidget *parentWindow)
{
    this->idu=idu;
    this->idrace=race;
    this->lat=lat;
    this->lon=lon;
    this->login=login;
    this->name=name;
    this->proj=proj;
    this->main=main;
    this->parentWindow=parentWindow;

    this->opp_trace=1;
    this->labelHidden=parentWindow->get_shLab_st();
    connect(parentWindow, SIGNAL(showALL(bool)),this,SLOT(slot_shShow()));
    connect(parentWindow, SIGNAL(hideALL(bool)),this,SLOT(slot_shHidden()));
    connect(parentWindow, SIGNAL(shOpp(bool)),this,SLOT(slot_shOpp()));
    connect(parentWindow, SIGNAL(shLab(bool)),this,SLOT(slot_shLab()));
    width=height=0;

    parentWindow->getScene()->addItem(this);

    setZValue(Z_VALUE_OPP);
    setFont(QFont("Helvetica",9));
    setData(0,OPP_WTYPE);

    fgcolor = QColor(0,0,0);
    int gr = 255;
    bgcolor = QColor(gr,gr,gr,150);

    this->isQtBoat = isQtBoat;
    trace.clear();
    trace_drawing = new vlmLine(proj,parentWindow->getScene(),Z_VALUE_OPP);
    trace_drawing->setLinePen(QPen(color));
    trace_drawing->setPointMode(color);

    myColor = color;

    paramChanged();

    if(!isQtBoat&& !parentWindow->get_shOpp_st())
        show();
    else
        hide();

    connect(proj, SIGNAL(projectionUpdated()), this, SLOT(updateProjection()));
    connect(main,SIGNAL(paramVLMChanged()),this,SLOT(paramChanged()));


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
    return QRectF(0,0,width,height);
}

void opponent::paint(QPainter * pnt, const QStyleOptionGraphicsItem * , QWidget * )
{
    if(isQtBoat || parentWindow->get_shOpp_st())
    {
        hide();
        return;
    }

    int dy = height/2;

    QFontMetrics fm(font());
    if(!labelHidden)
    {
        pnt->fillRect(9,0, width-10,height-1, QBrush(bgcolor));
        pnt->setFont(font());
        pnt->drawText(10,fm.height()-2,my_str);
    }
    QPen pen(myColor);
    pen.setWidth(4);
    pnt->setPen(pen);
    pnt->fillRect(0,dy-3,7,7, QBrush(myColor));

    int g = 60;
    pen = QPen(QColor(g,g,g));
    pen.setWidth(1);
    pnt->setPen(pen);
    if(!labelHidden)
        pnt->drawRect(9,0,width-10,height-1);
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
    switch(label_type)
    {
        case OPP_SHOW_LOGIN:
            my_str = login;
            str2 = idu + " - " + name;
            break;
        case OPP_SHOW_NAME:
            my_str = name;
            str2 = idu + " - " + login;
            break;
        case OPP_SHOW_IDU:
            my_str = idu;
            str2 = login + " - " + name;
            break;
    }
    setToolTip(str2);
    /* compute size */
    /* computing widget size */
    QFontMetrics fm(font());
    prepareGeometryChange();
    width = fm.width(my_str) + 10 + 2;
    height = qMax(fm.height()+2,10);

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
        #warning why doing this ?
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
    this->labelHidden=false;
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

#define OPP_MODE_REFRESH   0
#define OPP_MODE_NEWLIST   1

opponentList::opponentList(Projection * proj,MainWindow * main,myCentralWidget * parent, inetConnexion * inet) :
        QWidget(parent),
        inetClient(inet)
{
    inetClient::setName("OpponentList");
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
}

QString opponentList::getRaceId()
{
    if(opponent_list.size()<=0)
        return "";

    return opponent_list[0]->getRace();
}

void opponentList::setBoatList(QString list_txt,QString race,bool force)
{
    //qWarning() << "SetBoatList";
    if(!hasInet() || hasRequest())
    {
        qWarning() << "getOpponents bad state in inet - setBoatList: " << hasInet() << " " << hasRequest();
        return;
    }

    /* is a list defined ? */
    if(opponent_list.size()>0)
    {
        if(!force && opponent_list[0]->getRace() == race) /* it is the same race */
        { /* compare if same opp list */
            /* for now it is the same => refresh*/
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
    currentRace = race;
    currentMode = OPP_MODE_NEWLIST;

    if(currentList.size() > 0)
        getNxtOppData();
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
        qWarning() << "getOpponents bad state in inet - refreshData: " << hasInet() << " " << hasRequest();
        return;
    }

    if(opponent_list.size()<=0)
        return;

    currentRace = opponent_list[0]->getRace();
    currentOpponent = 0;
    currentMode=OPP_MODE_REFRESH;
    getNxtOppData();
}

void opponentList::getNxtOppData()
{
    int listSize = (currentMode==OPP_MODE_REFRESH?opponent_list.size():currentList.size());
    QString idu;
    if(currentOpponent>=listSize)
    {
        parent->update();
        return;
    }

    idu = (currentMode==OPP_MODE_REFRESH?opponent_list[currentOpponent]->getIduser():currentList[currentOpponent]);

    if(main->isBoat(idu))
    {
        if(currentMode==OPP_MODE_REFRESH)
            opponent_list[currentOpponent]->setIsQtBoat(true);
        else
            opponent_list.append(new opponent(colorTable[currentOpponent],idu,currentRace,proj,main,parent));

        currentOpponent++;
        getNxtOppData();
        return;
    }

    QString page;
    QTextStream(&page)
                        << "/gmap/index.php?"
                        << "type=ajax&riq=pos"
                        << "&idusers="
                        << idu
                        << "&idraces="
                        << currentRace;
    currentOpponent++;

    clearCurrentRequest();
    inetGet(OPP_BOAT_DATA,page);
}

void opponentList::requestFinished (QByteArray res_byte)
{
    QString page;
    QStringList list_res;
    QStringList lsval,lsval2;
    float lat,lon;
    QString login,name;
    QString idu;
    QString res(res_byte);

    switch(getCurrentRequest())
    {
        case OPP_BOAT_DATA:
            list_res=readData(res,OPP_TYPE_NAME);
            if(list_res.size()>0)
            {
                /* only one data should be returned */
                lsval=list_res[0].split(",");
                list_res=readData(res,OPP_TYPE_POSITION);
                lsval2=list_res[0].split(",");
                if (lsval2.size() == 2)
                {
                    lat=lsval2[0].toFloat();
                    lon=lsval2[1].toFloat();
                    login=lsval[0].mid(4,lsval[0].size()-4-4);
                    name=lsval[1].mid(5,lsval[1].size()-5-1);
                    if(!(lat==0 && lon ==0))
                    {
                        idu = (currentMode==OPP_MODE_REFRESH?
                            opponent_list[currentOpponent-1]->getIduser():currentList[currentOpponent-1]);

                        /*qWarning() << login << "-" << name
                            << " at (" << lat << "," << lon << ") - idu"
                            << idu ;*/
                        if(currentMode==OPP_MODE_REFRESH)
                            opponent_list[currentOpponent-1]->setNewData(lat,lon,name);
                        else
                            opponent_list.append(new opponent(colorTable[currentOpponent-1],idu,currentRace,
                                                        lat,lon,login,name,proj,main,parent));

                    QTextStream(&page)
                        << "/gmap/index.php?"
                        << "type=ajax&riq=trj"
                        << "&idusers="
                        << idu
                        << "&idraces="
                        << currentRace;
                    clearCurrentRequest();
                    inetGet(OPP_BOAT_TRJ,page);
                    break;
                    }
                }
            }
            getNxtOppData();
            break;
        case OPP_BOAT_TRJ:
            if(currentMode==OPP_MODE_REFRESH)
            {
                getTrace(res,opponent_list[currentOpponent-1]->getTrace());
                if(!parent->get_shOpp_st())
                    opponent_list[currentOpponent-1]->drawTrace();
            }
            else
            {
                if(!opponent_list.isEmpty())
                {
                    getTrace(res,opponent_list.last()->getTrace());
                    if(!parent->get_shOpp_st())
                        opponent_list.last()->drawTrace();
                }
            }
            getNxtOppData();
            break;
    }
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

void opponentList::getTrace(QString buff, QList<vlmPoint> * trace)
{
    QStringList lsval,lsval2;

    /* clear current trace*/
    trace->clear();

    /* parse buff */
    if(!buff.isEmpty())
    {
        lsval = readData(buff,OPP_TYPE_POSITION);
        for(int i=0;i<lsval.size();i++)
        {
            lsval2=lsval[i].split(",");
            if (lsval2.size() == 2)
            {
                vlmPoint pt(lsval2[1].toFloat(),lsval2[0].toFloat());
                trace->append(pt);
            }
        }
    }
}
