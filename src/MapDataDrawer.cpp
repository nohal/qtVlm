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

#ifdef QT_V5
#include <QtConcurrent/QtConcurrentMap>
#else
#include <QtConcurrentMap>
#endif

#include "Grib.h"
#include "Util.h"
#include "settings.h"
#include "Projection.h"
#include "GribRecord.h"
#include "IsoLine.h"
#include "DataColors.h"
#include "DataManager.h"
#include <QRgb>

#include "MapDataDrawer.h"
#include "mycentralwidget.h"
//#define timeStat

MapDataDrawer::MapDataDrawer(myCentralWidget *centralWidget) {
    this->centralWidget=centralWidget;
    this->dataManager=centralWidget->get_dataManager();
    gribMonoCpu=Settings::getSetting("gribMonoCpu",0).toInt()==1;

    isCloudsColorModeWhite = Settings::getSetting("cloudsColorMode", "white").toString() == "white";

    mapColorTransp = 255;

    windArrowSpace = 28;      // distance mini entre fleches
    windBarbuleSpace = 34;    // distance mini entre fleches
    windArrowSpaceOnGrid = 20;      // distance mini entre fleches
    windBarbuleSpaceOnGrid = 28;    // distance mini entre fleches
    windArrowSize = 14;       // longueur des fleches
    windBarbuleSize = 26;     // longueur des fleches avec barbules

    DataColors::load_colors(mapColorTransp);
    init_drawerInfo();

}

MapDataDrawer::~MapDataDrawer() {

}

/****************************************************************************
 * color getter
 ***************************************************************************/

#define init_1D_data(_elem,_fct) { \
    _elem.isOk=true;               \
    _elem.is2D=false;              \
    _elem.dataColorFct=_fct;       \
}

#define init_2D_data(_elem,_secData,_UV,_color) { \
    _elem.isOk=true;                              \
    _elem.is2D=true;                              \
    _elem.UV=_UV;                                 \
    _elem.secData_2D=_secData;                    \
    _elem.forcedInterpol=false;                   \
    _elem.dataColorName=_color;                   \
}

#define forceInterpol(_elem,_interpol) { \
    _elem.forcedInterpol=true;           \
    _elem.forcedInterpolType=_interpol;  \
}

void MapDataDrawer::init_drawerInfo(void) {
    for(int i=0;i<DATA_MAX;++i) drawerInfo[i].isOk=false;

    init_1D_data(drawerInfo[DATA_PRESSURE],&MapDataDrawer::getPressureColor);
    init_1D_data(drawerInfo[DATA_TEMP], &MapDataDrawer::getTemperatureColor);
    init_1D_data(drawerInfo[DATA_TEMP_POT], &MapDataDrawer::getTemperatureColor);
    init_1D_data(drawerInfo[DATA_DEWPOINT], &MapDataDrawer::getTemperatureColor);
    init_2D_data(drawerInfo[DATA_WIND_VX],DATA_WIND_VY,true,"wind_kts")
    init_2D_data(drawerInfo[DATA_CURRENT_VX],DATA_CURRENT_VY,true,"current_kts")
    init_1D_data(drawerInfo[DATA_HUMID_SPEC], &MapDataDrawer::getHumidColor);
    init_1D_data(drawerInfo[DATA_HUMID_REL], &MapDataDrawer::getHumidColor);
    init_1D_data(drawerInfo[DATA_PRECIP_RATE], &MapDataDrawer::getRainColor);
    init_1D_data(drawerInfo[DATA_PRECIP_TOT], &MapDataDrawer::getRainColor);
    init_1D_data(drawerInfo[DATA_SNOW_DEPTH], &MapDataDrawer::getSnowDepthColor);
    init_1D_data(drawerInfo[DATA_CLOUD_TOT], &MapDataDrawer::getCloudColor);
    init_1D_data(drawerInfo[DATA_FRZRAIN_CATEG], &MapDataDrawer::getBinaryColor);
    init_1D_data(drawerInfo[DATA_SNOW_CATEG], &MapDataDrawer::getBinaryColor);
    init_1D_data(drawerInfo[DATA_CIN], &MapDataDrawer::getCINColor);
    init_1D_data(drawerInfo[DATA_CAPE], &MapDataDrawer::getCAPEColor);
    init_1D_data(drawerInfo[DATA_WAVES_SIG_HGT_COMB],&MapDataDrawer::getWavesColor);
    init_1D_data(drawerInfo[DATA_WAVES_WND_HGT],&MapDataDrawer::getWavesColor);
    init_1D_data(drawerInfo[DATA_WAVES_WND_DIR],&MapDataDrawer::getWavesColor);
    init_1D_data(drawerInfo[DATA_WAVES_SWL_HGT],&MapDataDrawer::getWavesColor);
    init_1D_data(drawerInfo[DATA_WAVES_SWL_DIR],&MapDataDrawer::getWavesColor);
    init_1D_data(drawerInfo[DATA_WAVES_WHITE_CAP],&MapDataDrawer::getWavesWhiteCapColor);
    init_1D_data(drawerInfo[DATA_WAVES_MAX_HGT],&MapDataDrawer::getWavesColor);
    init_1D_data(drawerInfo[DATA_WAVES_MAX_DIR],&MapDataDrawer::getWavesColor);
    init_1D_data(drawerInfo[DATA_WAVES_PRIM_DIR],&MapDataDrawer::getWavesColor);
    init_1D_data(drawerInfo[DATA_WAVES_SEC_DIR],&MapDataDrawer::getWavesColor);
}

dataDrawerInfo * MapDataDrawer::get_drawerInfo(int type) {
    if(type>=0 && type<DATA_MAX && drawerInfo[type].isOk)
        return &drawerInfo[type];
    else
        return NULL;
}


QRgb  MapDataDrawer::getRainColor(double v, bool smooth) {
    return DataColors::get_color("rain_mmh",v,smooth);
}

QRgb  MapDataDrawer::getSnowDepthColor(double v, bool smooth) {
    return DataColors::get_color("snowdepth_m",v,smooth);
}

