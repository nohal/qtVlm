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

mapCompass::mapCompass(Projection * proj,MainWindow * main,myCentralWidget *parent) : QGraphicsWidget()
{

    isMoving=false;
    drawCompassLine=false;
    this->proj=proj;
    this->parent=parent;
    this->main=main;

    setPos(200,200);
    setZValue(Z_VALUE_COMPASS);
    setData(0,COMPASS_WTYPE);

    /* compassLine */
    compassLine = new orthoSegment(proj,parent->getScene(),Z_VALUE_COMPASS);
    connect(parent,SIGNAL(stopCompassLine()),this,SLOT(slot_stopCompassLine()));
    QPen penLine(QColor(Qt::white));
    penLine.setWidthF(1.6);
    compassLine->setLinePen(penLine);

    hdg_label = new QGraphicsSimpleTextItem();
    hdg_label->setZValue(Z_VALUE_COMPASS);
    hdg_label->setText("Hdg:");
    hdg_label->hide();
    QFont fnt=hdg_label->font();
    fnt.setBold(true);
    hdg_label->setFont(fnt);
    parent->getScene()->addItem(hdg_label);

    windAngle_label = new QGraphicsSimpleTextItem();
    windAngle_label->setZValue(Z_VALUE_COMPASS);
    windAngle_label->setText("Ang:");
    windAngle_label->hide();
    windAngle_label->setFont(fnt);
    parent->getScene()->addItem(windAngle_label);    
    size = 310;
}

void mapCompass::slot_paramChanged(void)
{
    if(Util::getSetting("showCompass",1).toInt()==1)
        show();
    else
       hide();
}

QRectF mapCompass::boundingRect() const
{
    return QRectF(0,0,size,size);
}

QPainterPath mapCompass::shape() const
 {
     QPainterPath path;
     path.addEllipse(boundingRect());
     return path;
 }

