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

/*************************************
Dessin des données GRIB (avec QT)
*************************************/

#ifndef GRIBPLOT_H
#define GRIBPLOT_H

#include <iostream>
#include <cmath>
#include <vector>
#include <list>
#include <set>

#include <QApplication>
#include <QPainter>

#include "GribReader.h"
#include "Projection.h"
#include "Isobar.h"
#include "Util.h"



//===============================================================
class GribPointInfo
{
    public :
        GribPointInfo(float x_, float y_, time_t  date_)
            {
                x = x_;
                y = y_;
                date = date_;
                vx = vy = pressure = temp = rain = cloud = humid = GRIB_NOTDEF;
            }

        bool hasWind()     const {return vx!=GRIB_NOTDEF && vy!=GRIB_NOTDEF;}
        bool hasPressure() const {return pressure!=GRIB_NOTDEF;}
        bool hasTemp()     const {return temp!=GRIB_NOTDEF;}
        bool hasRain()     const {return rain!=GRIB_NOTDEF;}
        bool hasCloud()    const {return cloud!=GRIB_NOTDEF;}
        bool hasHumid()    const {return humid!=GRIB_NOTDEF;}
        
        float   x, y;       // position
        time_t  date;
        float   vx, vy;     // wind
        float   pressure;
        float   temp;
        float   rain;
        float   cloud;
        float   humid;
};

//===============================================================
class GribPlot
{
    public:
        GribPlot();
        ~GribPlot();
        
        void loadGribFile(QString fileName);
        GribReader * getGribReader()     {return gribReader;}
        bool       isGribReaderOk()      {return gribReader!=NULL && gribReader->isOk();}
        
        void     setCurrentDate(time_t t);
        time_t   getCurrentDate()            {return currentDate;}
        std::set<time_t> * getListDates()    {return &listDates;}
        
        GribPointInfo  getGribPointInfo(float x, float y);

        // Carte de couleurs du vent
        void draw_WIND_Color(QPainter &pnt, const Projection *proj, bool smooth);
        // Carte de couleurs des précipitations
        void draw_RAIN_Color(QPainter &pnt, const Projection *proj, bool smooth);
        // Carte de couleurs de nébulosité
        void draw_CLOUD_Color(QPainter &pnt, const Projection *proj, bool smooth);
        // Carte de l'humidité relative en couleurs
        void draw_HUMID_Color(QPainter &pnt, const Projection *proj, bool smooth);
        
        // Flèches de direction du vent espacées régulièrement
        void draw_WIND_Arrows(
                    QPainter &pnt, const Projection *proj, bool barbules, QColor arrowsColor);
        
        void draw_PRESSURE_Isobars (QPainter &pnt, const Projection *proj);
        void draw_PRESSURE_IsobarsLabels (QPainter &pnt, const Projection *proj);
        void draw_PRESSURE_MinMax (QPainter &pnt, const Projection *proj);
        void setIsobarsStep(int step);

        // Températue (labels répartis sur la carte)
        void draw_TEMPERATURE_Labels(QPainter &pnt, const Projection *proj);
        
        // Rectangle translucide sur la zone couverte par les données
        void show_GRIB_CoverZone(QPainter &pnt, const Projection *proj);
        // Grille GRIB
        void draw_GribGrid(QPainter &pnt, const Projection *proj);
        // Dates de la prévision courante
        void draw_ForecastDates(QPainter &pnt, const Projection *proj);
                
        QRgb   getRainColor(float mm, bool smooth);
        QRgb   getCloudColor(float v, bool smooth);
        QRgb   getHumidColor(float v, bool smooth);
        QRgb   getWindColor(float v, bool smooth);
        QRgb   getTemperatureColor(float v, bool smooth);
        QRgb   getPressureColor(float v, bool smooth);
    
        void drawWindArrowWithBarbs(
        			QPainter &pnt, int i, int j,
        			float vx, float vy,
        			bool south,
        			QColor arrowColor=Qt::white);

    
    private:
        
        GribReader *gribReader;
        time_t  currentDate;
        std::set<time_t>    listDates;     // liste des dates des GribRecord
        std::list<Isobar *> listIsobars;   // liste d'isobares précalculées

        QColor windColor[14];        // couleur selon la force du vent en beauforts
        QColor rainColor[17];
        int    mapColorTransp;

        QColor windArrowColor;       // couleur des flèches du vent
        int    windArrowSpace;       // distance mini entre flèches (pixels)
        int    windArrowSize;        // longueur des flèches
        int    windBarbuleSpace;     // distance mini entre flèches (pixels)
        int    windBarbuleSize;      // longueur des flèches
        int    isobarsStep;          // Ecart entre isobares

        void initIsobars();
        
        void drawWindArrow(QPainter &pnt, int i, int j, float vx, float vy);
        
        void drawTransformedLine( QPainter &pnt,
                float si, float co,int di, int dj, int i,int j, int k,int l);
        
        void drawPetiteBarbule(QPainter &pnt, bool south,
                    float si, float co, int di, int dj, int b);
        void drawGrandeBarbule(QPainter &pnt,  bool south,
                    float si, float co, int di, int dj, int b);
        void drawTriangle(QPainter &pnt, bool south,
                    float si, float co, int di, int dj, int b);
        

};

#endif