QRgb  MapDataDrawer::getCloudColor(double v, bool smooth) {
    QRgb rgb;
    int tr;
    if (isCloudsColorModeWhite) {
        rgb = DataColors::get_color("clouds_white_pc",v, smooth);
        tr = (int)(2.5*v);
    }
    else {
        rgb = DataColors::get_color("clouds_black_pc",v, smooth);
        tr = mapColorTransp;
    }
    return qRgba (qRed(rgb), qGreen(rgb), qBlue(rgb), tr);
}

QRgb  MapDataDrawer::getDeltaTemperaturesColor(double v, bool smooth) {
    return DataColors::get_color("deltatemp_celcius",v,smooth);
}

QRgb  MapDataDrawer::getHumidColor(double v, bool smooth) {
    return DataColors::get_color("humidrel_pc",v,smooth);
}

QRgb  MapDataDrawer::getCINColor(double v, bool smooth) {
    return DataColors::get_color("cin_jkg",v,smooth);
}

QRgb  MapDataDrawer::getCAPEColor(double v, bool smooth) {
    return DataColors::get_color("cape_jkg",v,smooth);
}

QRgb  MapDataDrawer::getTemperatureColor(double v, bool smooth) {
    return DataColors::get_color("temp_celcius",v,smooth);
}

QRgb  MapDataDrawer::getPressureColor(double v, bool smooth) {
    // Même échelle colorée que pour le vent
    double x = v/100.0;	// Pa->hPa
    double t0 = 960;  // valeur mini de l'échelle
    double t1 = 1050;  // valeur maxi de l'échelle
    return DataColors::get_color_windColorScale(x, t0, t1, smooth);
}

// using new color class
QRgb MapDataDrawer::getWindColor(double v, bool smooth) {
    return DataColors::get_color("wind_kts",v,smooth);
}

QColor MapDataDrawer::getWindColorStatic(const double &v, const bool &smooth) {
    return QColor(DataColors::get_color("wind_kts",v,smooth));
}

QRgb MapDataDrawer::getCurrentColor(double v, bool smooth) {
    return DataColors::get_color("current_kts",v,smooth);
}

QColor MapDataDrawer::getCurrentColorStatic(const double &v, const bool &smooth) {
    return QColor(DataColors::get_color("current_kts",v,smooth));
}

QRgb MapDataDrawer::getBinaryColor(double v, bool smooth) {
    return DataColors::get_color("binary",v,smooth);
}

QRgb MapDataDrawer::getWavesColor(double v, bool smooth) {
    return DataColors::get_color("waves_m",v,smooth);
}

QRgb MapDataDrawer::getWavesWhiteCapColor(double v, bool smooth) {
    return DataColors::get_color("whitecap_prb",v,smooth);
}

//--------------------------------------------------------------------------
// Carte de couleurs generique 1D et 2D
//--------------------------------------------------------------------------

void MapDataDrawer::drawColorMapGeneric_DTC(QPainter &pnt, Projection *proj,
                                            int dataType,int levelType,int levelValue,
                                            bool smooth) {
    /* check if dataType can be drawn */
    if(dataType<0 || dataType>=DATA_MAX || !drawerInfo[dataType].isOk) return;

    if(drawerInfo[dataType].is2D)
        drawColorMapGeneric_2D_DTC(pnt,proj,dataType,drawerInfo[dataType].secData_2D,levelType,levelValue,smooth,drawerInfo[dataType].dataColorName);
    else
        drawColorMapGeneric_1D_DTC(pnt,proj,dataType,levelType,levelValue,smooth);
}


//--------------------------------------------------------------------------
// Carte de couleurs generique en dimension 1
//--------------------------------------------------------------------------
void MapDataDrawer::drawColorMapGeneric_1D_DTC(QPainter &pnt, Projection *proj,
                                               int dataType,int levelType,int levelValue,
                                               bool smooth) {
    GribRecord *rec_prev,*rec_nxt;
    time_t tPrev,tNxt;
    time_t currentDate=dataManager->get_currentDate();
    if(dataManager->get_data1D(dataType,levelType,levelValue,currentDate,
                                 &tPrev,&tNxt,&rec_prev,&rec_nxt))
        drawColorMapGeneric_1D(pnt,proj,smooth, currentDate,tPrev,tNxt,rec_prev,rec_nxt, drawerInfo[dataType].dataColorFct);
}

void MapDataDrawer::drawColorMapGeneric_1D (
                QPainter &pnt, const Projection *proj, bool smooth,
                time_t now,time_t tPrev,time_t tNxt,
                GribRecord * recPrev,GribRecord * recNxt,
                QRgb (MapDataDrawer::*function_getColor) (double v, bool smooth)
        )
{
    if (recPrev == NULL)
        return;
    if(function_getColor == NULL)
        return;
    int i, j;
    double x, y, v, v_2;
    int W = proj->getW();
    int H = proj->getH();
    QRgb   rgb;
    QImage *image = new QImage(W,H,QImage::Format_ARGB32);
    image->fill( qRgba(0,0,0,0));

    //qWarning() << "1D";

    for (i=0; i<W-1; i+=2) {
        for (j=0; j<H-1; j+=2) {
            proj->screen2map(i,j, &x, &y);
            if (! recPrev->isXInMap(x))
                x += 360.0;    // tour complet ?
            if (recPrev->isPointInMap(x, y)) {
                v = recPrev->getInterpolatedValue(x, y, MUST_INTERPOLATE_VALUE);
                if(v != GRIB_NOTDEF && tPrev!=tNxt)
                {
                    v_2=recNxt->getInterpolatedValue(x, y, MUST_INTERPOLATE_VALUE);
                    if(v_2 != GRIB_NOTDEF)
                        v=v+((v_2-v)/((double)(tNxt-tPrev)))*((double)(now-tPrev));
                }
                if (v != GRIB_NOTDEF) {
                    rgb = (this->*function_getColor) (v, smooth);
                    image->setPixel(i,  j, rgb);
                    image->setPixel(i+1,j, rgb);
                    image->setPixel(i,  j+1, rgb);
                    image->setPixel(i+1,j+1, rgb);
                }
            }
        }
    }

    pnt.drawImage(0,0,*image);
    delete image;
}

