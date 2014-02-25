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

#include <QDebug>

#include "MainWindow.h"
#include "StatusBar.h"
#include "Orthodromie.h"
#include "Util.h"
#include "boat.h"
#include "boatVLM.h"
#include "settings.h"
#include "DataManager.h"
#include "MapDataDrawer.h"
#include "Terrain.h"

StatusBar::StatusBar(MainWindow * mainWindow) : QStatusBar(mainWindow) {
    this->mainWindow=mainWindow;
    my_centralWidget = mainWindow->getMy_centralWidget();
    Util::setFontObject(this);
#ifdef __MAC_QTVLM
    QFont font(Settings::getSetting(defaultFontName).toString());
    double fontSize=Settings::getSetting(applicationFontSize).toDouble();
    font.setPointSizeF(fontSize);
#else
    QFontInfo finfo = fontInfo();
    QFont font("", finfo.pointSize(), QFont::Normal, false);
    font.setStyleHint(QFont::TypeWriter);
    font.setFamily("Courier");
    font.setPointSizeF(Settings::getSetting(applicationFontSize).toDouble());
    font.setFixedPitch(true);
#endif
#ifdef QT_V5
    if(Settings::getSetting(fusionStyle).toInt()==1)
        this->setStyleSheet("color: rgb(234, 221, 21);");
    else
#endif
        this->setStyleSheet("color: rgb(0, 0, 255);");

    this->setFont(font);
    //this->setTextFormat(Qt::RichText);

    labelOrtho = new QLabel("Welcome in QtVlm", this);
#ifdef QT_V5
    if(Settings::getSetting(fusionStyle).toInt()==1)
        labelOrtho->setStyleSheet("color: rgb(234, 221, 21);");
    else
#endif
        labelOrtho->setStyleSheet("color: rgb(0, 0, 255);");
    labelOrtho->setFont(font);
    labelOrtho->setTextFormat(Qt::RichText);

    labelGrib = new QLabel("", this);
#ifdef QT_V5
    if(Settings::getSetting(fusionStyle).toInt()==1)
        labelGrib->setStyleSheet("color: rgb(234, 154, 84);");
    else
#endif
        labelGrib->setStyleSheet("color: rgb(255, 0, 0);");
    labelGrib->setFont(font);
    labelGrib->setTextFormat(Qt::RichText);
    /*font=labelGrib->font();
    font.setBold(true);
    labelGrib->setFont(font);*/

    labelEta=new QLabel("",this);
#ifdef QT_V5
    if(Settings::getSetting(fusionStyle).toInt()==1)
        labelEta->setStyleSheet("color: rgb(51, 212, 195);");
    else
#endif
        labelEta->setStyleSheet("color: rgb(33,33,179);");
    labelEta->setFont(font);
    labelEta->setTextFormat(Qt::RichText);
    labelEta->setAlignment(Qt::AlignRight);


#ifdef __ANDROID__
    labelEta->setWordWrap(true);
    this->addWidget(labelGrib,0);
#else
    separator=new QLabel(" - ");
#ifdef QT_V5
    if(Settings::getSetting(fusionStyle).toInt()==1)
        separator->setStyleSheet("color: rgb(234, 221, 21);");
    else
#endif
        separator->setStyleSheet("color: rgb(0, 0, 255);");
    separator->setFont(font);
    this->addWidget(labelGrib,0);
    this->addWidget(separator,0);
    this->addWidget(labelOrtho,0);

    separator->setVisible(false);
#endif
    this->addPermanentWidget(labelEta,0);
    mainWindow->setStatusBar(this);
}

