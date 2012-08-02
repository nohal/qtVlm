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
#include <QImage>
#include <QColor>
#include <QBitmap>


loadImg::loadImg(Projection *proj, myCentralWidget *parent)
    : QGraphicsPixmapItem()

{
    this->bsb=NULL;
    this->bsbBuf=0;
    this->parent = parent;
    gribKap=new QGraphicsPixmapItem(this);
    this->proj=proj;
    this->setZValue(Z_VALUE_LOADIMG);
    gribKap->setZValue(Z_VALUE_LOADIMG+0.1);
    //gribKap->setFlag(QGraphicsItem::ItemIgnoresParentOpacity,true);
    gribKap->setOpacity(1);
    gribKap->setPos(0,0);
    connect (proj,SIGNAL(projectionUpdated()),this,SLOT(slot_updateProjection()));
    this->parent->getScene()->addItem(this);
    this->alpha=Settings::getSetting("kapAlpha",1.0).toDouble();
    this->gribAlpha=Settings::getSetting("kapGribAlpha",1.0).toDouble();
    this->drawGribOverKap=Settings::getSetting("kapDrawGrib",1).toInt()==1;
    this->gribColored=Settings::getSetting("kapGribColored",0).toInt()==1;
    this->setOpacity(alpha);
    this->slot_updateProjection();
    this->hide();
}
void loadImg::setGribOpacity(double d)
{
    gribKap->setOpacity(d);
}

void loadImg::setParams(double alpha, double gribAlpha, bool drawGribOverKap, bool gribColored)
{
    this->alpha=alpha;
    this->gribAlpha=gribAlpha;
    this->drawGribOverKap=drawGribOverKap;
    bool b=this->gribColored;
    this->gribColored=gribColored;
    Settings::setSetting("kapAlpha",alpha);
    Settings::setSetting("kapGribAlpha",gribAlpha);
    Settings::setSetting("kapDrawGrib",drawGribOverKap?1:0);
    Settings::setSetting("kapGribColored",gribColored?1:0);
    this->setOpacity(alpha);
    this->gribKap->setOpacity(gribAlpha);
    if(drawGribOverKap)
    {
        if(b!=this->gribColored)
            this->setImgGribKap(this->imgGribKap);
        gribKap->show();
    }
    else
        gribKap->hide();
}

loadImg::~loadImg()
{
    if(bsb!=NULL)
    {
        delete[] bsbBuf;
        bsb_close(bsb);
        delete bsb;
        bsb=NULL;
    }
}
void loadImg::redraw(bool b1, bool b2)
{
    bool s1=drawGribOverKap;
    bool s2=gribColored;
    drawGribOverKap=b1;
    gribColored=b2;
    setImgGribKap(imgGribKap);
    drawGribOverKap=s1;
    gribColored=s2;
}

void loadImg::setImgGribKap(QPixmap imgGribKap)
{
    this->imgGribKap=imgGribKap;
    if(!drawGribOverKap || imgGribKap.isNull() || imgGribKap.size().isEmpty())
    {
        gribKap->setPixmap(QPixmap(0,0));
        gribKap->hide();
        return;
    }
    if(!gribColored)
    {
        QBitmap mask=imgGribKap.createMaskFromColor(Qt::white,Qt::MaskOutColor);
        imgGribKap.fill(Qt::black);
        imgGribKap.setMask(mask);
    }
    gribKap->setPixmap(imgGribKap);
    gribKap->show();
}