//--------------------------------------------------------------------------
// Carte 2D
//--------------------------------------------------------------------------

void MapDataDrawer::drawColorMapGeneric_2D_DTC(QPainter &pnt, Projection *proj,
                                               int dataType_1,int dataType_2,int levelType,int levelValue,
                                               bool smooth,
                                               QString colorData) {
    GribRecord *recU1,*recV1,*recU2,*recV2;
    time_t tPrev,tNxt;
    time_t currentDate=dataManager->get_currentDate();
    int interpolationType=INTERPOLATION_UKN;
    if(drawerInfo[dataType_1].forcedInterpol)
        interpolationType=drawerInfo[dataType_1].forcedInterpolType;
    if(dataManager->get_data2D(dataType_1,dataType_2,levelType,levelValue,currentDate,
                                 &tPrev,&tNxt,&recU1,&recV1,&recU2,&recV2))
        drawColorMapGeneric_2D(pnt,proj,smooth,
                               currentDate,tPrev,tNxt,
                               recU1,recV1,recU2,recV2,colorData,drawerInfo[dataType_1].UV,interpolationType);
}

void MapDataDrawer::drawColorMapGeneric_2D(QPainter &pnt, Projection *proj, const bool &smooth,
                                               const time_t &now, const time_t &t1, const time_t &t2,
                                               GribRecord * recU1, GribRecord * recV1, GribRecord * recU2, GribRecord * recV2,
                                               const QString &color_name, const bool &UV, int interpolation_mode)
{
    if(gribMonoCpu || QThread::idealThreadCount()<=1) {
        drawColorMapGeneric_2D_OLD(pnt,proj,smooth,now,t1,t2,recU1,recV1,recU2,recV2,
                                   color_name,UV,interpolation_mode);
        return;
    }

    GribThreadData g;
    g.now=now;
    g.recU1=recU1;
    g.recU2=recU2;
    g.recV1=recV1;
    g.recV2=recV2;
    g.t1=t1;
    g.t2=t2;
    if(interpolation_mode==INTERPOLATION_UKN)
        interpolation_mode=dataManager->get_interpolationMode();
    g.interpolMode=interpolation_mode;
    g.dataManager=dataManager;
    g.mapDataDrawer=this;
    g.proj=proj;
    g.UV=UV;
    g.pntGrib=&pnt;
    ColorElement * colorElement=DataColors::get_colorElement(color_name);
    if(!colorElement) return;
    if(!colorElement->isCacheLoaded(smooth))
    {
        colorElement->clearCache();
        colorElement->loadCache(smooth);
    }
    g.colorElement=colorElement;
    QList<GribThreadData> data;
#if 1
    double nCpu=qMax(2,QThread::idealThreadCount());
    int w=floor((double)proj->getW()/(double)(nCpu/2.0));
    int h=floor((double)proj->getH()/2.0);

    for(int i=0;i<nCpu/2;++i)
    {
        int decalw=0;
        if(i>0)
            decalw=1;
        g.from=QPoint(i*w+decalw,0);
        g.to=QPoint((i+1)*w,h);
        data.append(g);
        g.from=QPoint(i*w+decalw,h+1);
        g.to=QPoint((i+1)*w,proj->getH());
        data.append(g);
    }
//    qWarning()<<nCpu<<data.size()<<w<<h;
//    foreach(GribThreadData g,data)
//        qWarning()<<g.from<<g.to<<QRect(g.from,g.to).size();
#else
    g.from=QPoint(0,0);
    g.to=QPoint(proj->getW()/2,proj->getH()/2);
    data.append(g);
    g.from=QPoint(proj->getW()/2+1,0);
    g.to=QPoint(proj->getW(),proj->getH()/2);
    data.append(g);
    g.from=QPoint(0,proj->getH()/2+1);
    g.to=QPoint(proj->getW()/2,proj->getH());
    data.append(g);
    g.from=QPoint(proj->getW()/2+1,proj->getH()/2+1);
    g.to=QPoint(proj->getW(),proj->getH());
    data.append(g);
#endif
#if 0
    for (int n=0;n<data.size();++n)
        result.append(drawColorMapGeneric_2D_Partial(data.at(n)));
#else
    QtConcurrent::blockingMapped(data, MapDataDrawer::drawColorMapGeneric_2D_Partial);
#endif
    return;
}

