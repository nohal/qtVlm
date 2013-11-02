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
#include <QMap>
#include <QMutex>

#include "class_list.h"
#include "dataDef.h"
#include "Grib.h"

struct GribThreadData
{
    time_t  now, t1, t2;
    GribRecord *recU1, *recV1, *recU2, *recV2;
    int interpolMode,windBarbuleSpace,windArrowSpace;
    DataManager * dataManager;
    MapDataDrawer * mapDataDrawer;
    ColorElement * colorElement;
    bool UV, showWindArrows, barbules;
    Projection * proj;
    QPoint from,to;
    QPainter *pntGrib;
};
Q_DECLARE_TYPEINFO(GribThreadData,Q_PRIMITIVE_TYPE);

bool drawColorMapGeneric_2D_Partial(const GribThreadData &g);

class MapDataDrawer
{
    public:
        MapDataDrawer(myCentralWidget * centralWidget);
        ~MapDataDrawer();

        // Carte de couleurs du vent
        void drawColorMapGeneric_2D(QPainter &pnt, Projection *proj, const bool &smooth,
                                                       const bool &showWindArrows, const bool &barbules,
                                                       const time_t &now, const time_t &t1, const time_t &t2,
                                                       GribRecord * recU1, GribRecord * recV1, GribRecord * recU2, GribRecord * recV2,
                                                       const QString &color_name, const bool &UV, int interpolation_mode=INTERPOLATION_UKN);
        void drawColorMapGeneric_2D_OLD(QPainter &pnt, const Projection *proj, const bool &smooth,
                                                       const bool &showWindArrows, const bool &barbules,
                                                       const time_t &now, const time_t &t1, const time_t &t2,
                                                       GribRecord * recU1, GribRecord * recV1, GribRecord * recU2, GribRecord * recV2,
                                                       const QString &color_name, const bool &UV, int interpolation_mode=INTERPOLATION_UKN);

        // Carte de couleurs des precipitations
        void draw_WIND_Color(QPainter &pnt, Projection *proj, bool smooth, bool showWindArrows, bool barbules);
        void draw_WIND_Color_OLD(QPainter &pnt, const Projection *proj, bool smooth,bool showWindArrows,bool barbules);
        void draw_CURRENT_Color(QPainter &pnt, Projection *proj, bool smooth, bool showWindArrows, bool barbules);
        void draw_RAIN_Color(QPainter &pnt, const Projection *proj, bool smooth);
        //void draw_SNOW_DEPTH_Color(QPainter &pnt, const Projection *proj, bool smooth);
        void draw_SNOW_CATEG_Color(QPainter &pnt, const Projection *proj, bool smooth);
        void draw_CAPEsfc(QPainter &pnt, const Projection *proj, bool smooth);
        void draw_CINsfc(QPainter &pnt, const Projection *proj, bool smooth);
        void draw_FRZRAIN_CATEG_Color(QPainter &pnt, const Projection *proj, bool smooth);
        // Carte de couleurs de nebulosite
        void draw_CLOUD_Color(QPainter &pnt, const Projection *proj, bool smooth);
        // Carte de l'humidite relative en couleurs
        void draw_HUMID_Color(QPainter &pnt, const Projection *proj, bool smooth);
        void draw_Temp_Color(QPainter &pnt, const Projection *proj, bool smooth);
        void draw_TempPot_Color(QPainter &pnt, const Projection *proj, bool smooth);
        void draw_Dewpoint_Color(QPainter &pnt, const Projection *proj, bool smooth);
        // Carte de l'ecart temperature-point de rosee
        void draw_DeltaDewpoint_Color(QPainter &pnt, const Projection *proj, bool smooth);

        void draw_wavesSigHgtComb(QPainter &pnt, const Projection *proj, bool smooth);
        void draw_wavesWnd(QPainter &pnt, Projection *proj, bool smooth, bool showArrows);
        void draw_wavesSwl(QPainter &pnt, Projection *proj, bool smooth, bool showArrows);
        void draw_wavesMax(QPainter &pnt, Projection *proj, bool smooth, bool showArrows);
        void draw_wavesWhiteCap(QPainter &pnt, Projection *proj, bool smooth);

