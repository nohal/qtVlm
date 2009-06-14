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

#define COMPASS_MARGIN 30
#define CROSS_SIZE 8
#define MARK_SIZE 12

mapCompass::mapCompass(Projection * proj,QWidget *parentWindow) : QWidget(parentWindow)
{
    size = 310;
    isMoving=false;
    this->proj=proj;
    setGeometry(200,200,size,size);
}

void  mapCompass::paintEvent(QPaintEvent *)
{

    QPainter pnt(this);

    QFontMetrics fm(pnt.font());
    int str_w,str_h,x,y;
    float angle;
    QString str;


    QPen pen(QColor(Qt::black));
    //QBrush brush(QColor(255,255,255,50));

    str_h=fm.height();
    int diam=(size-2*COMPASS_MARGIN)/2;
    //int diam_pie=diam + 12 +2*str_h/3;

    pnt.fillRect(0,0, width(),height(), QBrush(QColor(255,255,255,00)));

    pen.setWidth(1);
    pnt.setPen(pen);
    //pnt.setBrush(brush);

    //pnt.drawPie(size/2-diam_pie,size/2-diam_pie,2*diam_pie,2*diam_pie,0,5760);


   // pnt.setPen(pen);
    //pnt.drawArc(COMPASS_MARGIN,COMPASS_MARGIN,size-2*COMPASS_MARGIN,size-2*COMPASS_MARGIN,0,5760);

    /* move center to center of circle */
    pnt.translate(size/2,size/2);

    /* center cross */

    pnt.drawLine(-(CROSS_SIZE/2),0,(CROSS_SIZE/2),0);
    pnt.drawLine(0,-(CROSS_SIZE/2),0,(CROSS_SIZE/2));

    /* numbers */


    for(int i=0;i<360;i+=10)
    {
        str=QString().setNum(i);
        str_w=fm.width(str);
        angle=degToRad(i-90);
        x=(int)((diam+12)*cos(angle)-(float)(str_w/2));
        y=(int)((diam+12)*sin(angle)+(float)(str_h/2));
        pnt.drawText(x,y,str);
    }

    /* marks */
    for(int i=0;i<360;i++)
    {
        if(i%10==0) // big marks
        {
            pen.setWidth(2);
            pnt.setPen(pen);
            pnt.drawLine(0,-diam+MARK_SIZE,0,-diam);
        }
        else if(i%5==0) // big marks
        {
            pen.setWidth(1);
            pnt.setPen(pen);
            pnt.drawLine(0,-diam+MARK_SIZE,0,-diam+6);
        }
        else if(i%1==0) // big marks
        {
            pen.setWidth(1);
            pnt.setPen(pen);
            pnt.drawLine(0,-diam+MARK_SIZE,0,-diam+10);
        }
        pnt.rotate(1);
    }

    /* text de lattitude/longitude*/
    if(isMoving)
    {
        QString lat_txt,lon_txt;
        int lat_w;
        double lat,lon;

        QFont fnt=pnt.font();
        fnt.setBold(true);
        QFontMetrics fm2(fnt);
        pnt.setFont(fnt);
        str_h=fm.height();

        proj->screen2map(this->x()+size/2,this->y()+size/2,&lon,&lat);
        lat_txt=Util::pos2String(TYPE_LAT,lat);
        lon_txt=Util::pos2String(TYPE_LON,lon);
        lat_w=fm2.width(lat_txt);
        //lon_w=fm2.width(lon_txt);
        pnt.drawText(-lat_w-10,str_h,lat_txt);
        pnt.drawText(10,str_h,lon_txt);
    }
}

void  mapCompass::mousePressEvent(QMouseEvent * e)
{
    if (e->button() == Qt::LeftButton)
    {
        isMoving=true;
        mouseEvt=false;
        mouse_x=e->globalX();
        mouse_y=e->globalY();
        update();
    }
}

//-------------------------------------------------------------------------------
void  mapCompass::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
    {
        isMoving=false;
        update();
    }

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
}