bool loadImg::setMyImgFileName(QString s)
{
    this->myImgFileName=s;
    delete[] bsbBuf;
    borders.clear();
    if(bsb!=NULL)
    {
        bsb_close(bsb);
        delete bsb;
        bsb=NULL;
    }
    bsb=new BSBImage();
    if(bsb_open_header(s.toLocal8Bit().data(), bsb))
    {
        bsbBuf=new uint8_t[bsb->width];
        for(int i=0;i<bsb->num_plys;++i)
        {
            borders.append(QPointF(bsb->ply[i].lon,bsb->ply[i].lat));
        }
        QPolygon bordersXY;
        for(int i=0;i<borders.count();++i)
        {
            int X,Y;
            proj->map2screen(borders.at(i).x(),borders.at(i).y(),&X,&Y);
            bordersXY.append(QPoint(X,Y));
        }
        QRectF br=bordersXY.boundingRect();
        double lo1,la1,lo2,la2;
        proj->screen2map(br.topLeft().x(),br.topLeft().y(),&lo1,&la1);
        proj->screen2map(br.bottomRight().x(),br.bottomRight().y(),&lo2,&la2);
        proj->zoomOnZone(lo1,la1,lo2,la2);
        return true;
    }
    else
    {
        bsb_close(bsb);
        delete bsb;
        bsb=NULL;
        return false;
    }
}
void loadImg::convertBsb2Pixmap(BSBImage * b)
{
    QImage i(b->width,b->height,QImage::Format_Indexed8);
    i.setNumColors(b->num_colors);
    for(int col=0;col<b->num_colors;++col)
    {
        i.setColor(col,qRgb(b->red[col],b->green[col],b->blue[col]));
    }
    for(int y=0;y<i.height();++y)
    {
        uchar * ppix = i.scanLine(y);
        bsb_read_row(b,ppix);
    }
    //img=QPixmap().fromImage(i);
}
uint8_t * loadImg::getRow(int row)
{
    uint8_t * p =0;
    if(row>=0 && row<bsb->height)
    {
        bsb_read_row_at(bsb,row,bsbBuf);
        p=bsbBuf;
    }
    return p;
}

void loadImg::slot_updateProjection()
{
    if(bsb==NULL) return;
    QPolygon bordersXY;
    for(int i=0;i<borders.count();++i)
    {
        int X,Y;
        proj->map2screen(borders.at(i).x(),borders.at(i).y(),&X,&Y);
        bordersXY.append(QPoint(X,Y));
    }
    QRectF br=bordersXY.boundingRect();
    QRectF view(QPointF(0,0),QPointF(proj->getW(),proj->getH()));
    QRectF portion=view.intersected(br).normalized();
    if(portion.isEmpty() || portion.isEmpty())
    {
        this->setPixmap(QPixmap(0,0));
        this->hide();
        return;
    }
    QPixmap img(portion.size().toSize());
    img.fill(Qt::transparent);
    QPainter pnt(&img);
    QPen pen;
    pen.setWidth(1);
    pnt.setRenderHint(QPainter::Antialiasing, true);
    pnt.setRenderHint(QPainter::SmoothPixmapTransform, true);
    uint8_t * row=0;
    int imgX=-1;
    int imgY=-1;
    for(int y=portion.topLeft().y();y<=portion.bottomRight().y();++y)
    {
        ++imgY;
        row=0;
        imgX=-1;
        for(int x=portion.topLeft().x();x<=portion.bottomRight().x();++x)
        {
            ++imgX;
            int r=0, g=0, b=0;
            double lon,lat;
            proj->screen2map(x,y,&lon,&lat);
            int X,Y;
            if(bsb_LLtoXY(bsb,lon,lat,&X,&Y))
            {
                if (Y<0||Y>=bsb->height)
                {
                    row=0;
                    break;
                }
                if(row==0)
                    row=getRow(Y);
                if(X<0)
                    continue;
                if(X>bsb->width)
                    break;
                if(row==0) continue;
                r=bsb->red[row[X]];
                g=bsb->green[row[X]];
                b=bsb->blue[row[X]];
                QColor color(r,g,b);
                pen.setColor(color);
                pnt.setPen(pen);
                pnt.drawPoint(imgX,imgY);
            }
        }
    }
    pnt.end();
    this->setPixmap(img);
    setPos(portion.topLeft());
    this->show();
}
#if 0
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
#endif