bool MapDataDrawer::drawColorMapGeneric_2D_Partial(const GribThreadData &g)
    {
    const Projection * proj=g.proj;
    const time_t now=g.now;
    const time_t t1=g.t1;
    const time_t t2=g.t2;
    GribRecord * recU1=g.recU1;
    GribRecord * recV1=g.recV1;
    GribRecord * recU2=g.recU2;
    GribRecord * recV2=g.recV2;
    const QPoint from=g.from;
    const QPoint to=g.to;
    const bool UV=g.UV;
    ColorElement * colorElement=g.colorElement;
    QRect paintZone(from,to);
    int interpolation_mode=g.interpolMode;
    int i, j;
    double u,v,x,y;
    int W = paintZone.width();
    int H = paintZone.height();
    W+=W%2;
    H+=H%2;    
    QRgb   rgb;

    uchar * buffer=new uchar [(W+2)*(H+2)*4];    
    const int W4=(W+2)*4;

    for (i=0; i<=W; i+=2)
    {
        for (j=0; j<=H; j+=2)
        {
            proj->screen2map(i+from.x(),j+from.y(), &x, &y);
            if(Grib::interpolateValue_2D(x,y,now,t1,t2,recU1,recV1,recU2,recV2,&u,&v,interpolation_mode,UV))
            {
                rgb=colorElement->get_colorCached(u);
                const int alpha=qAlpha(rgb);
                const int red=qRed(rgb);
                const int green=qGreen(rgb);
                const int blue=qBlue(rgb);
                int index=(j*W4)+(i*4);
                buffer[index+3]=alpha;
                buffer[index+2]=red;
                buffer[index+1]=green;
                buffer[index]=blue;
                index=(j*W4)+((i+1)*4);
                buffer[index+3]=alpha;
                buffer[index+2]=red;
                buffer[index+1]=green;
                buffer[index]=blue;
                index=((j+1)*W4)+(i*4);
                buffer[index+3]=alpha;
                buffer[index+2]=red;
                buffer[index+1]=green;
                buffer[index]=blue;
                index=((j+1)*W4)+((i+1)*4);
                buffer[index+3]=alpha;
                buffer[index+2]=red;
                buffer[index+1]=green;
                buffer[index]=blue;
            }
            else
            {
                int index=(j*W4)+(i*4);
                buffer[index+3]=0;
                buffer[index+2]=0;
                buffer[index+1]=0;
                buffer[index]=0;
                index=(j*W4)+((i+1)*4);
                buffer[index+3]=0;
                buffer[index+2]=0;
                buffer[index+1]=0;
                buffer[index]=0;
                index=((j+1)*W4)+(i*4);
                buffer[index+3]=0;
                buffer[index+2]=0;
                buffer[index+1]=0;
                buffer[index]=0;
                index=((j+1)*W4)+((i+1)*4);
                buffer[index+3]=0;
                buffer[index+2]=0;
                buffer[index+1]=0;
                buffer[index]=0;
            }
        }
    }
    QImage image(buffer,W+2,H+2, W4, QImage::Format_ARGB32);
    g.mapDataDrawer->paintImage(&image,g.pntGrib,from);
    delete[] buffer;
    return true;
}
void MapDataDrawer::paintImage(QImage * image, QPainter * pnt, const QPoint &point)
{
    mutex.lock();
    pnt->drawImage(point,*image);
    mutex.unlock();
}

void MapDataDrawer::drawColorMapGeneric_2D_OLD(QPainter &pnt, const Projection *proj, const bool &smooth,
                                               const time_t &now, const time_t &t1, const time_t &t2,
                                               GribRecord * recU1, GribRecord * recV1, GribRecord * recU2, GribRecord * recV2,
                                               const QString &color_name, const bool &UV, int interpolation_mode)
{
//    QTime tot;
//    tot.start();
    int i, j;
    double u,v,x,y;
    const int W = proj->getW();
    const int H = proj->getH();
    QRgb   rgb;

    ColorElement * colorElement=DataColors::get_colorElement(color_name);
    if(!colorElement) return;
    if(!colorElement->isCacheLoaded(smooth))
    {
        colorElement->clearCache();
        colorElement->loadCache(smooth);
    }

    uchar * buffer=new uchar [W*H*4];
    const int W4=W*4;
    if(interpolation_mode==INTERPOLATION_UKN)
        interpolation_mode=dataManager->get_interpolationMode();
    for (i=0; i<W-2; i+=2)
    {
        for (j=0; j<H-2; j+=2)
        {
            proj->screen2map(i,j, &x, &y);
            if(Grib::interpolateValue_2D(x,y,now,t1,t2,recU1,recV1,recU2,recV2,&u,&v,interpolation_mode,UV))
            {
                rgb=colorElement->get_colorCached(u);
                const int alpha=qAlpha(rgb);
                const int red=qRed(rgb);
                const int green=qGreen(rgb);
                const int blue=qBlue(rgb);
                int index=(j*W4)+(i*4);
                buffer[index+3]=alpha;
                buffer[index+2]=red;
                buffer[index+1]=green;
                buffer[index]=blue;
                index=(j*W4)+((i+1)*4);
                buffer[index+3]=alpha;
                buffer[index+2]=red;
                buffer[index+1]=green;
                buffer[index]=blue;
                index=((j+1)*W4)+(i*4);
                buffer[index+3]=alpha;
                buffer[index+2]=red;
                buffer[index+1]=green;
                buffer[index]=blue;
                index=((j+1)*W4)+((i+1)*4);
                buffer[index+3]=alpha;
                buffer[index+2]=red;
                buffer[index+1]=green;
                buffer[index]=blue;
            }
            else
            {
                int index=(j*W4)+(i*4);
                buffer[index+3]=0;
                buffer[index+2]=0;
                buffer[index+1]=0;
                buffer[index]=0;
                index=(j*W4)+((i+1)*4);
                buffer[index+3]=0;
                buffer[index+2]=0;
                buffer[index+1]=0;
                buffer[index]=0;
                index=((j+1)*W4)+(i*4);
                buffer[index+3]=0;
                buffer[index+2]=0;
                buffer[index+1]=0;
                buffer[index]=0;
                index=((j+1)*W4)+((i+1)*4);
                buffer[index+3]=0;
                buffer[index+2]=0;
                buffer[index+1]=0;
                buffer[index]=0;
            }
        }
    }

    QImage image(buffer,W,H, W4, QImage::Format_ARGB32);
    pnt.drawImage(0,0,image);
    delete[] buffer;
}

