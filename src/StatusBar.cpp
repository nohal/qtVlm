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
    Util::setFontDialog(this);
#ifdef __MAC_QTVLM
    QFont font(Settings::getSetting("defaultFontName",QApplication::font().family()).toString());
    double fontSize=Settings::getSetting("applicationFontSize",8.0).toDouble();
    font.setPointSizeF(fontSize);
#else
    QFontInfo finfo = fontInfo();
    QFont font("", finfo.pointSize(), QFont::Normal, false);
    font.setStyleHint(QFont::TypeWriter);
    font.setFamily("Courier");
    font.setPointSizeF(Settings::getSetting("applicationFontSize",8.0).toDouble());
    font.setFixedPitch(true);
    setStyleSheet("QStatusBar::item {border: 0px;}");
#endif

    setFont(font);

    stBar_label_1 = new QLabel("Welcome in QtVlm", this);
    stBar_label_1->setStyleSheet("color: rgb(0, 0, 255);");
    stBar_label_1->setFont(font);
    this->addWidget(stBar_label_1);
    font.setBold(true);
    stBar_label_2 = new QLabel("", this);
    stBar_label_2->setStyleSheet("color: rgb(255, 0, 0);");
    stBar_label_2->setFont(font);
    this->addWidget(stBar_label_2);

    font.setBold(false);
    stBar_label_3 = new QLabel("", this);
    stBar_label_3->setFont(font);
    this->addWidget(stBar_label_3);
    font.setFixedPitch(false);

    showingSelectionMessage=false;

    //Util::setFontDialog(this);
    mainWindow->setStatusBar(this);
}

void StatusBar::showGribData(double x,double y)
{
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
            debugPOI = this->my_centralWidget->slot_addPOI("debug",0,b,a,-1,false,false,this->selectedBoat);
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

    if(showingSelectionMessage)
    {
        showingSelectionMessage=false;
        clearMessage();
    }

    if(!currentMessage().isEmpty())
        return;

    QString label1= Util::pos2String(TYPE_LAT,y) + " " + Util::pos2String(TYPE_LON,x);
    if(mainWindow->getSelectedBoat())
    {
        Orthodromie oo(mainWindow->getSelectedBoat()->getLon(),mainWindow->getSelectedBoat()->getLat(),x,y);
        label1=label1+QString().sprintf(" - %6.2f",oo.getAzimutDeg())+tr("deg")+
                QString().sprintf(" %7.2fNM",oo.getDistance());
    }
    stBar_label_1->setText(label1);

    DataManager * dataManager=my_centralWidget->get_dataManager();
    Terrain * terrain=my_centralWidget->get_terrain();
    MapDataDrawer * mapDrawer=my_centralWidget->get_mapDataDrawer();

    res.clear();

    if(dataManager && terrain && mapDrawer) {

        /* get BG data */
        int mode=terrain->get_colorMapMode();
        int levelType=terrain->get_colorMapLevelType();
        int levelValue=terrain->get_colorMapLevelValue();

        if(mode!=DATA_NOTDEF)
            res = compute_dataTxt(dataManager,mapDrawer,dataManager->get_dataTypes(),mode,levelType,levelValue,x,y);

        /* frst arrow */
        int arwMode=terrain->get_frstArwMode();
        int arwLevelType=terrain->get_frstArwLevelType();
        int arwLevelValue=terrain->get_frstArwLevelValue();

        if(arwMode!=DATA_NOTDEF && (arwMode!=mode || arwLevelType!=levelType || arwLevelValue!=levelValue)) {
            QString s=compute_dataTxt(dataManager,mapDrawer,dataManager->get_arrowTypesFst(),arwMode,arwLevelType,arwLevelValue,x,y);
            if(!s.isEmpty())
                res += " - " + s;
        }

        /* sec arrow */
        arwMode=terrain->get_secArwMode();
        arwLevelType=terrain->get_secArwLevelType();
        arwLevelValue=terrain->get_secArwLevelValue();

        if(arwMode!=DATA_NOTDEF && (arwMode!=mode || arwLevelType!=levelType || arwLevelValue!=levelValue)) {
            res += compute_dataTxt(dataManager,mapDrawer,dataManager->get_arrowTypesSec(),arwMode,arwLevelType,arwLevelValue,x,y);
        }
    }

    stBar_label_2->setText(res);
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
                res = " - " + mapDataTypes->value(mode).at(1);
                res += " " + Util::formatData(mode,v1,v2);
            }
        }
        else {
            //qWarning() << "[showGribData] 1D " << mode << " / " << levelType << " / " << levelValue;
            double v1=dataManager->getInterpolatedValue_1D(mode,levelType,levelValue,x,y,
                                                           dataManager->get_currentDate());
            //qWarning() << "[showGribData] val " << v1;
            res = " - " + mapDataTypes->value(mode).at(1);
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
            + Util::formatPosition(x0,y0)
            + " -> "
            + Util::formatPosition(x1,y1);

    Orthodromie orth(x0,y0, x1,y1);
    QString s;
    message = message+ "   "
                + tr("(dist.orthodromique:")
                + Util::formatDistance(orth.getDistance())
//                + tr("  init.dir: %1deg").arg(qRound(orth.getAzimutDeg()))
                + tr("  init.dir: %1deg").arg(s.sprintf("%.1f",orth.getAzimutDeg()))
                + ")";

    showingSelectionMessage=true;
    showMessage(message);

}

void StatusBar::drawVacInfo(void)
{
    boat * selBoat = mainWindow->getSelectedBoat();
    if(selBoat && selBoat->get_boatType()==BOAT_VLM
            && currentMessage().isEmpty())
    {
        QDateTime lastVac_date;
        lastVac_date.setTimeSpec(Qt::UTC);
        lastVac_date.setTime_t(((boatVLM*)selBoat)->getPrevVac());
        stBar_label_3->setText("- "+ tr("Derniere synchro") + ": " + lastVac_date.toString(tr("dd-MM-yyyy, HH:mm:ss")) + " - "+
                               tr("Prochaine vac dans") + ": " + QString().setNum(mainWindow->get_nxtVac_cnt()) + "s");
    }
}
