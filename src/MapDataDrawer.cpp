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

    initDataCodes();
}

MapDataDrawer::~MapDataDrawer() {

}

void MapDataDrawer::initDataCodes(void) {
    dataCodeMap.insert(drawWind,DataCode(DATA_WIND_VX,DATA_LV_ABOV_GND,10));
    dataCodeMap.insert(drawCurrent,DataCode(DATA_CURRENT_VX,DATA_LV_MSL,0));
    dataCodeMap.insert(drawCloud,DataCode(DATA_CLOUD_TOT,DATA_LV_ATMOS_ALL,0));
    dataCodeMap.insert(drawRain,DataCode(DATA_PRECIP_TOT,DATA_LV_GND_SURF,0));
    dataCodeMap.insert(drawCAPEsfc,DataCode(DATA_CAPE,DATA_LV_GND_SURF,0));
    dataCodeMap.insert(drawSnowCateg,DataCode(DATA_SNOW_CATEG,DATA_LV_GND_SURF,0));
    dataCodeMap.insert(drawFrzRainCateg,DataCode(DATA_FRZRAIN_CATEG,DATA_LV_GND_SURF,0));
    dataCodeMap.insert(drawHumid,DataCode(DATA_HUMID_REL,DATA_LV_ABOV_GND,2));
    dataCodeMap.insert(drawTemp,DataCode(DATA_TEMP,DATA_LV_ABOV_GND,2));
    dataCodeMap.insert(drawTempPot,DataCode(DATA_TEMP_POT,DATA_LV_SIGMA,9950));
    //dataCodeMap.insert(drawTempMin,DataCode());
    //dataCodeMap.insert(drawTempMax,DataCode());
    dataCodeMap.insert(drawDewpoint,DataCode(DATA_DEWPOINT,DATA_LV_ABOV_GND,2));
    dataCodeMap.insert(drawDeltaDewpoint,DataCode(DATA_DEWPOINT,DATA_LV_ABOV_GND,2));
    dataCodeMap.insert(drawCINsfc,DataCode(DATA_CIN,DATA_LV_GND_SURF,0));
    dataCodeMap.insert(drawWavesSigHgtComb,DataCode(DATA_WAVES_SIG_HGT_COMB,DATA_LV_GND_SURF,0));
    dataCodeMap.insert(drawWavesWnd,DataCode(DATA_WAVES_WND_DIR,DATA_LV_GND_SURF,0));
    dataCodeMap.insert(drawWavesSwl,DataCode(DATA_WAVES_SWL_DIR,DATA_LV_GND_SURF,0));
    dataCodeMap.insert(drawWavesPrimDir,DataCode(DATA_WAVES_PRIM_DIR,DATA_LV_GND_SURF,0));
    dataCodeMap.insert(drawWavesSecDir,DataCode(DATA_WAVES_SEC_DIR,DATA_LV_GND_SURF,0));
    dataCodeMap.insert(drawWavesWhiteCap,DataCode(DATA_WAVES_WHITE_CAP,DATA_LV_GND_SURF,0));
    dataCodeMap.insert(drawWavesMax,DataCode(DATA_WAVES_MAX_DIR,DATA_LV_GND_SURF,0));
}