//--------------------------------------------------------------------------
// Carte de couleurs generique de la difference entre 2 champs
//--------------------------------------------------------------------------
void  MapDataDrawer::drawColorMapGeneric_Abs_Delta_Data (
                QPainter &pnt, const Projection *proj, bool smooth,time_t now,
                time_t tPrevTemp,time_t tNxtTemp,GribRecord * recPrevTemp,GribRecord * recNxtTemp,
                time_t tPrevDewpoint,time_t tNxtDewpoint,GribRecord * recPrevDewpoint,GribRecord * recNxtDewpoint,
                QRgb (MapDataDrawer::*function_getColor) (double v, bool smooth)
        )
{
    if (recPrevTemp == NULL || recPrevDewpoint == NULL) return;
    int i, j;
    double x, y, vx, vy, v, vx2, vy2;
    int W = proj->getW();
    int H = proj->getH();
    QRgb   rgb;
    QImage *image = new QImage(W,H,QImage::Format_ARGB32);
    image->fill( qRgba(0,0,0,0));
    for (i=0; i<W-1; i+=2) {
        for (j=0; j<H-1; j+=2)
        {
            proj->screen2map(i,j, &x, &y);

            if (! recPrevTemp->isXInMap(x))
                x += 360.0;    // tour complet ?

            if (recPrevTemp->isPointInMap(x, y))
            {
                vx = recPrevTemp->getInterpolatedValue(x, y, MUST_INTERPOLATE_VALUE);
                if(vx != GRIB_NOTDEF && tPrevTemp!=tNxtTemp)
                {
                    vx2=recNxtTemp->getInterpolatedValue(x, y, MUST_INTERPOLATE_VALUE);
                    if(vx2 != GRIB_NOTDEF)
                        vx=vx+((vx2-vx)/((double)(tNxtTemp-tPrevTemp)))*((double)(now-tPrevTemp));
                }

                vy = recPrevDewpoint->getInterpolatedValue(x, y, MUST_INTERPOLATE_VALUE);
                if(vy != GRIB_NOTDEF && tPrevDewpoint!=tNxtDewpoint)
                {
                    vy2=recNxtDewpoint->getInterpolatedValue(x, y, MUST_INTERPOLATE_VALUE);
                    if(vy2 != GRIB_NOTDEF)
                        vy=vy+((vy2-vy)/((double)(tNxtDewpoint-tPrevDewpoint)))*((double)(now-tPrevDewpoint));
                }

                if (vx != GRIB_NOTDEF && vy != GRIB_NOTDEF)
                {
                    v = fabs(vx-vy);
                    rgb = (this->*function_getColor) (v, smooth);
                    image->setPixel(i,  j, rgb);
                    image->setPixel(i+1,j, rgb);
                    image->setPixel(i,  j+1, rgb);
                    image->setPixel(i+1,j+1, rgb);
                }
            }
        }
    }
        pnt.drawImage(0,0,*image);
    delete image;
}

//--------------------------------------------------------------------------
// Dessin de fleches pour data 2D
//--------------------------------------------------------------------------

void MapDataDrawer::drawArrowGeneric_DTC(QPainter &pnt, Projection *proj,QColor color,
                                         int dataType, int levelType, int levelValue,
                                         bool barbules) {
    GribRecord *recU1,*recV1,*recU2,*recV2;
    time_t tPrev,tNxt;

    if(dataType<0 || dataType>=DATA_MAX || !drawerInfo[dataType].isOk) return;

    if(drawerInfo[dataType].is2D) {
        int interpolationType=INTERPOLATION_UKN;
        if(drawerInfo[dataType].forcedInterpol)
            interpolationType=drawerInfo[dataType].forcedInterpolType;

        time_t currentDate=dataManager->get_currentDate();
        if(dataManager->get_data2D(dataType,drawerInfo[dataType].secData_2D,levelType,levelValue,currentDate,
                                   &tPrev,&tNxt,&recU1,&recV1,&recU2,&recV2))
            drawArrowGeneric_2D(pnt,proj,barbules,color,
                             currentDate,tPrev,tNxt,
                             recU1,recV1,recU2,recV2,drawerInfo[dataType].UV,interpolationType);
    }
    else {
        GribRecord *rec_prev,*rec_nxt;
        time_t tPrev,tNxt;
        time_t currentDate=dataManager->get_currentDate();
        if(dataManager->get_data1D(dataType,levelType,levelValue,currentDate,
                                     &tPrev,&tNxt,&rec_prev,&rec_nxt))
            drawArrowGeneric_1D(pnt,proj,color,currentDate,tPrev,tNxt,rec_prev,rec_nxt);
    }
}

void MapDataDrawer::drawArrowGeneric_2D(QPainter &pnt, Projection *proj,bool barbules,QColor color,
                              const time_t &now, const time_t &t1, const time_t &t2,
                              GribRecord * recU1, GribRecord * recV1, GribRecord * recU2, GribRecord * recV2,
                              const bool &UV, int interpolation_mode) {
    int space;
    if (barbules)
        space =  windBarbuleSpace;
    else
        space =  windArrowSpace;
    const int W = proj->getW()/space+1;
    const int H = proj->getH()/space+1;
    double x,y;
    double u,v;

    if(interpolation_mode==INTERPOLATION_UKN)
        interpolation_mode=dataManager->get_interpolationMode();

    for (int i=0; i<W; ++i) {
        for (int j=0; j<H; ++j) {
            proj->screen2map(i*space,j*space, &x, &y);
            if(Grib::interpolateValue_2D(x,y,now,t1,t2,recU1,recV1,recU2,recV2,&u,&v,interpolation_mode,UV)) {
                if(u<0)
                    continue;
                if (barbules)
                    drawWindArrowWithBarbs(pnt, i*space,j*space, u,v, y<0,color);
                else
                    drawWindArrow(pnt, i*space,j*space, v,color);
            }
        }
    }
}

void MapDataDrawer::drawArrowGeneric_1D(QPainter &pnt, Projection *proj,QColor color,
                              const time_t &now, const time_t &t1, const time_t &t2,
                              GribRecord * recU1, GribRecord * recU2) {
    int space =  windArrowSpace;
    const int W = proj->getW()/space+1;
    const int H = proj->getH()/space+1;
    double x,y;
    double val;

    for (int i=0; i<W; ++i) {
        for (int j=0; j<H; ++j) {
            proj->screen2map(i*space,j*space, &x, &y);
            if(Grib::interpolateValue_1D(x,y,now,t1,t2,recU1,recU2,&val)) {
                //qWarning() << val;
                val=degToRad(val);
                drawWindArrow(pnt, i*space,j*space, val,color);
            }
        }
    }
}

/****************************************************************************
 * Data map drawing (calls generic fct for real drawing operations)
 ***************************************************************************/