        void drawWindArrow(QPainter &pnt, int i, int j, double ang);
        void drawWindArrowWithBarbs(
                                QPainter &pnt, int i, int j,
                                double vkn, double ang,
                                bool south);
        void draw_PRESSURE_MinMax (QPainter &pnt, const Projection *proj);

        void  draw_Isobars (QPainter &pnt, const Projection *proj);
        void  draw_IsobarsLabels (QPainter &pnt, const Projection *proj);

        void  draw_Isotherms0 (QPainter &pnt, const Projection *proj);
        void  draw_Isotherms0Labels (QPainter &pnt, const Projection *proj);

        // Temperature (labels repartis sur la carte)
        void draw_TEMPERATURE_Labels(QPainter &pnt, const Projection *proj);


        static QColor getWindColorStatic(const double &v, const bool &smooth=true);
        static QColor getCurrentColorStatic(const double &v, const bool &smooth=true);

        FCT_SETGET_CST(bool,gribMonoCpu)
        QMap<int,DataCode> * get_dataCodeMap(void) { return &dataCodeMap; }

        enum DrawGribPlainDataMode {
            drawNone=0,
            drawWind,
            drawCurrent,
            drawCloud,
            drawRain,
            drawHumid,
            drawTemp,
            drawTempPot,
            drawDewpoint,
            drawDeltaDewpoint,
            drawSnowCateg,
            drawFrzRainCateg,
            drawCAPEsfc,
            drawCINsfc,
            drawWavesSigHgtComb,
            drawWavesWnd,
            drawWavesSwl,
            drawWavesMax,
            drawWavesWhiteCap,
            drawWavesPrimDir,
            drawWavesSecDir,
            MAX_DRAWGRIB_DATAMODE
        };

        void paintImage(QImage *image, QPainter *pnt, const QPoint &point);
        static bool drawColorMapGeneric_2D_Partial(const GribThreadData &g);
private:
        QMutex mutex;
        myCentralWidget *centralWidget;
        DataManager * dataManager;

        int    mapColorTransp;

        int    windArrowSpace;        // distance mini entre fleches (pixels)
        int    windArrowSpaceOnGrid;  // distance mini entre fleches si affichage sur grille
        int    windBarbuleSpace;      // distance mini entre fleches (pixels)
        int    windBarbuleSpaceOnGrid;  // distance mini entre fleches

        int    windArrowSize;         // longueur des fleches
        int    windBarbuleSize;       // longueur des fleches

        bool	isCloudsColorModeWhite;

        void initDataCodes(void);


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
        void drawTransformedLine(QPainter &pnt,
                const double &si, const double &co, const int &di, const int &dj, const int &i, const int &j, const int &k, const int &l);

        void drawPetiteBarbule(QPainter &pnt, bool south,
                    double si, double co, int di, int dj, int b);
        void drawGrandeBarbule(QPainter &pnt,  bool south,
                    double si, double co, int di, int dj, int b);
        void drawTriangle(QPainter &pnt, bool south,
                    double si, double co, int di, int dj, int b);


        QRgb   getWindColor              (const double v, const bool smooth);
        QRgb   getCurrentColor           (const double v, const bool smooth);
        QRgb   getWavesColor             (const double v, const bool smooth);
        QRgb   getWavesWhiteCapColor     (double v, bool smooth);
        QRgb   getTemperatureColor       (double v, bool smooth);
        QRgb   getRainColor              (double v, bool smooth);
        QRgb   getSnowDepthColor         (double v, bool smooth);
        QRgb   getCloudColor             (double v, bool smooth);
        QRgb   getCAPEColor              (double v, bool smooth);
        QRgb   getCINColor               (double v, bool smooth);
        QRgb   getDeltaTemperaturesColor (double v, bool smooth);
        QRgb   getHumidColor             (double v, bool smooth);
        QRgb   getPressureColor          (double v, bool smooth);
        QRgb   getBinaryColor            (double v, bool smooth);


        QMap<int,DataCode> dataCodeMap;

        bool gribMonoCpu;


};

#endif // MAPDATADRAWER_H
