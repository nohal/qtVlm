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

#include "loadImg.h"
#include "Util.h"
#include "MainWindow.h"
#include "settings.h"
#include "mycentralwidget.h"
#include "Projection.h"
#include "Orthodromie.h"
#include <QPixmap>


loadImg::loadImg(Projection *proj, myCentralWidget *parent)
    : QGraphicsPixmapItem()
{
    this->parent = parent;
    this->proj=proj;
    this->setZValue(Z_VALUE_LOADIMG);
    connect (proj,SIGNAL(projectionUpdated()),this,SLOT(slot_updateProjection()));
    this->parent->getScene()->addItem(this);
    this->alpha=1.0;
    this->setOpacity(alpha);
    this->slot_updateProjection();
    this->hide();
}
void loadImg::setLonLat(double lon1, double lat1, double lon2, double lat2)
{
    this->lat1=lat1;
    this->lon1=lon1;
    this->lat2=lat2;
    this->lon2=lon2;
}

loadImg::~loadImg()
{
}

void loadImg::setMyImgFileName(QString s)
{
    this->myImgFileName=s;
    if(!img.load(this->myImgFileName))
    {
        img=img.scaled(0,0);
        this->myImgFileName.clear();
    }
}

/**************************/
/* Events                 */
/**************************/


/*********************/
/* data manipulation */
/*********************/

void loadImg::slot_updateProjection()
{
    if(img.width()==0) return;
    double x1,y1,x2,y2;
    proj->map2screenDouble(lon1,lat1,&x1,&y1);
    proj->map2screenDouble(lon2,lat2,&x2,&y2);
    QRectF ibr(QPointF(x1,y1),QPointF(x2,y2));
    QRectF pbr(QPointF(0,0),QSize(proj->getW(),proj->getH()));
    QRectF intersection=ibr.intersected(pbr);
    if(intersection.isEmpty() || intersection.isNull())
    {
        imgTemp=QPixmap(0,0);
        this->setPixmap(imgTemp);
        this->hide();
    }
    else
    {
        proj->screen2mapDouble(intersection.topLeft().x(),intersection.topLeft().y(),&x1,&y1);
        proj->screen2mapDouble(intersection.bottomRight().x(),intersection.bottomRight().y(),&x2,&y2);
        double i1,i2,i3,i4;
        this->map2Image(lon1,lat1,&i1,&i2);
        this->map2Image(x1,y1,&i1,&i2);
        this->map2Image(x2,y2,&i3,&i4);
        QRectF rr(QPointF(i1,i2),QPointF(i3,i4));
#if 1
        imgTemp=img.copy(rr.toRect()).scaled(intersection.size().toSize(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
#else
        imgTemp=QPixmap(intersection.size().toSize());
        QRectF r(QPoint(0,0),intersection.size());
        QPainter pnt(&imgTemp);
        pnt.setRenderHint(QPainter::Antialiasing, true);
        pnt.setRenderHint(QPainter::SmoothPixmapTransform, true);
        pnt.drawPixmap(r,img,rr);
        pnt.end();
#endif
        this->setPixmap(imgTemp);
        setPos(intersection.topLeft());
        this->show();
    }
}
void loadImg::map2Image(double lon, double lat, double *i, double *j)
{
    double x=(lon-lon1)*(img.width()/(lon2-lon1));
    lat=lat*M_PI/180.0;
    double worldMapWidth=((img.width()/(lon2-lon1))*360.0)/(2.0*M_PI);
    double mapOffsetY=(worldMapWidth/2.0 * log((1.0+sin(degToRad(lat2)))/(1.0-sin(degToRad(lat2)))));
    double y=img.height()-((worldMapWidth/2.0*log((1.0+sin(lat))/(1.0-sin(lat))))-mapOffsetY);
    *i=x;
    *j=y;
}

