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
    //connect (proj,SIGNAL(projectionUpdated()),this,SLOT(slot_updateProjection()));
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
void loadImg::nominalZoom()
{
    if(bsb==NULL) return;
    QPolygon bordersXY;
    for(int i=0;i<borders.count();++i)
    {
        int X,Y;
        if(i==0)
            proj->map2screen(borders.at(i).x(),borders.at(i).y(),&X,&Y);
        else
            proj->map2screenByReference(borders.at(i-1).x(),bordersXY.last().x(),borders.at(i).x(),borders.at(i).y(),&X,&Y);
        bordersXY.append(QPoint(X,Y));
    }
    QRectF br=bordersXY.boundingRect().normalized();
    double lon1,lat1,lon2,lat2,centerLon, centerLat;
    proj->screen2mapDouble(br.center().x(),br.center().y(),&centerLon,&centerLat);
    proj->screen2mapDouble(br.topLeft().x(),br.topLeft().y(),&lon1,&lat1);
    proj->screen2mapDouble(br.bottomRight().x(),br.bottomRight().y(),&lon2,&lat2);
    // compute scale;
    double sX,sY,sYN,sYS;
    sX=bsb->width/fabs(lon1-lon2);
    sYN=log(tan(degToRad(lat1)/2 + M_PI_4));
    sYS=log(tan(degToRad(lat2)/2 + M_PI_4));
    sY=bsb->height/fabs(radToDeg(sYN-sYS));

    double scale=sX>sY?sY:sX;
    proj->setScaleAndCenterInMap(scale,centerLon,centerLat);
}