void StatusBar::showGribData(double x,double y)
{
    if(mainWindow->getMy_centralWidget()->isSelecting()) return;
    clearMessage();
#if 0 /*unflag to visualize closest point to next gate from mouse position*/
    if(!selectedBoat) return;
    QList<vlmLine*> gates=((boatVLM*)mainWindow->selectedBoat)->getGates();
    int nWP=this->selectedBoat->getNWP();

    if(gates.isEmpty() || nWP<=0 || nWP>gates.count())
    {
    }
    else
    {
        vlmLine *porte=NULL;
        for (int i=nWP-1;i<gates.count();++i)
        {
            porte=gates.at(i);
            if (!porte->isIceGate()) break;
        }
        double X,Y;
        proj->map2screenDouble(x,y,&X,&Y);
        double cx=X;
        double cy=Y;
        proj->map2screenDouble(porte->getPoints()->first().lon,porte->getPoints()->first().lat,&X,&Y);
        double ax=X;
        double ay=Y;
        proj->map2screenDouble(porte->getPoints()->last().lon,porte->getPoints()->last().lat,&X,&Y);
        double bx=X;
        double by=Y;
    #if 1 /*remove 1 pixel at each end to make sure we cross*/
        QLineF porteLine(ax,ay,bx,by);
        QLineF p1(porteLine.pointAt(0.5),porteLine.p1());
        p1.setLength(p1.length()-1);
        QLineF p2(porteLine.pointAt(0.5),porteLine.p2());
        p2.setLength(p2.length()-1);
        ax=p1.p2().x();
        ay=p1.p2().y();
        bx=p2.p2().x();
        by=p2.p2().y();
    #endif
        double r_numerator = (cx-ax)*(bx-ax) + (cy-ay)*(by-ay);
        double r_denomenator = (bx-ax)*(bx-ax) + (by-ay)*(by-ay);
        double r = r_numerator / r_denomenator;
    //
        double px = ax + r*(bx-ax);
        double py = ay + r*(by-ay);
    //


    //
    // (xx,yy) is the point on the lineSegment closest to (cx,cy)
    //
        double xx = px;
        double yy = py;
        if ( (r >= 0) && (r <= 1) )
        {
        }
        else
        {
            double dist1 = (cx-ax)*(cx-ax) + (cy-ay)*(cy-ay);
            double dist2 = (cx-bx)*(cx-bx) + (cy-by)*(cy-by);
            if (dist1 < dist2)
            {
                    xx = ax;
                    yy = ay;
            }
            else
            {
                    xx = bx;
                    yy = by;
            }

        }
        double a,b;
        proj->screen2mapDouble(xx,yy,&a,&b);
//        closest=vlmPoint(a,b);
//        Orthodromie oo(lon,lat,a,b);
//        closest.distArrival=oo.getDistance();
//        closest.capArrival=oo.getAzimutDeg();
        if(debugPOI==NULL)
            debugPOI = this->my_centralWidget->slot_addPOI("debug",0,b,a,-1,false,false);
        else
        {
            debugPOI->setLatitude(b);
            debugPOI->setLongitude(a);
            debugPOI->slot_updateProjection();
            QApplication::processEvents();
        }

    }
#endif
    QString res;
    bool hasGribData = false;


    QString label1= Util::pos2String(TYPE_LAT,y) + " " + Util::pos2String(TYPE_LON,x);
    if(mainWindow->getSelectedBoat())
    {
        Orthodromie oo(mainWindow->getSelectedBoat()->getLon(),mainWindow->getSelectedBoat()->getLat(),x,y);
        label1=label1+QString().sprintf(" - %6.2f",oo.getAzimutDeg())+tr("deg")+
                QString().sprintf(" %7.2fNM",oo.getDistance());
    }
    labelOrtho->setText(label1.replace(" ","&nbsp;"));

    DataManager * dataManager=my_centralWidget->get_dataManager();
    Terrain * terrain=my_centralWidget->get_terrain();
    MapDataDrawer * mapDrawer=my_centralWidget->get_mapDataDrawer();

    res.clear();

    if(dataManager && terrain && mapDrawer) {

        /* get BG data */
        int mode=terrain->get_colorMapMode();
        int levelType=terrain->get_colorMapLevelType();
        int levelValue=terrain->get_colorMapLevelValue();

        if(mode!=DATA_NOTDEF) {
            res = compute_dataTxt(dataManager,mapDrawer,dataManager->get_dataTypes(),mode,levelType,levelValue,x,y);
            hasGribData=!res.isEmpty();
        }

        /* frst arrow */
        int arwMode=terrain->get_frstArwMode();
        int arwLevelType=terrain->get_frstArwLevelType();
        int arwLevelValue=terrain->get_frstArwLevelValue();

        if(arwMode!=DATA_NOTDEF && (arwMode!=mode || arwLevelType!=levelType || arwLevelValue!=levelValue)) {
            QString s=compute_dataTxt(dataManager,mapDrawer,dataManager->get_arrowTypesFst(),arwMode,arwLevelType,arwLevelValue,x,y);
            if(!s.isEmpty()) {
                hasGribData=true;
                if(!res.isEmpty()) res += " - ";
                res += s;
            }
        }

        /* sec arrow */
        arwMode=terrain->get_secArwMode();
        arwLevelType=terrain->get_secArwLevelType();
        arwLevelValue=terrain->get_secArwLevelValue();

        if(arwMode!=DATA_NOTDEF && (arwMode!=mode || arwLevelType!=levelType || arwLevelValue!=levelValue)) {
            QString s=compute_dataTxt(dataManager,mapDrawer,dataManager->get_arrowTypesSec(),arwMode,arwLevelType,arwLevelValue,x,y);
            if(!s.isEmpty()) {
                hasGribData=true;
                if(!res.isEmpty()) res += " - ";
                res += s;
            }
        }
    }

    labelGrib->setText("<b>"+res.replace(" ","&nbsp;")+"</b>");
#ifndef __ANDROID__
    //qWarning() << "HasGribData="<< hasGribData;
    separator->setVisible(hasGribData);
#endif
}