void MapDataDrawer::drawTest_multi(QPainter &pnt, Projection *proj) {
    GribRecord *recU1,*recV1,*recU2,*recV2;
    time_t tPrev,tNxt;
    time_t currentDate=dataManager->get_currentDate();
    if(dataManager->get_data2D(DATA_WIND_VX,DATA_WIND_VY,DATA_LV_ABOV_GND,10,currentDate,
                                 &tPrev,&tNxt,&recU1,&recV1,&recU2,&recV2))
        drawColorMapGeneric_2D(pnt,proj,true,
                                    currentDate,tPrev,tNxt,
                               recU1,recV1,recU2,recV2,"wind_kts",true);
}

void MapDataDrawer::drawTest_mono(QPainter &pnt, const Projection *proj) {
    GribRecord *recU1,*recV1,*recU2,*recV2;
    time_t tPrev,tNxt;
    time_t currentDate=dataManager->get_currentDate();
    if(dataManager->get_data2D(DATA_WIND_VX,DATA_WIND_VY,DATA_LV_ABOV_GND,10,currentDate,
                                 &tPrev,&tNxt,&recU1,&recV1,&recU2,&recV2))
        drawColorMapGeneric_2D_OLD(pnt,proj,true,currentDate,tPrev,tNxt,
                               recU1,recV1,recU2,recV2,"wind_kts",true);
}

void MapDataDrawer::draw_DeltaDewpoint_Color(QPainter &pnt, const Projection *proj, bool smooth) {
    if(!dataManager || !dataManager->isOk()) return;

    GribRecord *rec_prevTemp,*rec_nxtTemp;
    time_t tPrevTemp,tNxtTemp;
    GribRecord *rec_prevDewpoint,*rec_nxtDewpoint;
    time_t tPrevDewpoint,tNxtDewpoint;
    time_t currentDate=dataManager->get_currentDate();
    if(dataManager->get_data1D(DATA_TEMP,DATA_LV_ABOV_GND,2,currentDate,
                                 &tPrevTemp,&tNxtTemp,&rec_prevTemp,&rec_nxtTemp)
        && dataManager->get_data1D(DATA_DEWPOINT,DATA_LV_ABOV_GND,2,currentDate,
                                     &tPrevDewpoint,&tNxtDewpoint,&rec_prevDewpoint,&rec_nxtDewpoint))
            drawColorMapGeneric_Abs_Delta_Data (pnt,proj,smooth,currentDate,
                                                tPrevTemp,tNxtTemp,rec_prevTemp,rec_nxtTemp,
                                                tPrevDewpoint,tNxtDewpoint,rec_prevDewpoint,rec_nxtDewpoint,
                                                        &MapDataDrawer::getDeltaTemperaturesColor );
}

/****************************************************************************
 * Isobar / Isotherm0 drawing
 ***************************************************************************/

void MapDataDrawer::draw_Isobars(QPainter &pnt, const Projection *proj,int levelType,int levelValue) {
    if(!dataManager) return;
    Grib * grib=dataManager->get_grib(DataManager::GRIB_GRIB);
    if(!grib || !grib->isOk()) return;
    std::list<IsoLine *>::iterator it;
    std::list<IsoLine *> * listPtr=grib->get_isobars(levelType,levelValue);
    if(!listPtr) {
        qWarning() << "non listPtr  for isoBar";
        return;
    }
    else
        qWarning() << listPtr->size() << " elem in listPtr isoBar";
    for(it=listPtr->begin(); it!=listPtr->end(); ++it)
    {
        (*it)->drawIsoLine(pnt, proj);
    }
}

void MapDataDrawer::draw_Isotherms0(QPainter &pnt, const Projection *proj) {
    if(!dataManager) return;
    Grib * grib=dataManager->get_grib(DataManager::GRIB_GRIB);
    if(!grib || !grib->isOk()) return;
    std::list<IsoLine *>::iterator it;
    std::list<IsoLine *> * listPtr=grib->get_isotherms0();
    for(it=listPtr->begin(); it!=listPtr->end(); ++it)
    {
        (*it)->drawIsoLine(pnt, proj);
    }
}

void MapDataDrawer::draw_IsoLinesLabels(QPainter &pnt, QColor &couleur, const Projection *proj,
                                                std::list<IsoLine *>*liste, double coef) {
    std::list<IsoLine *>::iterator it;
    int nbseg = 0;
    for(it=liste->begin(); it!=liste->end(); ++it)
    {
        nbseg += (*it)->getNbSegments();
    }
    int nbpix, density, first;
    nbpix = proj->getW()*proj->getH();
    if (nbpix == 0)
        return;
    double r = (double)nbseg/nbpix *1000;
    double dens = 10;
    density =  (int) (r*dens +0.5);
    if (density < 20)
        density = 20;
    first = 0;
    for(it=liste->begin(); it!=liste->end(); ++it)
    {
        first += 20;
        (*it)->drawIsoLineLabels(pnt, couleur, proj, density, first, coef);
    }
}

void MapDataDrawer::draw_Isotherms0Labels(QPainter &pnt, const Projection *proj) {
    if(!dataManager) return;
    Grib * grib=dataManager->get_grib(DataManager::GRIB_GRIB);
    if(!grib || !grib->isOk())  return;
    QColor couleur(200,80,80);
    std::list<IsoLine *> * listPtr=grib->get_isotherms0();
    draw_IsoLinesLabels(pnt, couleur, proj, listPtr, 1.0);
}

void MapDataDrawer::draw_IsobarsLabels(QPainter &pnt, const Projection *proj,int levelType,int levelValue) {
    if(!dataManager) return;
    Grib * grib=dataManager->get_grib(DataManager::GRIB_GRIB);
    if(!grib || !grib->isOk())  return;
    QColor couleur(40,40,40);
    std::list<IsoLine *> * listPtr=grib->get_isobars(levelType,levelValue);
    if(!listPtr) return;
    draw_IsoLinesLabels(pnt, couleur, proj, listPtr, 0.01);
}