void  mapCompass::paint(QPainter * pnt, const QStyleOptionGraphicsItem * , QWidget * )
{
    QFontMetrics fm(pnt->font());
    int str_w,str_h,x,y;
    double wind_speed;
    double lat,lon;
    double WP_lat,WP_lon;
    float WP_angle=-1;
    double WP_dist=-1;
    float angle;
    QString str;    
    proj->screen2map(this->x()+size/2,this->y()+size/2,&lon,&lat);

    QPen pen(QColor(Qt::black));

    str_h=fm.height();
    int diam=(size-2*COMPASS_MARGIN)/2;

    pnt->fillRect(0,0, size,size, QBrush(QColor(255,255,255,00)));

    pen.setWidth(1);
    pnt->setPen(pen);

    /* move center to center of circle */
    pnt->translate(size/2,size/2);

    /* center cross */

    pnt->drawLine(-(CROSS_SIZE/2),0,(CROSS_SIZE/2),0);
    pnt->drawLine(0,-(CROSS_SIZE/2),0,(CROSS_SIZE/2));

    /* external compass : direction */

    /* external numbers */
    for(int i=0;i<360;i+=10)
    {
        str=QString().setNum(i);
        str_w=fm.width(str);
        angle=degToRad(i-90);
        x=(int)((diam+12)*cos(angle)-(float)(str_w/2));
        y=(int)((diam+12)*sin(angle)+(float)(str_h/2));
        pnt->drawText(x,y,str);
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
            pnt->setPen(pen);
            pnt->drawLine(0,-diam+MARK_SIZE,0,-diam);
        }
        else if(i%5==0) // medium marks
        {
            pen.setWidth(1);
            pnt->setPen(pen);
            pnt->drawLine(0,-diam+MARK_SIZE,0,-diam+6);
        }
        else if(i%1==0) // small marks
        {
            pen.setWidth(1);
            pnt->setPen(pen);
            pnt->drawLine(0,-diam+MARK_SIZE,0,-diam+10);
        }
        pnt->rotate(1);
    }

    /* WP info */
    main->getBoatWP(&WP_lat,&WP_lon);
    if(WP_lat!=-1 && WP_lat!=0 && WP_lon!=-1 && WP_lon!=0)
    {
        Orthodromie orth(lon,lat,WP_lon,WP_lat);
        WP_angle=qRound(orth.getAzimutDeg());
        WP_dist=orth.getDistance();
        pnt->rotate(WP_angle);
        pen.setColor(Qt::white);
        pen.setWidth(4);
        pnt->setPen(pen);
        pnt->drawLine(0,-diam+MARK_SIZE,0,-diam+MARK_SIZE+DELTA);
        pnt->rotate(-WP_angle);
    }

    /* Internal compas : WIND */

    /* internal numbers */
    /*compute start angle = wind angle*/
    wind_angle=0;
    Grib * grib = parent->getGrib();
    if(grib)
        grib->getInterpolatedValue_byDates(lon,lat,grib->getCurrentDate(),&wind_speed,&wind_angle);
    wind_angle=radToDeg(wind_angle);
    main->getBoatBvmg(&bvmg_up,&bvmg_down,wind_speed);
    pen.setColor(Qt::black);
    pnt->setPen(pen);

    for(int i=0;i<360;i+=20)
    {
        str=QString().setNum(i>180?i-360:i);
        str_w=fm.width(str);
        angle=degToRad((i+wind_angle)-90);
        x=(int)((diam-2*MARK_SIZE-4*DELTA)*cos(angle)-(float)(str_w/2));
        y=(int)((diam-2*MARK_SIZE-4*DELTA)*sin(angle)+(float)(str_h/2));
        pnt->drawText(x,y,str);
    }

    /* internal marks */
    pnt->rotate(wind_angle);
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
            pnt->setPen(pen);
            pnt->drawLine(0,0,0,-diam-30);
        }
        else if(i%10==0) // big marks
        {
            pen.setWidth(2);
            pnt->setPen(pen);
            pnt->drawLine(0,-diam+DELTA+MARK_SIZE,0,-diam+DELTA+2*MARK_SIZE);
        }
        else if(i%5==0) // medium marks
        {
            pen.setWidth(1);
            pnt->setPen(pen);
            pnt->drawLine(0,-diam+DELTA+MARK_SIZE,0,-diam+DELTA+2*MARK_SIZE-6);
        }
        else if(i%1==0) // small marks
        {
            pen.setWidth(1);
            pnt->setPen(pen);
            pnt->drawLine(0,-diam+DELTA+MARK_SIZE,0,-diam+DELTA+2*MARK_SIZE-10);
        }
        pnt->rotate(1);
    }
    pen.setColor(Qt::black);
    pnt->rotate(-wind_angle);
    pnt->setPen(pen);

    /* text de lattitude/longitude*/
    if(isMoving)
    {
        QString lat_txt,lon_txt,wind_txt,s,WP_txt;
        int lat_w,wind_w,WP_w;

        QFont fnt=pnt->font();
        fnt.setBold(true);
        QFontMetrics fm2(fnt);
        pnt->setFont(fnt);
        str_h=fm.height();

        lat_txt=Util::pos2String(TYPE_LAT,lat);
        lon_txt=Util::pos2String(TYPE_LON,lon);
        s.sprintf("%.0f", wind_angle);
        wind_txt += tr("Vent: ") + s + tr("°");
        lat_w=fm2.width(lat_txt);
        wind_w=fm2.width(wind_txt);

        pnt->drawText(-lat_w-2,-str_h/2,lat_txt);
        pnt->drawText(2,-str_h/2,lon_txt);
        pnt->drawText(-wind_w/2,-str_h/2-str_h,wind_txt);

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
            pnt->drawText(-WP_w/2,str_h,WP_txt);

            s.sprintf("%.0f", WP_windAngle);
            WP_txt=tr("WP%vent: ") + s + tr("°");
            WP_w=fm2.width(WP_txt);
            pnt->drawText(-WP_w/2,2*str_h,WP_txt);

            s.sprintf("%.1f", WP_dist);
            WP_txt=tr("WP: ") + s + tr(" nm");
            WP_w=fm2.width(WP_txt);
            pnt->drawText(-WP_w/2,3*str_h,WP_txt);

        }
    }
}

void  mapCompass::mousePressEvent(QGraphicsSceneMouseEvent * e)
{
    if (e->button() == Qt::LeftButton)
    {
        if(!drawCompassLine)
        {
            isMoving=true;
            mouseEvt=false;
            mouse_x=e->scenePos().x();
            mouse_y=e->scenePos().y();
            update();
        }
    }
    if (e->button() == Qt::MidButton)
    {
        double lat,lon;
        main->get_selectedBoatPos(&lat,&lon);
        int new_x=0;
        int new_y=0;
        Util::computePos(proj,lat, lon, &new_x, &new_y);
        if(new_x<=-(size/2))
            return;
        if(new_y<=-(size/2))
            return;

        if(new_x>(parent->width()-size/2))
            return;
        if(new_y>(parent->height()-size/2))
            return;
        setPos(new_x-size/2, new_y-size/2);
    }
}