QString StatusBar::compute_dataTxt(DataManager * dataManager, MapDataDrawer* mapDrawer,
                                   QMap<int,QStringList> * mapDataTypes,
                                   int mode,int levelType,int levelValue,double x,double y) {
    QString res="";
    dataDrawerInfo * drawerInfo=mapDrawer->get_drawerInfo(mode);    

    if(!mapDataTypes->contains(mode)) {
        qWarning() << "Unkn mode fin dataType: " << mode;
        return res;
    }

    if(drawerInfo && drawerInfo->isOk) {
        /* interpolation */
        if(drawerInfo->is2D) {
            //qWarning() << "[showGribData] 2D";
            double v1,v2;
            int interpol=INTERPOLATION_UKN;
            if(drawerInfo->forcedInterpol)
                interpol=drawerInfo->forcedInterpolType;
            if(dataManager->getInterpolatedValue_2D(mode,drawerInfo->secData_2D,levelType,levelValue,
                                                    x,y,dataManager->get_currentDate(),&v1,&v2,
                                                    interpol,drawerInfo->UV)) {
                res = mapDataTypes->value(mode).at(1);
                res += " " + Util::formatData(mode,v1,v2);
            }
        }
        else {
            //qWarning() << "[showGribData] 1D " << mode << " / " << levelType << " / " << levelValue;
            double v1=dataManager->getInterpolatedValue_1D(mode,levelType,levelValue,x,y,
                                                           dataManager->get_currentDate());
            //qWarning() << "[showGribData] val " << v1;
            res = mapDataTypes->value(mode).at(1);
            res += " " + Util::formatData(mode,v1);
        }
    }
    else
        qWarning() << "[showGribData] no drawer info for data " << mode;
    return res;
}

void StatusBar::showSelectedZone(double x0, double y0, double x1, double y1)
{
    QString message =
            tr("Selection: ")
            + Util::pos2String(TYPE_LAT,y0) + " " + Util::pos2String(TYPE_LON,x0)
            + " -> "
            + Util::pos2String(TYPE_LAT,y1) + " " + Util::pos2String(TYPE_LON,x1);

    Orthodromie orth(x0,y0, x1,y1);
    QString s;
    message = message+ " "
                + tr("(dist.orthodromique:")
                + Util::formatDistance(orth.getDistance())
                + tr("  init.dir: %1deg").arg(s.sprintf("%6.2f",orth.getAzimutDeg()))
                + ")";
    showMessage(message,0);
}

/**********************************************************************/
/*                         ETA                                        */
/**********************************************************************/

void StatusBar::clear_eta(void) {
    labelEta->setText("<b>"+tr("No WP")+"</b>");
    this->labelEta->setMinimumSize(labelEta->sizeHint());
}

void StatusBar::update_eta(QDateTime eta_dtm)
{
    int nbS,j,h,m;
    QString txt;
    eta_dtm.setTimeSpec(Qt::UTC);
    QDateTime now = (QDateTime::currentDateTime()).toUTC();
    nbS=now.secsTo(eta_dtm);
    j = nbS/(24*3600);
    nbS-=j*24*3600;
    h=nbS/3600;
    nbS-=h*3600;
    m=qRound((double) nbS/60.0);
    txt.sprintf("- %dj %02dh%02dm",j,h,m);
    txt.replace("j",tr("j"));
    txt.replace("h",tr("h"));
    txt.replace("m",tr("m"));
    QString myEta=tr(" ETA WP")+": " +eta_dtm.toString(tr("dd-MM-yyyy, HH:mm:ss"));
    myEta.replace(" ","&nbsp;");
    txt.replace(" ","&nbsp;");
    labelEta->setText("<b>"+myEta+" "+txt+"</b>");
}
