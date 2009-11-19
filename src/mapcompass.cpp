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
#include <QMouseEvent>
#include <QPaintEvent>
#include <cmath>

#include "mapcompass.h"
#include "Util.h"
#include "Orthodromie.h"

#define COMPASS_MARGIN 30
#define CROSS_SIZE 8
#define MARK_SIZE 12
#define DELTA 4

mapCompass::mapCompass(Projection * proj,MainWindow * main,Terrain *terre) : QWidget(terre)
{
    size = 310;
    isMoving=false;
    this->proj=proj;
    this->terre=terre;
    this->main=main;
    setGeometry(200,200,size,size);
    setMouseTracking(true);
}

bool mapCompass::isUnder(int x_val, int y_val,bool strict)
{
    if(strict)
    {
        int center_x=x()+width()/2;
        int center_y=y()+height()/2;
        if(x_val>center_x-4 && x_val<center_x+4 && y_val>center_y-4 && y_val<center_y+4)
            return true;
    }
    else
    {
        if(x_val>x() && x_val < (x()+width()) && y_val>y() && y_val < (y()+height()))
            return true;
    }
    return false;
}

void  mapCompass::paintEvent(QPaintEvent *)
{

    QPainter pnt(this);

    QFontMetrics fm(pnt.font());
    int str_w,str_h,x,y;
    double wind_speed;
    double lat,lon;
    float WP_lat,WP_lon;
    float WP_angle=-1;
    float WP_dist=-1;
    float angle;
    QString str;
    float bvmg_up;
    float bvmg_down;
    proj->screen2map(this->x()+size/2,this->y()+size/2,&lon,&lat);

    QPen pen(QColor(Qt::black));

    str_h=fm.height();
    int diam=(size-2*COMPASS_MARGIN)/2;

    pnt.fillRect(0,0, width(),height(), QBrush(QColor(255,255,255,00)));

    pen.setWidth(1);
    pnt.setPen(pen);

    /* move center to center of circle */
    pnt.translate(size/2,size/2);

    /* center cross */

    pnt.drawLine(-(CROSS_SIZE/2),0,(CROSS_SIZE/2),0);
    pnt.drawLine(0,-(CROSS_SIZE/2),0,(CROSS_SIZE/2));

    /* external compass : direction */

    /* external numbers */
    for(int i=0;i<360;i+=10)
    {
        str=QString().setNum(i);
        str_w=fm.width(str);
        angle=degToRad(i-90);
        x=(int)((diam+12)*cos(angle)-(float)(str_w/2));
        y=(int)((diam+12)*sin(angle)+(float)(str_h/2));
        pnt.drawText(x,y,str);
    }

    /* External marks */
    for(int i=0;i<360;i++)
    {
//        if(i==0)
//            pen.setColor(Qt::red);
//        else
//            pen.setColor(Qt::black);

        if(i%10==0) // big marks
        {
            pen.setWidth(2);
            pnt.setPen(pen);
            pnt.drawLine(0,-diam+MARK_SIZE,0,-diam);
        }
        else if(i%5==0) // medium marks
        {
            pen.setWidth(1);
            pnt.setPen(pen);
            pnt.drawLine(0,-diam+MARK_SIZE,0,-diam+6);
        }
        else if(i%1==0) // small marks
        {
            pen.setWidth(1);
            pnt.setPen(pen);
            pnt.drawLine(0,-diam+MARK_SIZE,0,-diam+10);
        }
        pnt.rotate(1);
    }

    /* WP info */
    main->getBoatWP(&WP_lat,&WP_lon);
    qWarning() << "Wp pos: " << WP_lat << "," << WP_lon;
    if(WP_lat!=-1 && WP_lat!=0 && WP_lon!=-1 && WP_lon!=0)
    {
        Orthodromie orth(lon,lat,WP_lon,WP_lat);
        WP_angle=qRound(orth.getAzimutDeg());
        WP_dist=orth.getDistance();
        pnt.rotate(WP_angle);
        pen.setColor(Qt::red);
        pen.setWidth(2);
        pnt.setPen(pen);
        pnt.drawLine(0,-diam+MARK_SIZE,0,-diam+MARK_SIZE+DELTA);
        pnt.rotate(-WP_angle);
    }

    /* Internal compas : WIND */

    /* internal numbers */
    /*compute start angle = wind angle*/
    wind_angle=0;
    if(terre->getGrib()->isOk())
        terre->getGrib()->getInterpolatedValue_byDates(lon,lat,
                                          terre->getGrib()->getCurrentDate(),&wind_speed,&wind_angle);
    wind_angle=radToDeg(wind_angle);
    main->getBoatBvmg(&bvmg_up,&bvmg_down,wind_speed);
    pen.setColor(Qt::black);
    pnt.setPen(pen);

    for(int i=0;i<360;i+=20)
    {
        str=QString().setNum(i>180?i-360:i);
        str_w=fm.width(str);
        angle=degToRad((i+wind_angle)-90);
        x=(int)((diam-2*MARK_SIZE-4*DELTA)*cos(angle)-(float)(str_w/2));
        y=(int)((diam-2*MARK_SIZE-4*DELTA)*sin(angle)+(float)(str_h/2));
        pnt.drawText(x,y,str);
    }

    /* internal marks */
    pnt.rotate(wind_angle);
    int i_180;
    for(int i=0;i<360;i++)
    {
        if(i>180)
            i_180=360-i;
        else
            i_180=i;
        if((i_180<qRound(bvmg_up)  || i_180>qRound(bvmg_down)) && qRound(bvmg_up)!=-1)
            pen.setColor(Qt::red);
        else
            pen.setColor(Qt::black);
        pen.setStyle(Qt::SolidLine);
        if(i_180==qRound(bvmg_up)  || i_180==qRound(bvmg_down))
        {
            pen.setColor(Qt::red);
            pen.setStyle(Qt::DashLine);
            pen.setWidth(1);
            pnt.setPen(pen);
            pnt.drawLine(0,0,0,-diam-30);
        }
        else if(i%10==0) // big marks
        {
            pen.setWidth(2);
            pnt.setPen(pen);
            pnt.drawLine(0,-diam+DELTA+MARK_SIZE,0,-diam+DELTA+2*MARK_SIZE);
        }
        else if(i%5==0) // medium marks
        {
            pen.setWidth(1);
            pnt.setPen(pen);
            pnt.drawLine(0,-diam+DELTA+MARK_SIZE,0,-diam+DELTA+2*MARK_SIZE-6);
        }
        else if(i%1==0) // small marks
        {
            pen.setWidth(1);
            pnt.setPen(pen);
            pnt.drawLine(0,-diam+DELTA+MARK_SIZE,0,-diam+DELTA+2*MARK_SIZE-10);
        }
        pnt.rotate(1);
    }
    pen.setColor(Qt::black);
    pnt.rotate(-wind_angle);
    pnt.setPen(pen);

    /* text de lattitude/longitude*/
    if(isMoving)
    {
        QString lat_txt,lon_txt,wind_txt,s,WP_txt;
        int lat_w,wind_w,WP_w;

        QFont fnt=pnt.font();
        fnt.setBold(true);
        QFontMetrics fm2(fnt);
        pnt.setFont(fnt);
        str_h=fm.height();

        lat_txt=Util::pos2String(TYPE_LAT,lat);
        lon_txt=Util::pos2String(TYPE_LON,lon);
        s.sprintf("%.0f", wind_angle);
        wind_txt += tr("Vent: ") + s + tr("°");
        lat_w=fm2.width(lat_txt);
        wind_w=fm2.width(wind_txt);

        pnt.drawText(-lat_w-2,-str_h/2,lat_txt);
        pnt.drawText(2,-str_h/2,lon_txt);
        pnt.drawText(-wind_w/2,-str_h/2-str_h,wind_txt);

        /* WP Data */
        if(WP_angle!=-1)
        {
            double WP_windAngle=WP_angle-wind_angle;
            if(qAbs(WP_windAngle)>180)
            {
                if(WP_windAngle<0)
                    WP_windAngle=360+WP_windAngle;
                else
                    WP_windAngle=WP_windAngle-360;
            }
            s.sprintf("%.0f", WP_angle);
            WP_txt=tr("WP: ") + s + tr("°");
            WP_w=fm2.width(WP_txt);
            pnt.drawText(-WP_w/2,str_h,WP_txt);

            s.sprintf("%.0f", WP_windAngle);
            WP_txt=tr("WP%vent: ") + s + tr("°");
            WP_w=fm2.width(WP_txt);
            pnt.drawText(-WP_w/2,2*str_h,WP_txt);

            s.sprintf("%.1f", WP_dist);
            WP_txt=tr("WP: ") + s + tr(" nm");
            WP_w=fm2.width(WP_txt);
            pnt.drawText(-WP_w/2,3*str_h,WP_txt);

        }
    }
}

