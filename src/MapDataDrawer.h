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
    int interpolMode;
    DataManager * dataManager;
    MapDataDrawer * mapDataDrawer;
    ColorElement * colorElement;
    bool UV;
    Projection * proj;
    QPoint from,to;
    QPainter *pntGrib;
};
Q_DECLARE_TYPEINFO(GribThreadData,Q_PRIMITIVE_TYPE);

struct dataDrawerInfo {
    bool is2D;
    bool isOk;
    int secData_2D;
    bool UV;
    QString dataColorName;
    QRgb (MapDataDrawer::*dataColorFct) (double v, bool smooth);
    bool forcedInterpol;
    int forcedInterpolType;
};

bool drawColorMapGeneric_2D_Partial(const GribThreadData &g);

class MapDataDrawer
{
    public:
        MapDataDrawer(myCentralWidget * centralWidget);
        ~MapDataDrawer();

        // Carte 1D et 2D

        dataDrawerInfo * get_drawerInfo(int type);

        void drawColorMapGeneric_DTC(QPainter &pnt, Projection *proj,
                                                    int dataType,int levelType,int levelValue,
                                                    bool smooth);

        // Carte 2D
        void drawColorMapGeneric_2D(QPainter &pnt, Projection *proj, const bool &smooth,
                                    #ifdef DRAW_ARROW
                                                       const bool &showWindArrows, const bool &barbules,
                            #endif
                                                       const time_t &now, const time_t &t1, const time_t &t2,
                                                       GribRecord * recU1, GribRecord * recV1, GribRecord * recU2, GribRecord * recV2,
                                                       const QString &color_name, const bool &UV, int interpolation_mode=INTERPOLATION_UKN);
        void drawColorMapGeneric_2D_OLD(QPainter &pnt, const Projection *proj, const bool &smooth,
                                                       const time_t &now, const time_t &t1, const time_t &t2,
                                                       GribRecord * recU1, GribRecord * recV1, GribRecord * recU2, GribRecord * recV2,
                                                       const QString &color_name, const bool &UV, int interpolation_mode=INTERPOLATION_UKN);

        void drawColorMapGeneric_2D_DTC(QPainter &pnt, Projection *proj,
                                        int dataType_1, int dataType_2, int levelType, int levelValue,
                                                       bool smooth,QString colorData);

        // Carte 1D
        void drawColorMapGeneric_1D_DTC(QPainter &pnt, Projection *proj,
                                                       int dataType,int levelType,int levelValue,
                                                       bool smooth);

        // Affichage des fleches
        void drawArrowGeneric_DTC(QPainter &pnt, Projection *proj,QColor color,
                                                 int dataType, int levelType, int levelValue,
                                                 bool barbules);
        void drawArrowGeneric_2D(QPainter &pnt, Projection *proj,bool barbules,QColor color,
                                      const time_t &now, const time_t &t1, const time_t &t2,
                                      GribRecord * recU1, GribRecord * recV1, GribRecord * recU2, GribRecord * recV2,
                                      const bool &UV, int interpolation_mode=INTERPOLATION_UKN);
        void drawArrowGeneric_1D(QPainter &pnt, Projection *proj,QColor color,
                                      const time_t &now, const time_t &t1, const time_t &t2,
                                      GribRecord * recU1, GribRecord * recU2);
        // Carte de couleurs des precipitations
        void drawTest_multi(QPainter &pnt, Projection *proj);
        void drawTest_mono(QPainter &pnt, const Projection *proj);
        // Carte de l'ecart temperature-point de rosee
        void draw_DeltaDewpoint_Color(QPainter &pnt, const Projection *proj, bool smooth);


        void draw_wavesWnd(QPainter &pnt, Projection *proj, bool smooth, bool showArrows);
        void draw_wavesSwl(QPainter &pnt, Projection *proj, bool smooth, bool showArrows);
        void draw_wavesMax(QPainter &pnt, Projection *proj, bool smooth, bool showArrows);

        void drawWindArrow(QPainter &pnt, int i, int j, double ang, QColor color=QColor(255,255,255));
        void drawWindArrowWithBarbs(
                                QPainter &pnt, int i, int j,
                                double vkn, double ang,
                bool south,QColor color=QColor(255,255,255));
        void draw_PRESSURE_MinMax (QPainter &pnt, const Projection *proj);

        void  draw_Isobars (QPainter &pnt, const Projection *proj, int levelType, int levelValue);
        void  draw_IsobarsLabels (QPainter &pnt, const Projection *proj, int levelType, int levelValue);

        void  draw_Isotherms0 (QPainter &pnt, const Projection *proj);
        void  draw_Isotherms0Labels (QPainter &pnt, const Projection *proj);

        void draw_labelGeneric(QPainter &pnt,Projection *proj, int dataType,int levelType, int levelValue,QColor color);

        // Temperature (labels repartis sur la carte)
        void draw_TEMPERATURE_Labels(QPainter &pnt, const Projection *proj);


        static QColor getWindColorStatic(const double &v, const bool &smooth=true);
        static QColor getCurrentColorStatic(const double &v, const bool &smooth=true);

        FCT_SETGET_CST(bool,grib_monoCpu)

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

        void init_drawerInfo(void);
        dataDrawerInfo drawerInfo[DATA_MAX];


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
        QRgb   getIceCoverColor          (double v, bool smooth);


        bool grib_monoCpu;


};

#endif // MAPDATADRAWER_H
