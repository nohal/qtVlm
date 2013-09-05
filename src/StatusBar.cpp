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

#include "MainWindow.h"
#include "StatusBar.h"
#include "Orthodromie.h"
#include "Grib.h"
#include "Util.h"
#include "boat.h"
#include "boatVLM.h"
#include "settings.h"
StatusBar::StatusBar(MainWindow * mainWindow) : QStatusBar(mainWindow) {
    this->mainWindow=mainWindow;
    my_centralWidget = mainWindow->getMy_centralWidget();

    QFontInfo finfo = fontInfo();
    QFont font("", finfo.pointSize(), QFont::Normal, false);
    font.setStyleHint(QFont::TypeWriter);
    font.setFamily("Courier");
    font.setFixedPitch(true);
    font.setPointSizeF(Settings::getSetting("applicationFontSize",8.25).toDouble());
    setFont(font);
    setStyleSheet("QStatusBar::item {border: 0px;}");

    stBar_label_1 = new QLabel("Welcome in QtVlm", this);
    stBar_label_1->setFont(font);
    stBar_label_1->setStyleSheet("color: rgb(0, 0, 255);");
    this->addWidget(stBar_label_1);
    font.setBold(true);
    stBar_label_2 = new QLabel("", this);
    stBar_label_2->setFont(font);
    stBar_label_2->setStyleSheet("color: rgb(255, 0, 0);");
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

void StatusBar::showWindData(double x,double y)
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
    QString s, res;
    double a,b;

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

    Grib * grib = my_centralWidget->getGrib();
    bool bo=false;
    res.clear();
    bo=(grib && grib->getInterpolatedValue_byDates(x,y,grib->getCurrentDate(),&a,&b));
    if(bo)
    {
        res = "- " + tr(" Vent") + ": ";
        s.sprintf("%6.2f", radToDeg(b));
        res += s+tr("deg")+" ";
        s.sprintf("%6.2f",a);
        res += s+tr(" kts");
    }
    bo=(grib && grib->getInterpolatedValueCurrent_byDates(x,y,grib->getCurrentDate(),&a,&b));
    if(bo)
    {
        res += " - " + tr(" Courant") + ": ";
        s.sprintf("%6.2f", Util::A360(radToDeg(b)+180.0));
        res += s+tr("deg")+", ";
        s.sprintf("%6.2f",a);
        res += s+tr(" kts");
    }
    else
    {
        grib=my_centralWidget->getGribCurrent();
        bo=(grib && grib->getInterpolatedValueCurrent_byDates(x,y,grib->getCurrentDate(),&a,&b));
        if(bo)
        {
            res += " - " + tr(" Courant") + ": ";
            s.sprintf("%6.2f", Util::A360(radToDeg(b)+180.0));
            res += s+tr("deg")+", ";
            s.sprintf("%6.2f",a);
            res += s+tr(" kts");
        }
    }
    stBar_label_2->setText(res);
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