//-------------------------------------------------------------------------------
void  mapCompass::enterEvent (QEvent *)
{
    if(!terre->getHasCompassLine())
    {
        //enterCursor = cursor();
        setCursor(Qt::PointingHandCursor);
    }

}
//-------------------------------------------------------------------------------
void  mapCompass::leaveEvent (QEvent *)
{
    //unsetCursor();
}

void  mapCompass::mousePressEvent(QMouseEvent * e)
{
    if (e->button() == Qt::LeftButton)
    {
        if(terre->getHasCompassLine())
            e->ignore();
        else
        {
            isMoving=true;
            mouseEvt=false;
            mouse_x=e->globalX();
            mouse_y=e->globalY();
            setCursor(Qt::BlankCursor);
            update();
        }
    }
}

//-------------------------------------------------------------------------------
void  mapCompass::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
    {
        if(terre->getHasCompassLine())
            e->ignore();
        else
        {
            isMoving=false;
            setCursor(Qt::PointingHandCursor);
            update();
        }
    }
}

void mapCompass::slotMouseDblClicked(QMouseEvent * e)
{
    e->ignore();
}

void  mapCompass::mouseMoveEvent (QMouseEvent * e)
{
    if(isMoving)
    {
        int new_x=x()+(e->globalX()-mouse_x);
        int new_y=y()+(e->globalY()-mouse_y);

        if(new_x<=-(size/2))
            new_x=-size/2;
        if(new_y<=-(size/2))
            new_y=-size/2;
        if(new_x>(((QWidget*)(parent()))->width()-size/2))
            new_x=((QWidget*)(parent()))->width()-size/2;
        if(new_y>(((QWidget*)(parent()))->height()-size/2))
            new_y=((QWidget*)(parent()))->height()-size/2;

        move(new_x,new_y);
        mouse_x=e->globalX();
        mouse_y=e->globalY();
    }
    else
    {
        if(terre->getHasCompassLine())
            unsetCursor();        
    }
    e->ignore();
}