/****************************************************************************
 * color getter
 ***************************************************************************/

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
// Carte de couleurs generique en dimension 1
//--------------------------------------------------------------------------
void MapDataDrawer::drawColorMapGeneric_1D (
                QPainter &pnt, const Projection *proj, bool smooth,
                time_t now,time_t tPrev,time_t tNxt,
                GribRecord * recPrev,GribRecord * recNxt,
                QRgb (MapDataDrawer::*function_getColor) (double v, bool smooth)
        )
{
    if (recPrev == NULL)
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
// Carte de couleurs du vent
//--------------------------------------------------------------------------
void MapDataDrawer::drawColorMapGeneric_2D_OLD(QPainter &pnt, const Projection *proj, bool smooth,
                                               bool showWindArrows, bool barbules,
                                               time_t now, time_t t1, time_t t2,
                                               GribRecord * recU1, GribRecord * recV1, GribRecord * recU2, GribRecord * recV2,
                                               QString color_name, bool UV, int interpolation_mode)
{
    int i, j;
    double u,v,x,y;
    int W = proj->getW();
    int H = proj->getH();
    int space=0;
    int W_s=0,H_s=0;
    QRgb   rgb;

    ColorElement * colorElement=DataColors::get_colorElement(color_name);
    if(!colorElement) return;
    colorElement->loadCache(smooth);

    int sz=1;
    if(showWindArrows)
    {
        if (barbules)
            space =  windBarbuleSpace;
        else
            space =  windArrowSpace;

        W_s=W/space+1;
        H_s=H/space+1;
        sz=(W_s+2)*(H_s+2);
    }
    QVector<double> u_tab(sz,-1.0);
    QVector<double> v_tab(sz);
    QVector<bool> y_tab(sz);

    uchar * buffer=new uchar [W*H*4];
    int indice=0;
    if(interpolation_mode==INTERPOLATION_UKN)
        interpolation_mode=dataManager->get_interpolationMode();
    for (i=0; i<W-2; i+=2)
    {
        for (j=0; j<H-2; j+=2)
        {
            proj->screen2map(i,j, &x, &y);
            if(Grib::interpolateValue_2D(x,y,now,t1,t2,recU1,recV1,recU2,recV2,&u,&v,interpolation_mode,UV))
            {
                if(showWindArrows && i%space==0 && j%space==0)
                {
                    int i_s=i/space;
                    int j_s=j/space;
                    indice=i_s*H_s+j_s;
                    u_tab[indice]=u;
                    v_tab[indice]=v;
                    y_tab[indice]=(y<0);
                }

                rgb=colorElement->get_colorCached(u);

                int index=(j*W*4)+(i*4);
                buffer[index+3]=255;
                buffer[index+2]=qRed(rgb);
                buffer[index+1]=qGreen(rgb);
                buffer[index]=qBlue(rgb);
                index=(j*W*4)+((i+1)*4);
                buffer[index+3]=255;
                buffer[index+2]=qRed(rgb);
                buffer[index+1]=qGreen(rgb);
                buffer[index]=qBlue(rgb);
                index=((j+1)*W*4)+(i*4);
                buffer[index+3]=255;
                buffer[index+2]=qRed(rgb);
                buffer[index+1]=qGreen(rgb);
                buffer[index]=qBlue(rgb);
                index=((j+1)*W*4)+((i+1)*4);
                buffer[index+3]=255;
                buffer[index+2]=qRed(rgb);
                buffer[index+1]=qGreen(rgb);
                buffer[index]=qBlue(rgb);
            }
            else
            {
                int index=(j*W*4)+(i*4);
                buffer[index+3]=0;
                buffer[index+2]=0;
                buffer[index+1]=0;
                buffer[index]=0;
                index=(j*W*4)+((i+1)*4);
                buffer[index+3]=0;
                buffer[index+2]=0;
                buffer[index+1]=0;
                buffer[index]=0;
                index=((j+1)*W*4)+(i*4);
                buffer[index+3]=0;
                buffer[index+2]=0;
                buffer[index+1]=0;
                buffer[index]=0;
                index=((j+1)*W*4)+((i+1)*4);
                buffer[index+3]=0;
                buffer[index+2]=0;
                buffer[index+1]=0;
                buffer[index]=0;
            }
        }
    }

    QImage image(buffer,W,H, W*4, QImage::Format_ARGB32);
    pnt.drawImage(0,0,image);
    delete[] buffer;

    if(showWindArrows)
    {
        for (i=0; i<W_s; ++i)
        {
            for (j=0; j<H_s; ++j)
            {
                indice=i*H_s+j;
                u=u_tab.at(indice);
                v=v_tab.at(indice);

                if(u<0)
                    continue;
                if (barbules)
                    drawWindArrowWithBarbs(pnt, i*space,j*space, u,v, y_tab.at(indice));
                else
                    drawWindArrow(pnt, i*space,j*space, v);

            }
        }
    }
    colorElement->clearCache();
}
void MapDataDrawer::drawColorMapGeneric_2D(QPainter &pnt, const Projection *proj, bool smooth,
                                               bool showWindArrows, bool barbules,
                                               time_t now, time_t t1, time_t t2,
                                               GribRecord * recU1, GribRecord * recV1, GribRecord * recU2, GribRecord * recV2,
                                               QString color_name, bool UV, int interpolation_mode
                               )
{

    if(gribMonoCpu || QThread::idealThreadCount()<=1) {
        drawColorMapGeneric_2D_OLD(pnt,proj,smooth,showWindArrows,barbules,now,t1,t2,recU1,recV1,recU2,recV2,
                                   color_name,UV,interpolation_mode);
        return;
    }

    int i, j;
    double u,v,x,y;
    int W = proj->getW();
    int H = proj->getH();
    int space=0;
    int W_s=0,H_s=0;

    int sz=1;
    if(showWindArrows)
    {
        if (barbules)
            space =  windBarbuleSpace;
        else
            space =  windArrowSpace;

        W_s=W/space+1;
        H_s=H/space+1;
        sz=(W_s+2)*(H_s+2);
    }
    QVector<double> u_tab(sz,-1.0);
    QVector<double> v_tab(sz);
    QVector<bool> y_tab(sz);

    if(interpolation_mode==INTERPOLATION_UKN)
        interpolation_mode=dataManager->get_interpolationMode();

    int pass=-1;
    GribThreadData g;
    g.cD=now;
    g.recU1=recU1;
    g.recU2=recU2;
    g.recV1=recV1;
    g.recV2=recV2;
    g.tP=t1;
    g.tN=t2;
    g.interpolMode=interpolation_mode;
    g.smooth=smooth;
    g.dataManager=dataManager;
    g.mapDataDrawer=this;
    g.colorElement=DataColors::get_colorElement(color_name);
    g.UV=UV;
    if(!g.colorElement) return;
    g.colorElement->loadCache(smooth);
    QList<GribThreadData> windData;
    windData.reserve(W*H);
    for (i=0; i<W-2; i+=2)
    {
        for (j=0; j<H-2; j+=2)
        {
            proj->screen2map(i,j, &x, &y);
            g.p=QPointF(x,y);
            windData.append(g);
        }
    }

    QList<GribThreadResult> windResults  = QtConcurrent::blockingMapped(windData, interpolateThreaded);

//    QImage image(W,H,QImage::Format_ARGB32_Premultiplied);
//    image.fill(Qt::transparent);
    uchar * buffer=new uchar [W*H*4];
    int indice;
    for (i=0; i<W-2; i+=2)
    {
        for (j=0; j<H-2; j+=2)
        {
            ++pass;
            if(windResults.at(pass).tws!=-1)
            {
                if(showWindArrows && i%space==0 && j%space==0)
                {
                    int i_s=i/space;
                    int j_s=j/space;
                    indice=i_s*H_s+j_s;
                    u_tab[indice]=windResults.at(pass).tws;
                    v_tab[indice]=windResults.at(pass).twd;
                    y_tab[indice]=windData.at(pass).p.y()<0;
                }
                const QRgb rg=windResults.at(pass).rgb;
                //image.setPixel(i,  j,rg);
                int index=(j*W*4)+(i*4);
                buffer[index+3]=qAlpha(rg);
                buffer[index+2]=qRed(rg);
                buffer[index+1]=qGreen(rg);
                buffer[index]=qBlue(rg);
                //image.setPixel(i+1,j,rg);
                index=(j*W*4)+((i+1)*4);
                buffer[index+3]=qAlpha(rg);
                buffer[index+2]=qRed(rg);
                buffer[index+1]=qGreen(rg);
                buffer[index]=qBlue(rg);
                //image.setPixel(i,  j+1,rg);
                index=((j+1)*W*4)+(i*4);
                buffer[index+3]=qAlpha(rg);
                buffer[index+2]=qRed(rg);
                buffer[index+1]=qGreen(rg);
                buffer[index]=qBlue(rg);
                //image.setPixel(i+1,j+1,rg);
                index=((j+1)*W*4)+((i+1)*4);
                buffer[index+3]=qAlpha(rg);
                buffer[index+2]=qRed(rg);
                buffer[index+1]=qGreen(rg);
                buffer[index]=qBlue(rg);
            }
        }
    }
    QImage image(buffer,W,H, W*4, QImage::Format_ARGB32);
    pnt.drawImage(0,0,image);
    delete[] buffer;

    if(showWindArrows)
    {
        for (i=0; i<W_s; ++i)
        {
            for (j=0; j<H_s; ++j)
            {
                indice=i*H_s+j;
                u=u_tab.at(indice);

                if(u<0)
                    continue;
                v=v_tab.at(indice);
                if (barbules)
                    drawWindArrowWithBarbs(pnt, i*space,j*space, u,v, y_tab.at(indice));
                else
                    drawWindArrow(pnt, i*space,j*space, v);

            }
        }
    }
    g.colorElement->clearCache();
    //qWarning() << "Finished multi: " << calibration.elapsed();
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

/****************************************************************************
 * Data map drawing (calls generic fct for real drawing operations)
 ***************************************************************************/

void MapDataDrawer::draw_WIND_Color(QPainter &pnt, const Projection *proj, bool smooth,bool showWindArrows,bool barbules) {
    GribRecord *recU1,*recV1,*recU2,*recV2;
    time_t tPrev,tNxt;
    time_t currentDate=dataManager->get_currentDate();
    if(dataManager->get_data2D(DATA_WIND_VX,DATA_WIND_VY,DATA_LV_ABOV_GND,10,currentDate,
                                 &tPrev,&tNxt,&recU1,&recV1,&recU2,&recV2))
        drawColorMapGeneric_2D(pnt,proj,smooth, showWindArrows,barbules,currentDate,tPrev,tNxt,
                               recU1,recV1,recU2,recV2,"wind_kts",true);
}

void MapDataDrawer::draw_wavesSigHgtComb(QPainter &pnt, const Projection *proj, bool smooth) {
    GribRecord *rec_prev,*rec_nxt;
    time_t tPrev,tNxt;
    time_t currentDate=dataManager->get_currentDate();
    if(dataManager->get_data1D(DATA_WAVES_SIG_HGT_COMB,DATA_LV_GND_SURF,0,currentDate,
                                 &tPrev,&tNxt,&rec_prev,&rec_nxt))
        drawColorMapGeneric_1D(pnt,proj,smooth, currentDate,tPrev,tNxt,rec_prev,rec_nxt, &MapDataDrawer::getWavesColor);
}

void MapDataDrawer::draw_wavesWnd(QPainter &pnt, const Projection *proj, bool smooth,bool showArrows) {
    GribRecord *recU1,*recV1,*recU2,*recV2;
    time_t tPrev,tNxt;
    time_t currentDate=dataManager->get_currentDate();
    if(dataManager->get_data2D(DATA_WAVES_WND_HGT,DATA_WAVES_WND_DIR,DATA_LV_GND_SURF,0,currentDate,
                                 &tPrev,&tNxt,&recU1,&recV1,&recU2,&recV2))
        drawColorMapGeneric_2D(pnt,proj,smooth, showArrows,false,currentDate,
                               tPrev,tNxt,recU1,recV1,recU2,recV2, "waves_m",false,INTERPOLATION_TWSA);
}

void MapDataDrawer::draw_wavesSwl(QPainter &pnt, const Projection *proj, bool smooth,bool showArrows) {
    GribRecord *recU1,*recV1,*recU2,*recV2;
    time_t tPrev,tNxt;
    time_t currentDate=dataManager->get_currentDate();
    if(dataManager->get_data2D(DATA_WAVES_SWL_HGT,DATA_WAVES_SWL_DIR,DATA_LV_GND_SURF,0,currentDate,
                                 &tPrev,&tNxt,&recU1,&recV1,&recU2,&recV2))
        drawColorMapGeneric_2D(pnt,proj,smooth, showArrows,false,currentDate,
                               tPrev,tNxt,recU1,recV1,recU2,recV2, "waves_m",false,INTERPOLATION_TWSA);
}

void MapDataDrawer::draw_wavesMax(QPainter &pnt, const Projection *proj, bool smooth,bool showArrows) {
    GribRecord *recU1,*recV1,*recU2,*recV2;
    time_t tPrev,tNxt;
    time_t currentDate=dataManager->get_currentDate();
    if(dataManager->get_data2D(DATA_WAVES_MAX_HGT,DATA_WAVES_MAX_DIR,DATA_LV_GND_SURF,0,currentDate,
                                 &tPrev,&tNxt,&recU1,&recV1,&recU2,&recV2))
        drawColorMapGeneric_2D(pnt,proj,smooth, showArrows,false,currentDate,
                               tPrev,tNxt,recU1,recV1,recU2,recV2, "waves_m",false,INTERPOLATION_TWSA);
}

void MapDataDrawer::draw_wavesWhiteCap(QPainter &pnt, const Projection *proj, bool smooth) {
    GribRecord *rec_prev,*rec_nxt;
    time_t tPrev,tNxt;
    time_t currentDate=dataManager->get_currentDate();
    if(dataManager->get_data1D(DATA_WAVES_WHITE_CAP,DATA_LV_GND_SURF,0,currentDate,
                                 &tPrev,&tNxt,&rec_prev,&rec_nxt))
        drawColorMapGeneric_1D(pnt,proj,smooth, currentDate,tPrev,tNxt,rec_prev,rec_nxt, &MapDataDrawer::getWavesWhiteCapColor);
}

void MapDataDrawer::draw_WIND_Color_OLD(QPainter &pnt, const Projection *proj, bool smooth,bool showWindArrows,bool barbules) {
    GribRecord *recU1,*recV1,*recU2,*recV2;
    time_t tPrev,tNxt;
    time_t currentDate=dataManager->get_currentDate();
    if(dataManager->get_data2D(DATA_WIND_VX,DATA_WIND_VY,DATA_LV_ABOV_GND,10,currentDate,
                                 &tPrev,&tNxt,&recU1,&recV1,&recU2,&recV2))
        drawColorMapGeneric_2D_OLD(pnt,proj,smooth, showWindArrows,barbules,currentDate,tPrev,tNxt,
                               recU1,recV1,recU2,recV2,"wind_kts",true);
}

void MapDataDrawer::draw_CURRENT_Color(QPainter &pnt, const Projection *proj, bool smooth,bool showWindArrows,bool barbules) {
    GribRecord *recU1,*recV1,*recU2,*recV2;
    time_t tPrev,tNxt;
    time_t currentDate=dataManager->get_currentDate();
    if(dataManager->get_data2D(DATA_CURRENT_VX,DATA_CURRENT_VY,DATA_LV_MSL,0,currentDate,
                                 &tPrev,&tNxt,&recU1,&recV1,&recU2,&recV2))
        drawColorMapGeneric_2D(pnt,proj,smooth, showWindArrows,barbules,currentDate,tPrev,tNxt,
                               recU1,recV1,recU2,recV2,"current_kts",true);
}

void MapDataDrawer::draw_RAIN_Color(QPainter &pnt, const Projection *proj, bool smooth) {
    GribRecord *rec_prev,*rec_nxt;
    time_t tPrev,tNxt;
    time_t currentDate=dataManager->get_currentDate();
    if(dataManager->get_data1D(DATA_PRECIP_TOT,DATA_LV_GND_SURF,0,currentDate,
                                 &tPrev,&tNxt,&rec_prev,&rec_nxt))
        drawColorMapGeneric_1D(pnt,proj,smooth, currentDate,tPrev,tNxt,rec_prev,rec_nxt, &MapDataDrawer::getRainColor);
}

/*
void MapDataDrawer::draw_SNOW_DEPTH_Color(QPainter &pnt, const Projection *proj, bool smooth) {
    if(!dataManager || !dataManager->isOk()) return;
    GribRecord *rec_prev,*rec_nxt;
    time_t tPrev,tNxt;
    time_t currentDate=dataManager->get_currentDate();
    if(grib->getGribRecordArroundDates(DATA_SNOW_DEPTH,DATA_LV_GND_SURF,0,currentDate,
                                 &tPrev,&tNxt,&rec_prev,&rec_nxt))
        drawColorMapGeneric_1D(pnt,proj,smooth, currentDate,tPrev,tNxt,rec_prev,rec_nxt, &MapDataDrawer::getSnowDepthColor);
}
*/

void MapDataDrawer::draw_SNOW_CATEG_Color(QPainter &pnt, const Projection *proj, bool smooth) {
    if(!dataManager || !dataManager->isOk()) return;
    GribRecord *rec_prev,*rec_nxt;
    time_t tPrev,tNxt;
    time_t currentDate=dataManager->get_currentDate();
    if(dataManager->get_data1D(DATA_SNOW_CATEG,DATA_LV_GND_SURF,0,currentDate,
                                 &tPrev,&tNxt,&rec_prev,&rec_nxt))
        drawColorMapGeneric_1D(pnt,proj,smooth, currentDate,tPrev,tNxt,rec_prev,rec_nxt, &MapDataDrawer::getBinaryColor);
}

void MapDataDrawer::draw_FRZRAIN_CATEG_Color(QPainter &pnt, const Projection *proj, bool smooth) {
    if(!dataManager || !dataManager->isOk()) return;
    GribRecord *rec_prev,*rec_nxt;
    time_t tPrev,tNxt;
    time_t currentDate=dataManager->get_currentDate();
    if(dataManager->get_data1D(DATA_FRZRAIN_CATEG,DATA_LV_GND_SURF,0,currentDate,
                                 &tPrev,&tNxt,&rec_prev,&rec_nxt))
        drawColorMapGeneric_1D(pnt,proj,smooth, currentDate,tPrev,tNxt,rec_prev,rec_nxt, &MapDataDrawer::getBinaryColor);
}

void MapDataDrawer::draw_CLOUD_Color(QPainter &pnt, const Projection *proj, bool smooth) {
    if(!dataManager || !dataManager->isOk()) return;
    isCloudsColorModeWhite = Settings::getSetting("cloudsColorMode", "white").toString() == "white";

    GribRecord *rec_prev,*rec_nxt;
    time_t tPrev,tNxt;
    time_t currentDate=dataManager->get_currentDate();
    if(dataManager->get_data1D(DATA_CLOUD_TOT,DATA_LV_ATMOS_ALL,0,currentDate,
                                 &tPrev,&tNxt,&rec_prev,&rec_nxt))
        drawColorMapGeneric_1D(pnt,proj,smooth, currentDate,tPrev,tNxt,rec_prev,rec_nxt, &MapDataDrawer::getCloudColor);
}

void MapDataDrawer::draw_HUMID_Color(QPainter &pnt, const Projection *proj, bool smooth) {
    if(!dataManager || !dataManager->isOk()) return;
    GribRecord *rec_prev,*rec_nxt;
    time_t tPrev,tNxt;
    time_t currentDate=dataManager->get_currentDate();
    if(dataManager->get_data1D(DATA_HUMID_REL,DATA_LV_ABOV_GND,2,currentDate,
                                 &tPrev,&tNxt,&rec_prev,&rec_nxt))
        drawColorMapGeneric_1D(pnt,proj,smooth, currentDate,tPrev,tNxt,rec_prev,rec_nxt, &MapDataDrawer::getHumidColor);
}

void MapDataDrawer::draw_Temp_Color(QPainter &pnt, const Projection *proj, bool smooth) {
    if(!dataManager || !dataManager->isOk()) return;
    GribRecord *rec_prev,*rec_nxt;
    time_t tPrev,tNxt;
    time_t currentDate=dataManager->get_currentDate();
    if(dataManager->get_data1D(DATA_TEMP,DATA_LV_ABOV_GND,2,currentDate,
                                 &tPrev,&tNxt,&rec_prev,&rec_nxt))
        drawColorMapGeneric_1D(pnt,proj,smooth, currentDate,tPrev,tNxt,rec_prev,rec_nxt, &MapDataDrawer::getTemperatureColor);
}

void MapDataDrawer::draw_TempPot_Color(QPainter &pnt, const Projection *proj, bool smooth) {
    if(!dataManager || !dataManager->isOk()) return;
    GribRecord *rec_prev,*rec_nxt;
    time_t tPrev,tNxt;
    time_t currentDate=dataManager->get_currentDate();
    if(dataManager->get_data1D(DATA_TEMP_POT,DATA_LV_SIGMA,9950,currentDate,
                                 &tPrev,&tNxt,&rec_prev,&rec_nxt))
        drawColorMapGeneric_1D(pnt,proj,smooth, currentDate,tPrev,tNxt,rec_prev,rec_nxt, &MapDataDrawer::getTemperatureColor);
}

void MapDataDrawer::draw_Dewpoint_Color(QPainter &pnt, const Projection *proj, bool smooth) {
    if(!dataManager || !dataManager->isOk()) return;
    GribRecord *rec_prev,*rec_nxt;
    time_t tPrev,tNxt;
    time_t currentDate=dataManager->get_currentDate();
    if(dataManager->get_data1D(DATA_DEWPOINT,DATA_LV_ABOV_GND,2,currentDate,
                                 &tPrev,&tNxt,&rec_prev,&rec_nxt))
        drawColorMapGeneric_1D(pnt,proj,smooth, currentDate,tPrev,tNxt,rec_prev,rec_nxt, &MapDataDrawer::getTemperatureColor);
}

void MapDataDrawer::draw_CAPEsfc(QPainter &pnt, const Projection *proj, bool smooth) {
    if(!dataManager || !dataManager->isOk()) return;
    GribRecord *rec_prev,*rec_nxt;
    time_t tPrev,tNxt;
    time_t currentDate=dataManager->get_currentDate();
    if(dataManager->get_data1D(DATA_CAPE,DATA_LV_GND_SURF,0,currentDate,
                                 &tPrev,&tNxt,&rec_prev,&rec_nxt))
        drawColorMapGeneric_1D(pnt,proj,smooth, currentDate,tPrev,tNxt,rec_prev,rec_nxt, &MapDataDrawer::getCAPEColor);
}

void MapDataDrawer::draw_CINsfc(QPainter &pnt, const Projection *proj, bool smooth) {
    if(!dataManager || !dataManager->isOk()) return;
    GribRecord *rec_prev,*rec_nxt;
    time_t tPrev,tNxt;
    time_t currentDate=dataManager->get_currentDate();
    if(dataManager->get_data1D(DATA_CIN,DATA_LV_GND_SURF,0,currentDate,
                                 &tPrev,&tNxt,&rec_prev,&rec_nxt))
        drawColorMapGeneric_1D(pnt,proj,smooth, currentDate,tPrev,tNxt,rec_prev,rec_nxt, &MapDataDrawer::getCINColor);
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

void MapDataDrawer::draw_Isobars(QPainter &pnt, const Projection *proj) {
    if(!dataManager) return;
    Grib * grib=dataManager->get_grib(DataManager::GRIB_GRIB);
    if(!grib || !grib->isOk()) return;
    std::list<IsoLine *>::iterator it;
    std::list<IsoLine *> * listPtr=grib->get_isobars();
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

void MapDataDrawer::draw_IsobarsLabels(QPainter &pnt, const Projection *proj) {
    if(!dataManager) return;
    Grib * grib=dataManager->get_grib(DataManager::GRIB_GRIB);
    if(!grib || !grib->isOk())  return;
    QColor couleur(40,40,40);
    std::list<IsoLine *> * listPtr=grib->get_isobars();
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
        double si, double co,int di, int dj, int i,int j, int k,int l)
{
    int ii, jj, kk, ll;
    ii = (int) (i*co-j*si +0.5) + di;
    jj = (int) (i*si+j*co +0.5) + dj;
    kk = (int) (k*co-l*si +0.5) + di;
    ll = (int) (k*si+l*co +0.5) + dj;
    // Clip force a cause d'un bug qpixmap sous windows
    int w = pnt.device()->width();
    int h = pnt.device()->height();
    if (       Util::isInRange(ii, 0, w)
            && Util::isInRange(kk, 0, w)
            && Util::isInRange(jj, 0, h)
            && Util::isInRange(ll, 0, h) )
        pnt.drawLine(ii, jj, kk, ll);
}
//-----------------------------------------------------------------------------
void MapDataDrawer::drawWindArrow(QPainter &pnt, int i, int j, double ang)
{
    ang-=PI_2;
    double si=sin(ang),  co=cos(ang);
    QPen pen( QColor(255, 255, 255));
    pen.setWidth(2);
    pnt.setPen(pen);
    drawTransformedLine(pnt, si,co, i-windArrowSize/2,j,  0,0, windArrowSize, 0);   // hampe
    drawTransformedLine(pnt, si,co, i-windArrowSize/2,j,  0,0,  5, 2);   // flèche
    drawTransformedLine(pnt, si,co, i-windArrowSize/2,j,  0,0,  5,-2);   // flèche
}

//-----------------------------------------------------------------------------
void MapDataDrawer::drawWindArrowWithBarbs(
                        QPainter &pnt,
                        int i, int j, double vkn, double ang,
                        bool south)
{
    ang-=PI_2;
    double si=sin(ang),  co=cos(ang);

    QPen pen( QColor(255,255,255));
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


/************************************************************
 * multithread wind computation                             *
 ***********************************************************/

GribThreadResult interpolateThreaded(const GribThreadData &g) {
    double tws=-1, twd=-1;
    GribThreadResult r;
    if(Grib::interpolateValue_2D(g.p.x(),g.p.y(),g.cD,g.tP,g.tN,
                                                   g.recU1,g.recV1,g.recU2,g.recV2,&tws,&twd,
                                                   g.interpolMode,g.UV))
            r.rgb=g.colorElement->get_colorCached(tws);
    else
    {
        tws=-1;
        r.rgb=qRgba(0,0,0,0);
    }
    r.tws=tws;
    r.twd=twd;
    return r;
}