int loadImg::setMyImgFileName(QString s, bool zoom)
{
    this->myImgFileName=s;
    borders.clear();
    if(bsb!=NULL)
    {
        bsb_close(bsb);
        delete bsb;
        bsb=NULL;
        delete[] bsbBuf;
    }
    bsb=new BSBImage();
    if(bsb_open_header(s.toLocal8Bit().data(), bsb)!=0)
    {
        qWarning()<<"kap projection is:"<<bsb->projection;
        if(bsb->num_wpxs==0 || bsb->num_wpys==0)
            qWarning()<<"No polynomials found in kap file, will use internal solution";
        else
            qWarning()<<"polynomials found in kap file..";
        bsbBuf=new uint8_t[bsb->width];
        for(int i=0;i<bsb->num_plys;++i)
        {
            borders.append(QPointF(bsb->ply[i].lon,bsb->ply[i].lat));
        }
        QPolygon bordersXY;
        for(int i=0;i<borders.count();++i)
        {
            int X,Y;
            if(i==0)
                proj->map2screen(borders.at(i).x(),borders.at(i).y(),&X,&Y);
            else
                proj->map2screenByReference(borders.at(i-1).x(),bordersXY.last().x(),borders.at(i).x(),borders.at(i).y(),&X,&Y);
            bordersXY.append(QPoint(X,Y));
        }
        QRectF br=bordersXY.boundingRect();
        double lo1,la1,lo2,la2;
        proj->screen2map(br.topLeft().x(),br.topLeft().y(),&lo1,&la1);
        proj->screen2map(br.bottomRight().x(),br.bottomRight().y(),&lo2,&la2);
        if(zoom)
            proj->zoomOnZone(lo1,la1,lo2,la2);
#if 0
        this->convertBsb2Pixmap(bsb); //for debugging, just see if we can decode the image. Result in myKap.png
#endif
        return 1;
    }
    else
    {
        bsb_close(bsb);
        delete bsb;
        bsb=NULL;
        return 0;
    }
}
void loadImg::convertBsb2Pixmap(BSBImage * b)
{
    QImage i(b->width,b->height,QImage::Format_Indexed8);
    i.setColorCount(b->num_colors);
    for(int col=0;col<b->num_colors;++col)
    {
        i.setColor(col,qRgb(b->red[col],b->green[col],b->blue[col]));
    }
    for(int y=0;y<i.height();++y)
    {
        uchar * ppix = i.scanLine(y);
        bsb_read_row(b,ppix);
    }
    i.save("myKap.png");
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
QPixmap loadImg::getSnapshot(QSize size)
{
    QPixmap snapshot(0,0);
    if(bsb==NULL) return snapshot;
    QPolygon bordersXY;
    for(int i=0;i<borders.count();++i)
    {
        int X,Y;
        if(i==0)
            proj->map2screen(borders.at(i).x(),borders.at(i).y(),&X,&Y);
        else
            proj->map2screenByReference(borders.at(i-1).x(),bordersXY.last().x(),borders.at(i).x(),borders.at(i).y(),&X,&Y);
        bordersXY.append(QPoint(X,Y));
    }
    QRectF br=bordersXY.boundingRect().normalized();
    QPointF centerMap=br.center();
    double centerLon,centerLat;
    proj->screen2mapDouble(centerMap.x(),centerMap.y(),&centerLon,&centerLat);
    Projection myProj(size.width(),size.height(),centerLon,centerLat);
    double lon1,lat1,lon2,lat2;
    proj->screen2mapDouble(br.topLeft().x(),br.topLeft().y(),&lon1,&lat1);
    proj->screen2mapDouble(br.bottomRight().x(),br.bottomRight().y(),&lon2,&lat2);
    myProj.zoomOnZone(lon1,lat1,lon2,lat2);
    myProj.setCenterInMap(centerLon,centerLat);
    bordersXY.clear();
    for(int i=0;i<borders.count();++i)
    {
        int X,Y;
        if(i==0)
            myProj.map2screen(borders.at(i).x(),borders.at(i).y(),&X,&Y);
        else
            myProj.map2screenByReference(borders.at(i-1).x(),bordersXY.last().x(),borders.at(i).x(),borders.at(i).y(),&X,&Y);
        bordersXY.append(QPoint(X,Y));
    }
    QRectF portion=bordersXY.boundingRect().normalized();
    if(portion.isEmpty())
    {
        return snapshot;
    }
    uint8_t * row=0;
    QSizeF portionSize=portion.size();
    bool overZoomed=false;
    QPointF topLeft;
    myProj.screen2mapDouble(portion.topLeft().toPoint(),&topLeft);
    QPointF bottomRight;
    myProj.screen2mapDouble(portion.bottomRight().toPoint(),&bottomRight);
    int minX,minY;
    bsb_LLtoXY(bsb,topLeft.x(),topLeft.y(),&minX,&minY);
    int maxX,maxY;
    bsb_LLtoXY(bsb,bottomRight.x(),bottomRight.y(),&maxX,&maxY);
    QRectF Portion(QPointF(minX,minY),QPointF(maxX,maxY));
    Portion=Portion.normalized();
    if(Portion.width()/portion.width()<2.0 || Portion.height()/portion.height()<2.0)
        overZoomed=true; //meaning there is no space to take 2 pixels so we take them all anyway, no need to call screen2Map.
    double quality=2.0;
    QSize imgSize;
    if(overZoomed)
    {
        quality=1.0;
        portion=Portion;
        imgSize=portion.size().toSize();
    }
    else
    {
        //quality=qMin(2.0,Portion.width()/portion.width());
        imgSize=QSizeF(portionSize.width()*quality,portionSize.height()*quality).toSize();
    }
    //qWarning()<<"overzoomed"<<overZoomed<<Portion.size()<<portionSize<<imgSize<<quality<<portion.topLeft()<<portion.bottomRight();
    int imgWidth=imgSize.width();
    int imgHeight=imgSize.height();
    int bitsPerPixel=3;
    uchar * buffer=new uchar [imgWidth*imgHeight*bitsPerPixel];
    int imgX=-1;
    int imgY=-1;
    for(double y=portion.topLeft().y();y<portion.bottomRight().y();y+=1.0/quality)
    {
        ++imgY;
        row=0;
        imgX=-1;
        for(double x=portion.topLeft().x();x<portion.bottomRight().x();x+=1.0/quality)
        {
            ++imgX;
            int X,Y;
            bool isOK=true;
            if(!overZoomed)
            {
                double lon,lat;
                myProj.screen2mapDouble(x,y,&lon,&lat);
                isOK=bsb_LLtoXY(bsb,lon,lat,&X,&Y);
            }
            else
            {
                X=qRound(x);
                Y=qRound(y);
            }
            if(isOK)
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
                const int r=bsb->red[row[X]];
                const int g=bsb->green[row[X]];
                const int b=bsb->blue[row[X]];
                const int index=(imgY*imgWidth*3)+(imgX*3);
                buffer[index]=r;
                buffer[index+1]=g;
                buffer[index+2]=b;
            }
        }
    }
    QImage img(buffer,imgWidth,imgHeight, imgWidth*3, QImage::Format_RGB888);
    //img.save("snapshop.png");
    img=img.scaled(portionSize.toSize(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
    snapshot=QPixmap::fromImage(img);
    delete[] buffer;
    return snapshot;
}
void loadImg::slot_updateProjection()
{
    if(bsb==NULL) return;
    QPolygon bordersXY;
    for(int i=0;i<borders.count();++i)
    {
        int X,Y;
        if(i==0)
            proj->map2screen(borders.at(i).x(),borders.at(i).y(),&X,&Y);
        else
            proj->map2screenByReference(borders.at(i-1).x(),bordersXY.last().x(),borders.at(i).x(),borders.at(i).y(),&X,&Y);
        bordersXY.append(QPoint(X,Y));
    }
    QRectF br=bordersXY.boundingRect();
    QRectF view(QPointF(0,0),QPointF(proj->getW(),proj->getH()));
    QRectF portion=view.intersected(br).normalized();
    if(portion.isEmpty())
    {
        this->setPixmap(QPixmap(0,0));
        this->hide();
        return;
    }
    uint8_t * row=0;
    QSizeF portionSize=portion.size();
    bool overZoomed=false;
    QPointF topLeft;
    proj->screen2mapDouble(portion.topLeft().toPoint(),&topLeft);
    QPointF bottomRight;
    proj->screen2mapDouble(portion.bottomRight().toPoint(),&bottomRight);
    int minX,minY;
    bsb_LLtoXY(bsb,topLeft.x(),topLeft.y(),&minX,&minY);
    int maxX,maxY;
    bsb_LLtoXY(bsb,bottomRight.x(),bottomRight.y(),&maxX,&maxY);
    QRectF Portion(QPointF(minX,minY),QPointF(maxX,maxY));
    Portion=Portion.normalized();
    if(Portion.width()/portion.width()<2.0 || Portion.height()/portion.height()<2.0)
        overZoomed=true; //meaning there is no space to take 2 pixels so we take them all anyway, no need to call screen2Map.
    double quality=2.0;
    QPointF leftCorner=portion.topLeft();
    QSize imgSize;
    if(overZoomed)
    {
        quality=1.0;
        portion=Portion;
        imgSize=portion.size().toSize();
    }
    else
    {
        //quality=qMin(2.0,Portion.width()/portion.width());
        imgSize=QSizeF(portionSize.width()*quality,portionSize.height()*quality).toSize();
    }
    //qWarning()<<"overzoomed"<<overZoomed<<Portion.size()<<portionSize<<imgSize<<quality;
    int imgWidth=imgSize.width();
    int imgHeight=imgSize.height();
    int bitsPerPixel=3;
    uchar * buffer=new uchar [imgWidth*imgHeight*bitsPerPixel];
    int imgX=-1;
    int imgY=-1;
    for(double y=portion.topLeft().y();y<portion.bottomRight().y();y+=1.0/quality)
    {
        ++imgY;
        row=0;
        imgX=-1;
        for(double x=portion.topLeft().x();x<portion.bottomRight().x();x+=1.0/quality)
        {
            ++imgX;
            int X,Y;
            bool isOK=true;
            if(!overZoomed)
            {
                double lon,lat;
                proj->screen2mapDouble(x,y,&lon,&lat);
                isOK=bsb_LLtoXY(bsb,lon,lat,&X,&Y);
            }
            else
            {
                X=qRound(x);
                Y=qRound(y);
            }
            if(isOK)
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
                const int r=bsb->red[row[X]];
                const int g=bsb->green[row[X]];
                const int b=bsb->blue[row[X]];
                const int index=(imgY*imgWidth*3)+(imgX*3);
                buffer[index]=r;
                buffer[index+1]=g;
                buffer[index+2]=b;
            }
        }
    }
    QImage img(buffer,imgWidth,imgHeight, imgWidth*3, QImage::Format_RGB888);
    img=img.scaled(portionSize.toSize(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
    this->setPixmap(QPixmap::fromImage(img));
    delete[] buffer;
    setPos(leftCorner);
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

