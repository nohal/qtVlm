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

#include "routeInfo.h"
#include "ui_routeInfo.h"
#include "Util.h"
#include "settings.h"
#include "MapDataDrawer.h"
#include "Terrain.h"


routeInfo::routeInfo(myCentralWidget *parent, ROUTE *route) :
    QDialog(parent)
{
    this->setWindowFlags(Qt::Tool);
    setupUi(this);
    Util::setFontDialog(this);
    this->route=route;
    this->parent=parent;
    this->setWindowTitle(QObject::tr("Information au point d'interpolation pour ")+route->getName());
    drawBoat.moveTo(20,10);
    drawBoat.quadTo(12,22,17,30);
    drawBoat.lineTo(23,30);
    drawBoat.quadTo(28,22,20,10);
    //qWarning()<<"end of roadInfo init";
}
void routeInfo::setValues(double twd, double tws, double twa, double bs, double hdg, double cnm, double dnm, bool engineUsed,
                          bool south, double cog, double sog, double cs, double cd, double wh, double wd, bool night, double wsh)
{
    //qWarning()<<"inside routeInfo::setValues()";
    TWD->setValue(twd);
    TWS->setValue(tws);
    TWA->setValue(qAbs(twa));
    BS->setValue(bs);
    HDG->setValue(hdg);
    CNM->setValue(cnm);
    DNM->setValue(dnm);
    COG->setValue(cog);
    SOG->setValue(sog);
    wave_dir->setValue(wd);
    wave_height->setValue(wh);
    sig_wave_height->setValue(wsh);
    if(cs<0)
    {
        CS->setValue(-1);
        CD->setValue(-1);
    }
    else
    {
        CS->setValue(cs);
        CD->setValue(cd);
    }
    double Y=90-qAbs(twa);
    double a=tws*cos(degToRad(Y));
    double b=tws*sin(degToRad(Y));
    double bb=b+bs;
    double aws=sqrt(a*a+bb*bb);
    double awa=90-radToDeg(atan(bb/a));
    AWA->setValue(awa);
    AWS->setValue(aws);
    if(engineUsed)
    {
        amure->setText(tr("Au moteur"));
    }
    else
    {
        if(twa>=0)
            amure->setText(tr("Babord amure"));
        else
            amure->setText(tr("Tribord amure"));
    }
    QPixmap img(40,40);
    if(engineUsed)
    {
        img.load(appFolder.value("img")+"propeller.png");
    }
    else
    {
        if(night)
            img.fill(QColor(238,241,125));
        else
            img.fill(QColor(105,109,124));
    }
    QPainter pnt(&img);
    pnt.setRenderHint(QPainter::Antialiasing);
    QPen pen(Qt::gray);
    pen.setWidthF(0.5);
    QRadialGradient radialGrad(QPointF(20, 20), 15);
    radialGrad.setColorAt(0, Qt::white);
    radialGrad.setColorAt(0.8, Qt::blue);
    pen.setColor(Qt::gray);
    pnt.setPen(pen);
    pnt.setBrush(QBrush(radialGrad));
    QMatrix mat=QMatrix().translate(20,20).rotate(hdg).translate(-20,-20);
    pnt.setMatrix(mat);
    pnt.drawPath(this->drawBoat);
    pnt.setMatrixEnabled(false);
    QColor rgb=Qt::white;
    if(parent->get_mapDataDrawer())
    {
        rgb=QColor(MapDataDrawer::getWindColorStatic(tws,true));
        rgb.setAlpha(255);
    }
    pen.setColor(rgb);
    pen.setWidth(2);
    pnt.setPen(pen);
    pnt.setBrush(Qt::NoBrush);
    if(!engineUsed)
        this->drawWindArrowWithBarbs(pnt,20,20,
                                 tws,twd,
                                 south);
    this->iconWind->setPixmap(img);
}

routeInfo::~routeInfo()
{
}
void routeInfo::closeEvent(QCloseEvent *)
{
    QPoint position=this->pos();
    route->setShowInterpolData(false);
    this->move(position);
}
void routeInfo::resizeEvent(QResizeEvent *e)
{
    QDialog::resizeEvent(e);
    Settings::setSetting(this->objectName()+".height",this->height());
    Settings::setSetting(this->objectName()+".width",this->width());
}

