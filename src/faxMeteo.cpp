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


#include <cassert>

#include <QMessageBox>
#include <QDebug>

#include "faxMeteo.h"
#include "Util.h"
#include "MainWindow.h"
#include "settings.h"
#include "mycentralwidget.h"
#include "Projection.h"


faxMeteo::faxMeteo(Projection *proj, myCentralWidget *parent)
    : QGraphicsWidget()
{
    this->parent = parent;
    this->proj=proj;
    this->setZValue(Z_VALUE_FAXMETEO);
    this->setData(0,FAXMETEO_WTYPE);
    this->isMoving=false;

    this->lat=Settings::getSetting("faxMeteoLat",0).toDouble();
    this->lon=Settings::getSetting("faxMeteoLon",0).toDouble();
    this->latRange=Settings::getSetting("faxMeteoLatRange",10).toDouble();
    this->lonRange=Settings::getSetting("faxMeteoLonRange",10).toDouble();
    this->alpha=Settings::getSetting("faxMeteoAlpha",0.7).toDouble();
    this->setImgFileName(Settings::getSetting("faxMeteoFileName","").toString());
//    this->lat=65;
//    this->lon=-101;
//    this->latRange=52;
//    this->lonRange=111;
//    this->alpha=0.7;
    this->modifier=0;

    int x1Fax,y1Fax,x2Fax,y2Fax;
    this->proj->map2screen(this->lon,this->lat,&x1Fax,&y1Fax);
    this->proj->map2screen(this->lon,this->lat-latRange,&x2Fax,&y2Fax);
    double newHeight=QLineF(x1Fax,y1Fax,x2Fax,y2Fax).length();
    double lonRight=this->lon-lonRange;
    if(lonRight>180) lonRight=360-lonRight;
    this->proj->map2screen(lonRight,lat,&x2Fax,&y2Fax);
    double newWidth=QLineF(x1Fax,y1Fax,x2Fax,y2Fax).length();
    this->br=QRectF(0,0,newWidth,newHeight);


    connect (proj,SIGNAL(projectionUpdated()),this,SLOT(slot_updateProjection()));
    this->parent->getScene()->addItem(this);
    this->setOpacity(alpha);
    this->slot_updateProjection();
    this->show();
}

faxMeteo::~faxMeteo()
{
    Settings::setSetting("faxMeteoLat",lat);
    Settings::setSetting("faxMeteoLon",lon);
    Settings::setSetting("faxMeteoLatRange",latRange);
    Settings::setSetting("faxMeteoLonRange",lonRange);
    Settings::setSetting("faxMeteoAlpha",alpha);
    Settings::setSetting("faxMeteoFileName",imgFileName);
}
void faxMeteo::setImgFileName(QString imgFileName)
{
    this->imgFileName=imgFileName;
    faxImg=QPixmap(this->imgFileName);
}

/**************************/
/* Events                 */
/**************************/

void faxMeteo::mousePressEvent(QGraphicsSceneMouseEvent * e)
{
    if (e->button() == Qt::LeftButton && (e->modifiers()==Qt::ShiftModifier || e->modifiers()==Qt::AltModifier || e->modifiers()==Qt::ControlModifier))
    {
         mouse_x=e->scenePos().x();
         mouse_y=e->scenePos().y();
         isMoving=true;
         modifier=e->modifiers();
         if(modifier==Qt::ShiftModifier)
             setCursor(Qt::ClosedHandCursor);
         else if (modifier==Qt::AltModifier)
             setCursor(Qt::SizeVerCursor);
         else
             setCursor(Qt::SizeAllCursor);
     }
     else
         e->ignore();
}

bool faxMeteo::tryMoving(int x, int y)
{
    if(isMoving)
    {
        int new_x=this->x()+(x-mouse_x);
        int new_y=this->y()+(y-mouse_y);
        if(modifier==Qt::ShiftModifier)
        {
            prepareGeometryChange();
            setPos(new_x,new_y);
            proj->screen2map(new_x,new_y, &lon, &lat);
        }
        else if (modifier==Qt::AltModifier)
        {
            if(y>mouse_y)
                alpha=alpha+0.01;
            else
                alpha=alpha-0.01;
            if(alpha>1) alpha=1;
            if(alpha<0.1) alpha=0.1;
            this->setOpacity(alpha);
            update();
        }
        else
        {
            latRange+=0.1*(y-mouse_y);
            if(latRange<1) latRange=1;
            lonRange+=0.1*(x-mouse_x);
            if(lonRange<1) lonRange=1;
            prepareGeometryChange();
            update();
        }
        mouse_x=x;
        mouse_y=y;
        return true;
    }
    return false;
}

void faxMeteo::mouseReleaseEvent(QGraphicsSceneMouseEvent * e)
{
    if(isMoving)
    {
        isMoving=false;
        setCursor(Qt::PointingHandCursor);
        if(modifier==Qt::ShiftModifier)
        {
            int X=e->scenePos().x();
            int Y=e->scenePos().y();
            int new_x=this->x()+(X-mouse_x);
            int new_y=this->y()+(Y-mouse_y);
            proj->screen2map(new_x,new_y, &lon, &lat);
            prepareGeometryChange();
            setPos(new_x,new_y);
        }
        prepareGeometryChange();
        update();
        return;
    }
}

/*********************/
/* data manipulation */
/*********************/

void faxMeteo::slot_updateProjection()
{
    int x1Fax,y1Fax;
    this->proj->map2screen(this->lon,this->lat,&x1Fax,&y1Fax);
    prepareGeometryChange();
    this->setPos(x1Fax,y1Fax);
}

/***************************/
/* Paint & other qGraphics */
/***************************/

void faxMeteo::paint(QPainter * pnt, const QStyleOptionGraphicsItem * , QWidget * )
{
    if(faxImg.isNull())
    {
        br=QRectF(0,0,0,0);
        return;
    }
    pnt->setRenderHint(QPainter::Antialiasing, true);
    pnt->setRenderHint(QPainter::SmoothPixmapTransform, true);
    int x1Fax,y1Fax,x2Fax,y2Fax;
    this->proj->map2screen(this->lon,this->lat,&x1Fax,&y1Fax);
    this->proj->map2screen(this->lon,this->lat-latRange,&x2Fax,&y2Fax);
    double newHeight=QLineF(x1Fax,y1Fax,x2Fax,y2Fax).length();
    double lonRight=this->lon-lonRange;
    if(lonRight>180) lonRight=360-lonRight;
    this->proj->map2screen(lonRight,lat,&x2Fax,&y2Fax);
    double newWidth=QLineF(x1Fax,y1Fax,x2Fax,y2Fax).length();
    if(newHeight<faxImg.height()*20.0)
    {
        QPixmap faxResized=faxImg.scaled(newWidth,newHeight,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
        pnt->drawPixmap(0,0,faxResized);
        br=QRectF(faxResized.rect());
        QPointF center=br.center();
        br.setSize(QSize(br.width()*1.2,br.height()*1.2));
        br.moveCenter(center);
    }
}

QRectF faxMeteo::boundingRect() const
{
    return br;
}
QPainterPath faxMeteo::shape() const
{
    QPainterPath path;
    QRectF brBis=br;
    QPointF center=br.center();
    brBis.setSize(QSize(br.width()/1.2,br.height()/1.2));
    brBis.moveCenter(center);
    path.addRect(brBis.toRect());
    return path;
}