//-------------------------------------------------------------------------------
void  mapCompass::mouseReleaseEvent(QGraphicsSceneMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
    {
        if(!drawCompassLine)
        {
            isMoving=false;
            update();
        }
    }
}

bool mapCompass::tryMoving(int x, int y)
{
    if(drawCompassLine)
    {
        // show heading and wind angle label
        updateCompassLineLabels(x, y);

        compassLine->moveSegment(x,y);
        return true;
    }

    if(isMoving)
    {
        int new_x=this->x()+(x-mouse_x);
        int new_y=this->y()+(y-mouse_y);

        if(new_x<=-(size/2))
            new_x=-size/2;
        if(new_y<=-(size/2))
            new_y=-size/2;

        if(new_x>(parent->width()-size/2))
            new_x=parent->width()-size/2;
        if(new_y>(parent->height()-size/2))
            new_y=parent->height()-size/2;

        setPos(new_x,new_y);
        mouse_x=x;
        mouse_y=y;
        return true;
    }
    else
        return false;
}

void mapCompass::updateCompassLineLabels(int x, int y)
{
    double pos_angle,pos_wind_angle;
    double xa,xb,ya,yb;
    proj->screen2map(this->x()+size/2,this->y()+size/2,&xa,&ya);
    proj->screen2map(x,y,&xb,&yb);
    Orthodromie orth(xa,ya,xb,yb);
    pos_angle=qRound(orth.getAzimutDeg());
    hdg_label->setText(QString().sprintf("Hdg: %.0f %c",pos_angle,176));
    /* attention wind_angle, bvmg_up et bvmg_down sont calcules dans paint */
    bool drawWindAngle=true;
    if(!parent->getGrib())
        drawWindAngle=false;

    if(drawWindAngle)
    {
        pos_wind_angle=pos_angle-wind_angle;
        if(qAbs(pos_wind_angle)>180)
        {
            if(pos_wind_angle<0)
                pos_wind_angle=360+pos_wind_angle;
            else
                pos_wind_angle=pos_wind_angle-360;
        }

        if(qAbs(pos_wind_angle)< bvmg_up || qAbs(pos_wind_angle)> bvmg_down)
            windAngle_label->setBrush(QColor(Qt::red));
        else
            windAngle_label->setBrush(QColor(Qt::black));

        windAngle_label->setText(QString().sprintf("Ang: %.0f %c",pos_wind_angle,176));
        windAngle_label->show();
    }
    else
        windAngle_label->hide();

    if( (pos_angle>285 && pos_angle<=360) || (pos_angle>=0 && pos_angle<75) || (pos_angle>105 && pos_angle<255)) /* horizontal */
    {
        if(drawWindAngle)
        {
             hdg_label->setPos(x-hdg_label->boundingRect().width()-10,y-hdg_label->boundingRect().height()/2);
             windAngle_label->setPos(x+10,y-windAngle_label->boundingRect().height()/2);
        }
         else
             hdg_label->setPos(x-hdg_label->boundingRect().width()/2,y-hdg_label->boundingRect().height()/2);
    }
    else /* vertical */
    {
        if(drawWindAngle)
        {
            hdg_label->setPos(x-hdg_label->boundingRect().width()/2,y-hdg_label->boundingRect().height()-10);
            windAngle_label->setPos(x-windAngle_label->boundingRect().width()/2,y+10);
        }
        else
            hdg_label->setPos(x-hdg_label->boundingRect().width()/2,y-hdg_label->boundingRect().height()-10);
    }
}

void mapCompass::slot_stopCompassLine(void)
{
    if(drawCompassLine) /* toggle compass line only if it is ON */
        slot_compassLine(0,0);
}

void mapCompass::slot_compassLine(int click_x, int click_y)
{
    drawCompassLine=!drawCompassLine;
    if(drawCompassLine)
    {
        windAngle_label->show();
        hdg_label->show();
        updateCompassLineLabels(click_x,click_y);
        compassLine->initSegment(this->x()+size/2,this->y()+size/2, click_x,click_y);
    }
    else
    {
        windAngle_label->hide();
        hdg_label->hide();
        compassLine->hideSegment();
    }
}
