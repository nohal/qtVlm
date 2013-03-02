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
    this->presetNb="1";
    this->loadPreset();

    this->modifier=0;

    int x1Fax,y1Fax,x2Fax,y2Fax;
    this->proj->map2screen(this->lon,this->lat,&x1Fax,&y1Fax);
    this->proj->map2screenByReference(lon,x1Fax,this->lon,this->lat-latRange,&x2Fax,&y2Fax);
    double newHeight=QLineF(x1Fax,y1Fax,x2Fax,y2Fax).length();
    double lonRight=this->lon-lonRange;
    if(lonRight>180) lonRight=360-lonRight;
    this->proj->map2screenByReference(lon,x1Fax,lonRight,lat,&x2Fax,&y2Fax);
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
    savePreset();
}
void faxMeteo::savePreset()
{
    Settings::setSetting("faxMeteoLat"+presetNb,lat);
    Settings::setSetting("faxMeteoLon"+presetNb,lon);
    Settings::setSetting("faxMeteoLatRange"+presetNb,latRange);
    Settings::setSetting("faxMeteoLonRange"+presetNb,lonRange);
    Settings::setSetting("faxMeteoAlpha"+presetNb,alpha);
    Settings::setSetting("faxMeteoFileName"+presetNb,imgFileName);
}
void faxMeteo::loadPreset()
{
    this->lat=Settings::getSetting("faxMeteoLat"+presetNb,0).toDouble();
    this->lon=Settings::getSetting("faxMeteoLon"+presetNb,0).toDouble();
    this->latRange=Settings::getSetting("faxMeteoLatRange"+presetNb,10).toDouble();
    this->lonRange=Settings::getSetting("faxMeteoLonRange"+presetNb,10).toDouble();
    this->alpha=Settings::getSetting("faxMeteoAlpha"+presetNb,0.7).toDouble();
    if(alpha<MIN_ALPHA) alpha=MIN_ALPHA;
    if(alpha>1) alpha=1;
    this->setImgFileName(Settings::getSetting("faxMeteoFileName"+presetNb,"").toString());
    this->setOpacity(alpha);
    int x1Fax,y1Fax,x2Fax,y2Fax;
    this->proj->map2screen(this->lon,this->lat,&x1Fax,&y1Fax);
    this->proj->map2screenByReference(lon,x1Fax,this->lon,this->lat-latRange,&x2Fax,&y2Fax);
    double newHeight=QLineF(x1Fax,y1Fax,x2Fax,y2Fax).length();
    double lonRight=this->lon-lonRange;
    if(lonRight>180) lonRight=360-lonRight;
    this->proj->map2screenByReference(lon,x1Fax,lonRight,lat,&x2Fax,&y2Fax);
    double newWidth=QLineF(x1Fax,y1Fax,x2Fax,y2Fax).length();
    prepareGeometryChange();
    this->br=QRectF(0,0,newWidth,newHeight);
    this->slot_updateProjection();
}

void faxMeteo::setImgFileName(QString imgFileName)
{
    this->imgFileName=imgFileName;
    if(!faxImg.load(this->imgFileName))
        faxImg=faxImg.scaled(0,0);

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
            alpha+=0.01*(y-mouse_y);
            if(alpha>1) alpha=1;
            if(alpha<MIN_ALPHA) alpha=MIN_ALPHA;
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
    if(faxImg.isNull() || faxImg.height()==0)
    {
        br=QRectF(0,0,0,0);
        return;
    }
    pnt->setRenderHint(QPainter::Antialiasing, true);
    pnt->setRenderHint(QPainter::SmoothPixmapTransform, true);
    int x1Fax,y1Fax,x2Fax,y2Fax;
    this->proj->map2screen(this->lon,this->lat,&x1Fax,&y1Fax);
    this->proj->map2screenByReference(lon,x1Fax,this->lon,this->lat-latRange,&x2Fax,&y2Fax);
    double newHeight=QLineF(x1Fax,y1Fax,x2Fax,y2Fax).length();
    double lonRight=this->lon-lonRange;
    if(lonRight>180) lonRight=360-lonRight;
    this->proj->map2screenByReference(lon,x1Fax,lonRight,lat,&x2Fax,&y2Fax);
    double newWidth=QLineF(x1Fax,y1Fax,x2Fax,y2Fax).length();
    QPixmap faxResized;
    qWarning()<<"faxmeteo:"<<x1Fax<<y1Fax<<x2Fax<<y2Fax<<newWidth<<newHeight<<proj->getXmax()<<proj->getW();
    if(newHeight<3000)
    {
        faxResized=faxImg.scaled(newWidth,newHeight,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
        br=QRectF(faxResized.rect());
        this->setOpacity(alpha);
    }
    else
    {
        QPen pen;
        pen.setColor(Qt::red);
        pnt->setPen(pen);
        QFont fontbig("TypeWriter", 12, QFont::Bold, false);
        fontbig.setStyleHint(QFont::TypeWriter);
        fontbig.setStretch(QFont::Condensed);
        QColor   transpcolor(255,255,255,120);
        pnt->setBrush(transpcolor);
        pnt->setFont(fontbig);
        pnt->setPen(transpcolor);
        QFontMetrics fm(fontbig);
        QString badZoom=tr("Trop de zoom pour le faxMeteo");
        QSize Fsize=fm.size(Qt::TextSingleLine,badZoom);
        pnt->drawRect(-x1Fax+3,-y1Fax+33,Fsize.width()+2,Fsize.height());
        pnt->setPen(pen);
        pnt->drawText(-x1Fax+5,-y1Fax+38+Fsize.height()/2,badZoom);
        br=QRectF(-x1Fax,-y1Fax,500,500);
        this->setOpacity(1.0);
    }
    pnt->drawPixmap(0,0,faxResized);
    QPointF center=br.center();
    br.setSize(QSize(br.width()*1.2,br.height()*1.2));
    br.moveCenter(center);
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