/****************************************************************************
 * Min(L) / Max(H) drawing
 ***************************************************************************/


void MapDataDrawer::draw_PRESSURE_MinMax(QPainter &pnt, const Projection *proj)
{
    if(!dataManager || !dataManager->isOk()) return;
    GribRecord *rec_prev,*rec_nxt;
    time_t tPrev,tNxt;
    time_t currentDate=dataManager->get_currentDate();
    if(!dataManager->get_data1D(DATA_PRESSURE,DATA_LV_MSL,0,currentDate,
                                 &tPrev,&tNxt,&rec_prev,&rec_nxt))
        return;

    int i, j, W, H, pi,pj;
    double x, y, v,v1;
    double a,b,c,d,e,f,g,h,a1,b1,c1,d1,e1,f1,g1,h1;

    QFont fontPressureMinMax("Times", 18, QFont::Bold, true);
    QFontMetrics fmet(fontPressureMinMax);
    pnt.setPen(QColor(0,0,0));
    pnt.setFont(fontPressureMinMax);
    W = rec_prev->get_Ni();
    H = rec_prev->get_Nj();

    for (j=1; j<H-1; j++) {     // !!!! 1 to end-1
        for (i=1; i<W-1; i++) {
            v = rec_prev->getValue( i, j );

            a=rec_prev->getValue( i-1, j-1 );
            b=rec_prev->getValue( i-1, j   );
            c=rec_prev->getValue( i-1, j+1 );
            d=rec_prev->getValue( i  , j-1 );
            e=rec_prev->getValue( i  , j+1 );
            f=rec_prev->getValue( i+1, j-1 );
            g=rec_prev->getValue( i+1, j   );
            h=rec_prev->getValue( i+1, j+1 );
            if(tNxt!=tPrev)
            {
                v1 = rec_nxt->getValue( i, j );
                a1=rec_nxt->getValue( i-1, j-1 );
                b1=rec_nxt->getValue( i-1, j   );
                c1=rec_nxt->getValue( i-1, j+1 );
                d1=rec_nxt->getValue( i  , j-1 );
                e1=rec_nxt->getValue( i  , j+1 );
                f1=rec_nxt->getValue( i+1, j-1 );
                g1=rec_nxt->getValue( i+1, j   );
                h1=rec_nxt->getValue( i+1, j+1 );

                v = v + ((v1-v)/((double)(tNxt-tPrev)))*((double)(currentDate-tPrev));
                a = a + ((a1-a)/((double)(tNxt-tPrev)))*((double)(currentDate-tPrev));
                b = b + ((b1-b)/((double)(tNxt-tPrev)))*((double)(currentDate-tPrev));
                c = c + ((c1-c)/((double)(tNxt-tPrev)))*((double)(currentDate-tPrev));
                d = d + ((d1-d)/((double)(tNxt-tPrev)))*((double)(currentDate-tPrev));
                e = e + ((e1-e)/((double)(tNxt-tPrev)))*((double)(currentDate-tPrev));
                f = f + ((f1-f)/((double)(tNxt-tPrev)))*((double)(currentDate-tPrev));
                g = g + ((g1-g)/((double)(tNxt-tPrev)))*((double)(currentDate-tPrev));
                h = h + ((h1-h)/((double)(tNxt-tPrev)))*((double)(currentDate-tPrev));
            }

            if ( v < 101200
                   && v < a  // Minima local ?
                   && v < b
                   && v < c
                   && v < d
                   && v < e
                   && v < f
                   && v < g
                   && v < h
            ) {
                x = rec_prev->getX(i);
                y = rec_prev->getY(j);
                proj->map2screen(x,y, &pi, &pj);
                pnt.drawText(pi-fmet.width('L')/2, pj+fmet.ascent()/2, "L");
                proj->map2screen(x-360.0,y, &pi, &pj);
                pnt.drawText(pi-fmet.width('L')/2, pj+fmet.ascent()/2, "L");
            }
            if ( v > 101200
                   && v >= a  // Maxima local ?
                   && v >= b
                   && v >= c
                   && v >= d
                   && v >= e
                   && v >= f
                   && v >= g
                   && v >= h
            ) {
                x = rec_prev->getX(i);
                y = rec_prev->getY(j);
                proj->map2screen(x,y, &pi, &pj);
                pnt.drawText(pi-fmet.width('H')/2, pj+fmet.ascent()/2, "H");
                proj->map2screen(x-360.0,y, &pi, &pj);
                pnt.drawText(pi-fmet.width('H')/2, pj+fmet.ascent()/2, "H");
            }
        }
    }
}

/****************************************************************************
 * Label drawing
 ***************************************************************************/
void MapDataDrawer::draw_labelGeneric(QPainter &pnt,Projection *proj, int dataType,int levelType, int levelValue,QColor color) {
    if(!dataManager || !dataManager->isOk()) return;

    /* check if dataType can be drawn */
    if(dataType<0 || dataType>=DATA_MAX || !drawerInfo[dataType].isOk) return;

    GribRecord *recU1,*recV1,*recU2,*recV2;
    time_t tPrev,tNxt;
    time_t currentDate=dataManager->get_currentDate();
    double val1,val2;
    bool res;

    double x, y;
    int i, j, dimin, djmin;
    dimin = 50;
    djmin = 30;

    QFont fontLabels("Times", 9, QFont::Bold, true);
    QFontMetrics fmet(fontLabels);
    pnt.setFont(fontLabels);
    pnt.setPen(color);

    /* getting the grib records */
    if(drawerInfo[dataType].is2D)
        res=dataManager->get_data2D(dataType,drawerInfo[dataType].secData_2D,levelType,levelValue,currentDate,&tPrev,&tNxt,
                                    &recU1,&recV1,&recU2,&recV2);
    else
        res=dataManager->get_data1D(dataType,levelType,levelValue,currentDate,&tPrev,&tNxt,&recU1,&recV1);

    int interpolMode=INTERPOLATION_DEFAULT;
    if(drawerInfo[dataType].forcedInterpol)
        interpolMode=drawerInfo[dataType].forcedInterpolType;

    // get out of fct if we can't get the grib records
    if(!res) return;

    for (j=0; j<proj->getH(); j+= djmin) {
        for (i=0; i<proj->getW(); i+= dimin) {
            proj->screen2map(i,j, &x,&y);

            // get the interpolated value
            if(drawerInfo[dataType].is2D)
                res=Grib::interpolateValue_2D(x,y,currentDate,tPrev,tNxt,
                                              recU1,recV1,recU2,recV2,&val1,&val2,interpolMode,drawerInfo[dataType].UV);
            else
                res=Grib::interpolateValue_1D(x,y,currentDate,tPrev,tNxt,recU1,recU2,&val1);

            QString strLabel;
            if(res) {
               strLabel = Util::formatSimpleData(dataType,val1);
            }
            /*else
                strLabel= "U";*/

            pnt.drawText(i-fmet.width("XXX")/2, j+fmet.ascent()/2, strLabel);

        }
    }

}