void routeInfo::drawTransformedLine( QPainter &pnt,
        double si, double co,int di, int dj, int i,int j, int k,int l)
{
    int ii, jj, kk, ll;
    ii = (int) (i*co-j*si +0.5) + di;
    jj = (int) (i*si+j*co +0.5) + dj;
    kk = (int) (k*co-l*si +0.5) + di;
    ll = (int) (k*si+l*co +0.5) + dj;
    pnt.drawLine(ii, jj, kk, ll);
}
//-----------------------------------------------------------------------------
void routeInfo::drawPetiteBarbule(QPainter &pnt, bool south,
                    double si, double co, int di, int dj, int b)
{
    if (south)
        drawTransformedLine(pnt, si,co, di,dj,  b,0,  b+2, -5);
    else
        drawTransformedLine(pnt, si,co, di,dj,  b,0,  b+2, 5);
}
//---------------------------------------------------------------
void routeInfo::drawGrandeBarbule(QPainter &pnt, bool south,
                    double si, double co, int di, int dj, int b)
{
    if (south)
        drawTransformedLine(pnt, si,co, di,dj,  b,0,  b+4,-10);
    else
        drawTransformedLine(pnt, si,co, di,dj,  b,0,  b+4,10);
}
//---------------------------------------------------------------
void routeInfo::drawWindArrowWithBarbs(QPainter &pnt, int i, int j, double vkn, double ang,
                        bool south)
{
    ang = degToRad(ang);
    ang-=PI_2;
    double si=sin(ang),  co=cos(ang);


    if (vkn < 1)
    {
        int r = 5;     // vent tres faible, dessine un cercle
        pnt.drawEllipse(i-r,j-r,2*r,2*r);
    }
    else {
        const int windBarbuleSize = 30;     // longueur des fleches avec barbules
        // Fleche centree sur l'origine
        int dec = -windBarbuleSize/2;
        drawTransformedLine(pnt, si,co, i,j,  dec,0,  dec+windBarbuleSize, 0);   // hampe
        drawTransformedLine(pnt, si,co, i,j,  dec,0,  dec+5, 2);    // fleche
        drawTransformedLine(pnt, si,co, i,j,  dec,0,  dec+5, -2);   // fleche

                int b1 = dec+windBarbuleSize -4;  // position de la 1ere barbule
                if (vkn >= 7.5  &&  vkn < 45 ) {
                        b1 = dec+windBarbuleSize;  // position de la 1ere barbule si >= 10 noeuds
                }

        if (vkn < 7.5) {  // 5 ktn
            drawPetiteBarbule(pnt,south, si,co, i,j, b1);
        }
        else if (vkn < 12.5) { // 10 ktn
            drawGrandeBarbule(pnt,south, si,co, i,j, b1);
        }
        else if (vkn < 17.5) { // 15 ktn
            drawGrandeBarbule(pnt,south, si,co, i,j, b1);
            drawPetiteBarbule(pnt,south, si,co, i,j, b1-4);
        }
        else if (vkn < 22.5) { // 20 ktn
            drawGrandeBarbule(pnt,south, si,co, i,j, b1);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-4);
        }
        else if (vkn < 27.5) { // 25 ktn
            drawGrandeBarbule(pnt,south, si,co, i,j, b1);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-4);
            drawPetiteBarbule(pnt,south, si,co, i,j, b1-8);
        }
        else if (vkn < 32.5) { // 30 ktn
            drawGrandeBarbule(pnt,south, si,co, i,j, b1);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-4);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-8);
        }
        else if (vkn < 37.5) { // 35 ktn
            drawGrandeBarbule(pnt,south, si,co, i,j, b1);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-4);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-8);
            drawPetiteBarbule(pnt,south, si,co, i,j, b1-12);
        }
        else if (vkn < 45) { // 40 ktn
            drawGrandeBarbule(pnt,south, si,co, i,j, b1);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-4);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-8);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-12);
        }
        else if (vkn < 55) { // 50 ktn
            drawTriangle(pnt,south, si,co, i,j, b1-4);
        }
        else if (vkn < 65) { // 60 ktn
            drawTriangle(pnt,south, si,co, i,j, b1-4);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-8);
        }
        else if (vkn < 75) { // 70 ktn
            drawTriangle(pnt,south, si,co, i,j, b1-4);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-8);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-12);
        }
        else if (vkn < 85) { // 80 ktn
            drawTriangle(pnt,south, si,co, i,j, b1-4);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-8);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-12);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-16);
        }
        else { // > 90 ktn
            drawTriangle(pnt,south, si,co, i,j, b1-4);
            drawTriangle(pnt,south, si,co, i,j, b1-12);
        }
    }
}
void routeInfo::drawTriangle(QPainter &pnt, bool south,
                    double si, double co, int di, int dj, int b)
{
    if (south) {
        drawTransformedLine(pnt, si,co, di,dj,  b,0,  b+4,-10);
        drawTransformedLine(pnt, si,co, di,dj,  b+8,0,  b+4,-10);
    }
    else {
        drawTransformedLine(pnt, si,co, di,dj,  b,0,  b+4,10);
        drawTransformedLine(pnt, si,co, di,dj,  b+8,0,  b+4,10);
    }
}
