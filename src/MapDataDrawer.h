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

#ifndef MAPDATADRAWER_H
#define MAPDATADRAWER_H

#include <QPainter>

#include "class_list.h"
#include "dataDef.h"

#define NEW_COLOR_CLASS

struct GribThreadData
{
    QPointF p;
    time_t  cD, tP, tN;
    GribRecord *recU1, *recV1, *recU2, *recV2;
    int interpolMode;
    bool smooth;
    Grib * grib;
    MapDataDrawer * mapDataDrawer;
#ifdef NEW_COLOR_CLASS
    ColorElement * colorElement;
#endif
};
Q_DECLARE_TYPEINFO(GribThreadData,Q_PRIMITIVE_TYPE);
struct GribThreadResult
{
    double tws;
    double twd;
    QRgb rgb;
};
Q_DECLARE_TYPEINFO(GribThreadResult,Q_PRIMITIVE_TYPE);

GribThreadResult interpolateThreaded(const GribThreadData &g);

class MapDataDrawer
{
    public:
        MapDataDrawer(myCentralWidget * centralWidget);
        ~MapDataDrawer();

        // Carte de couleurs du vent
        void draw_WIND_Color_old(Grib * grib,QPainter &pnt, const Projection *proj, bool smooth, bool showWindArrows, bool barbules);
        void draw_WIND_Color(Grib *grib, QPainter &pnt, const Projection *proj, bool smooth,
                               bool showWindArrows, bool barbules);
        void draw_CURRENT_Color(Grib *grib, QPainter &pnt, const Projection *proj, bool smooth,
                               bool showWindArrows, bool barbules);

        // Carte de couleurs des precipitations
        void draw_RAIN_Color(Grib *grib,QPainter &pnt, const Projection *proj, bool smooth);
        void draw_SNOW_DEPTH_Color(Grib *grib,QPainter &pnt, const Projection *proj, bool smooth);
        void draw_SNOW_CATEG_Color(Grib *grib,QPainter &pnt, const Projection *proj, bool smooth);
        void draw_CAPEsfc(Grib *grib,QPainter &pnt, const Projection *proj, bool smooth);
        void draw_FRZRAIN_CATEG_Color(Grib *grib,QPainter &pnt, const Projection *proj, bool smooth);
        // Carte de couleurs de nebulosite
        void draw_CLOUD_Color(Grib *grib,QPainter &pnt, const Projection *proj, bool smooth);
        // Carte de l'humidite relative en couleurs
        void draw_HUMID_Color(Grib *grib,QPainter &pnt, const Projection *proj, bool smooth);
        void draw_Temp_Color(Grib *grib,QPainter &pnt, const Projection *proj, bool smooth);
        void draw_TempPot_Color(Grib *grib,QPainter &pnt, const Projection *proj, bool smooth);
        void draw_Dewpoint_Color(Grib *grib,QPainter &pnt, const Projection *proj, bool smooth);
        // Carte de l'ecart temperature-point de rosee
        void draw_DeltaDewpoint_Color(Grib *grib,QPainter &pnt, const Projection *proj, bool smooth);

        void draw_PRESSURE_MinMax (Grib *grib,QPainter &pnt, const Projection *proj);

        void  draw_Isobars (Grib *grib,QPainter &pnt, const Projection *proj);
        void  draw_IsobarsLabels (Grib *grib,QPainter &pnt, const Projection *proj);

        void  draw_Isotherms0 (Grib *grib,QPainter &pnt, const Projection *proj);
        void  draw_Isotherms0Labels (Grib *grib,QPainter &pnt, const Projection *proj);

        // Temperature (labels repartis sur la carte)
        void draw_TEMPERATURE_Labels(Grib *grib,QPainter &pnt, const Projection *proj);


        static QColor getWindColorStatic(const double &v, const bool &smooth=true);
        static QColor getCurrentColorStatic(const double &v, const bool &smooth=true);

        FCT_SETGET_CST(bool,gribMonoCpu)

    private:
        myCentralWidget *centralWidget;

        QColor windColor[14];        // couleur selon la force du vent en beauforts
        QColor rainColor[17];
        int    mapColorTransp;

        int    windArrowSpace;        // distance mini entre fleches (pixels)
        int    windArrowSpaceOnGrid;  // distance mini entre fleches si affichage sur grille
        int    windBarbuleSpace;      // distance mini entre fleches (pixels)
        int    windBarbuleSpaceOnGrid;  // distance mini entre fleches

        int    windArrowSize;         // longueur des fleches
        int    windBarbuleSize;       // longueur des fleches

        bool	isCloudsColorModeWhite;

        void drawWindArrow(QPainter &pnt, int i, int j, double ang);
        void drawWindArrowWithBarbs(
                                QPainter &pnt, int i, int j,
                                double vkn, double ang,
                                bool south);

        void drawColorMapGeneric_1D (
                QPainter &pnt, const Projection *proj, bool smooth,
                time_t now,time_t tPrev,time_t tNxt,
                GribRecord * recPrev,GribRecord * recNxt,
                QRgb (MapDataDrawer::*function_getColor) (double v, bool smooth)
                );

        void  drawColorMapGeneric_Abs_Delta_Data (
                        QPainter &pnt, const Projection *proj, bool smooth,time_t now,
                        time_t tPrevTemp,time_t tNxtTemp,GribRecord * recPrevTemp,GribRecord * recNxtTemp,
                        time_t tPrevDewpoint,time_t tNxtDewpoint,GribRecord * recPrevDewpoint,GribRecord * recNxtDewpoint,
                        QRgb (MapDataDrawer::*function_getColor) (double v, bool smooth)
                );
        void draw_IsoLinesLabels(QPainter &pnt, QColor &couleur, const Projection *proj,
                                                        std::list<IsoLine *> *liste, double coef);
        void drawTransformedLine( QPainter &pnt,
                double si, double co,int di, int dj, int i,int j, int k,int l);

        void drawPetiteBarbule(QPainter &pnt, bool south,
                    double si, double co, int di, int dj, int b);
        void drawGrandeBarbule(QPainter &pnt,  bool south,
                    double si, double co, int di, int dj, int b);
        void drawTriangle(QPainter &pnt, bool south,
                    double si, double co, int di, int dj, int b);


        QRgb   getWindColor        (const double v, const bool smooth);
        QRgb   getCurrentColor     (const double v, const bool smooth);
        QRgb   getTemperatureColor (double v, bool smooth);
        QRgb   getRainColor        (double v, bool smooth);
        QRgb   getSnowDepthColor   (double v, bool smooth);
        QRgb   getCloudColor       (double v, bool smooth);
        QRgb   getCAPEColor  (double v, bool smooth);
        QRgb   getDeltaTemperaturesColor (double v, bool smooth);
        QRgb   getHumidColor       (double v, bool smooth);
        QRgb   getPressureColor    (double v, bool smooth);
        QRgb   getBinaryColor    (double v, bool smooth);





        bool gribMonoCpu;


};

#endif // MAPDATADRAWER_H