/****************************************************************************
 * Temperature label drawing
 ***************************************************************************/

void MapDataDrawer::draw_TEMPERATURE_Labels(QPainter &pnt, const Projection *proj) {
    if(!dataManager || !dataManager->isOk()) return;
    GribRecord *rec_prev,*rec_nxt;
    time_t tPrev,tNxt;
    time_t currentDate=dataManager->get_currentDate();
    if(!dataManager->get_data1D(DATA_TEMP,DATA_LV_ABOV_GND,2,currentDate,
                                 &tPrev,&tNxt,&rec_prev,&rec_nxt))
        return;


    QFont fontTemperatureLabels("Times", 9, QFont::Bold, true);
    QFontMetrics fmet(fontTemperatureLabels);
    pnt.setFont(fontTemperatureLabels);
    pnt.setPen(QColor(0,0,0));

    double x, y, v,v1;
    int i, j, dimin, djmin;
    dimin = 50;
    djmin = 30;

    for (j=0; j<proj->getH(); j+= djmin) {
        for (i=0; i<proj->getW(); i+= dimin) {
            proj->screen2map(i,j, &x,&y);

            v = rec_prev->getInterpolatedValue(x, y, MUST_INTERPOLATE_VALUE);
            if (v!= GRIB_NOTDEF) {
                if(tNxt!=tPrev)
                {
                    v1 = rec_nxt->getInterpolatedValue(x, y, MUST_INTERPOLATE_VALUE);
                    if (v1!= GRIB_NOTDEF)
                        v = v + ((v1-v)/((double)(tNxt-tPrev)))*((double)(currentDate-tPrev));
                }
                QString strtemp = Util::formatTemperature_short(v);
                pnt.drawText(i-fmet.width("XXX")/2, j+fmet.ascent()/2, strtemp);
            }

        }
    }
}

//-----------------------------------------------------------------------------
void MapDataDrawer::drawTransformedLine( QPainter &pnt,
        const double &si, const double &co,const int &di, const int &dj, const int &i,const int &j, const int &k,const int &l)
{
    int ii, jj, kk, ll;
    ii = (int) (i*co-j*si +0.5) + di;
    jj = (int) (i*si+j*co +0.5) + dj;
    kk = (int) (k*co-l*si +0.5) + di;
    ll = (int) (k*si+l*co +0.5) + dj;
    pnt.drawLine(ii, jj, kk, ll);
}
//-----------------------------------------------------------------------------
void MapDataDrawer::drawWindArrow(QPainter &pnt, int i, int j, double ang, QColor color)
{
    ang-=PI_2;
    double si=sin(ang),  co=cos(ang);
    QPen pen(color);
    pen.setWidth(2);
    pnt.setPen(pen);
    drawTransformedLine(pnt, si,co, i-windArrowSize/2,j,  0,0, windArrowSize, 0);   // hampe
    drawTransformedLine(pnt, si,co, i-windArrowSize/2,j,  0,0,  5, 2);   // flèche
    drawTransformedLine(pnt, si,co, i-windArrowSize/2,j,  0,0,  5,-2);   // flèche
}

//-----------------------------------------------------------------------------
void MapDataDrawer::drawWindArrowWithBarbs(QPainter &pnt,
                        int i, int j, double vkn, double ang,
                        bool south, QColor color)
{
    ang-=PI_2;
    double si=sin(ang),  co=cos(ang);

    QPen pen(color);
    pen.setWidth(2);
    pnt.setPen(pen);
    pnt.setBrush(Qt::NoBrush);
    if (vkn < 1)
    {
        int r = 5;     // vent tres faible, dessine un cercle
        pnt.drawEllipse(i-r,j-r,2*r,2*r);
    }
    else {
        // Fleche centree sur l'origine
        int dec = -windBarbuleSize/2;
        drawTransformedLine(pnt, si,co, i,j,  dec,0,  dec+windBarbuleSize, 0);   // hampe
        drawTransformedLine(pnt, si,co, i,j,  dec,0,  dec+5, 2);    // flèche
        drawTransformedLine(pnt, si,co, i,j,  dec,0,  dec+5, -2);   // flèche

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
//---------------------------------------------------------------
void MapDataDrawer::drawPetiteBarbule(QPainter &pnt, bool south,
                    double si, double co, int di, int dj, int b)
{
    if (south)
        drawTransformedLine(pnt, si,co, di,dj,  b,0,  b+2, -5);
    else
        drawTransformedLine(pnt, si,co, di,dj,  b,0,  b+2, 5);
}
//---------------------------------------------------------------
void MapDataDrawer::drawGrandeBarbule(QPainter &pnt, bool south,
                    double si, double co, int di, int dj, int b)
{
    if (south)
        drawTransformedLine(pnt, si,co, di,dj,  b,0,  b+4,-10);
    else
        drawTransformedLine(pnt, si,co, di,dj,  b,0,  b+4,10);
}
//---------------------------------------------------------------
void MapDataDrawer::drawTriangle(QPainter &pnt, bool south,
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


