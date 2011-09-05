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

/*************************
Lecture mise en mémoire d'un fichier GRIB

*************************/

#ifndef GRIB_H
#define GRIB_H

#include <iostream>
#include <cmath>
#include <vector>
#include <set>
#include <map>

#include <QPainter>
#include <QApplication>

#include "class_list.h"

#include "dataDef.h"
#include "zuFile.h"


//===============================================================
class Grib
{
    public:
        Grib();
        Grib(const Grib &model);
        ~Grib();
        
        void  loadGribFile(QString fileName);
        bool  isOk()                 {return ok;}
        long  getFileSize()          {return fileSize;}
        std::string getFileName()    {return fname;}


        int          getNumberOfGribRecords(int dataType,int levelType,int levelValue);
        int          getTotalNumberOfGribRecords();

        GribRecord * getFirstGribRecord();
		
        std::vector<GribRecord *> * getListOfGribRecords(int dataType,int levelType,int levelValue);
        
        double		 getHoursBeetweenGribRecords()  {return hoursBetweenRecords;}
        GribRecord * getGribRecord(int dataType,int levelType,int levelValue, time_t date);
        int getDewpointDataStatus(int /*levelType*/,int /*levelValue*/);
        
        std::set<time_t>  * getListDates()   {return &setAllDates;}
        
        int        getNumberOfDates()      {return setAllDates.size();}
        time_t     getRefDate()            {return setAllDates.size()>0 ?
                                                       *setAllDates.begin() : 0;}
        time_t     getMinDate()            {return ok?minDate:-1; }
        time_t     getMaxDate()            {return ok?maxDate:-1; }

        int getInterpolationMode(void) { return interpolation_param; }
        void setInterpolationMode(int val) { interpolation_param=val; }

        // Valeur pour un point et une date quelconques

        bool getInterpolatedValue_byDates(double d_long, double d_lat, time_t now,double * u, double * v,
                                          int interpolation_type=INTERPOLATION_UKN,bool debug=false);

        // Rectangle de la zone couverte par les données
        bool getZoneExtension (double *x0,double *y0, double *x1,double *y1);

        void setCurrentDate(time_t t);
        time_t      getCurrentDate()         {if(isOk()) return currentDate; else return 0;}

        void    setIsobarsStep(double step);
        double  getIsobarsStep() const {return isobarsStep;}
        void    setIsotherms0Step(double step);
        double  getIsotherms0Step() const {return isotherms0Step;}

        // Carte de couleurs du vent
        void draw_WIND_Color(QPainter &pnt, const Projection *proj, bool smooth,
                               bool showWindArrows,bool barbules);
        QString drawCartouche(QPainter &pnt);
        void show_CoverZone(QPainter &pnt, Projection * proj);
        void draw_GribGrid(QPainter &pnt, const Projection *proj);
        // Carte de couleurs des précipitations
        void draw_RAIN_Color(QPainter &pnt, const Projection *proj, bool smooth);
        void draw_SNOW_DEPTH_Color(QPainter &pnt, const Projection *proj, bool smooth);
        void draw_SNOW_CATEG_Color(QPainter &pnt, const Projection *proj, bool smooth);
        void draw_CAPEsfc(QPainter &pnt, const Projection *proj, bool smooth);
        void draw_FRZRAIN_CATEG_Color(QPainter &pnt, const Projection *proj, bool smooth);
        // Carte de couleurs de nébulosité
        void draw_CLOUD_Color(QPainter &pnt, const Projection *proj, bool smooth);
        // Carte de l'humidité relative en couleurs
        void draw_HUMID_Color(QPainter &pnt, const Projection *proj, bool smooth);
        void draw_Temp_Color(QPainter &pnt, const Projection *proj, bool smooth);
        void draw_TempPot_Color(QPainter &pnt, const Projection *proj, bool smooth);
        void draw_Dewpoint_Color(QPainter &pnt, const Projection *proj, bool smooth);
        // Carte de l'écart température-point de rosée
        void draw_DeltaDewpoint_Color(QPainter &pnt, const Projection *proj, bool smooth);

        void draw_PRESSURE_MinMax (QPainter &pnt, const Projection *proj);

        void  draw_Isobars (QPainter &pnt, const Projection *proj);
        void  draw_IsobarsLabels (QPainter &pnt, const Projection *proj);

        void  draw_Isotherms0 (QPainter &pnt, const Projection *proj);
        void  draw_Isotherms0Labels (QPainter &pnt, const Projection *proj);

        // Températue (labels répartis sur la carte)
        void draw_TEMPERATURE_Labels(QPainter &pnt, const Projection *proj);

        enum GribFileDataStatus { DATA_IN_FILE, NO_DATA_IN_FILE, COMPUTED_DATA };
        QRgb   getWindColor     (const double v, const bool smooth);

    private:
        bool   ok;
        std::string fname;
        ZUFILE *file;
        long    fileSize;
        double  hoursBetweenRecords;
        int	dewpointDataStatus;

        std::map < std::string,
        		   std::vector<GribRecord *>* >  mapGribRecords;
        void initNewGrib();
        void storeRecordInMap(GribRecord *rec);
		
        void   readGribFileContent();
        void   readAllGribRecords();
        void   createListDates();
        double computeHoursBeetweenGribRecords();
        double computeDewPoint(double lon, double lat, time_t now);
        std::set<time_t> setAllDates;
        
        void clean_vector(std::vector<GribRecord *> &ls);
        void clean_all_vectors();
        std::vector<GribRecord *> * getFirstNonEmptyList();

        void initIsobars();
        void initIsotherms0();
        std::list<IsoLine *> listIsobars;      // liste d'isobares précalculées
        std::list<IsoLine *> listIsotherms0;   // liste d'isothermes 0C précalculées

        QString 	fileName;

        time_t  	currentDate;
        time_t          minDate;
        time_t          maxDate;

        QColor windColor[14];        // couleur selon la force du vent en beauforts
        QColor rainColor[17];
        int    mapColorTransp;

        int    windArrowSpace;        // distance mini entre flèches (pixels)
        int    windArrowSpaceOnGrid;  // distance mini entre flèches si affichage sur grille
        int    windBarbuleSpace;      // distance mini entre flèches (pixels)
        int    windBarbuleSpaceOnGrid;  // distance mini entre flèches

        int    windArrowSize;         // longueur des flèches
        int    windBarbuleSize;       // longueur des flèches

        bool	mustDuplicateFirstCumulativeRecord;
        bool	isCloudsColorModeWhite;

        // Détermine les GribRecord qui encadrent une date
        void 	findGribsAroundDate (int dataType,int levelType,int levelValue, time_t date,
                                     GribRecord **before, GribRecord **after);
        bool getInterpolationParam(time_t now,time_t * t1,time_t * t2,GribRecord ** recU1,GribRecord ** recV1,
                           GribRecord ** recU2,GribRecord ** recV2,bool debug=false);

        bool getInterpolatedValue_byDates(double d_long, double d_lat, time_t now, time_t t1,time_t t2,
                                              GribRecord *recU1,GribRecord *recV1,GribRecord *recU2,GribRecord *recV2,
                                              double * u, double * v,int interpolation_type=INTERPOLATION_UKN,bool debug=false);
        bool getGribRecordArroundDates(int dataType,int levelType,int levelValue,
                                        time_t now,time_t * tPrev,time_t * tNxt,
                                        GribRecord ** recPrev,GribRecord ** recNxt);
        void drawWindArrow(QPainter &pnt, int i, int j, double ang);
        void drawWindArrowWithBarbs(
                                QPainter &pnt, int i, int j,
                                double vkn, double ang,
                                bool south);

        void drawColorMapGeneric_1D (
                QPainter &pnt, const Projection *proj, bool smooth,
                time_t now,time_t tPrev,time_t tNxt,
                GribRecord * recPrev,GribRecord * recNxt,
                QRgb (Grib::*function_getColor) (double v, bool smooth)
                );

        void  drawColorMapGeneric_Abs_Delta_Data (
                        QPainter &pnt, const Projection *proj, bool smooth,time_t now,
                        time_t tPrevTemp,time_t tNxtTemp,GribRecord * recPrevTemp,GribRecord * recNxtTemp,
                        time_t tPrevDewpoint,time_t tNxtDewpoint,GribRecord * recPrevDewpoint,GribRecord * recNxtDewpoint,
                        QRgb (Grib::*function_getColor) (double v, bool smooth)
                );
        void draw_IsoLinesLabels(QPainter &pnt, QColor &couleur, const Projection *proj,
                                                        std::list<IsoLine *>liste, double coef);
        void drawTransformedLine( QPainter &pnt,
                double si, double co,int di, int dj, int i,int j, int k,int l);

        void drawPetiteBarbule(QPainter &pnt, bool south,
                    double si, double co, int di, int dj, int b);
        void drawGrandeBarbule(QPainter &pnt,  bool south,
                    double si, double co, int di, int dj, int b);
        void drawTriangle(QPainter &pnt, bool south,
                    double si, double co, int di, int dj, int b);

        QRgb   getAltitudeColor (double m, bool smooth);
        QRgb   getRainColor     (double mm, bool smooth);
        QRgb   getSnowDepthColor(double mm, bool smooth);
        QRgb   getHumidColor    (double v, bool smooth);
        QRgb   getTemperatureColor (double v, bool smooth);
        QRgb   getPressureColor    (double v, bool smooth);
        QRgb   getDeltaTemperaturesColor (double v, bool smooth);

        QRgb   getCAPEColor  (double v, bool smooth);
        QRgb   getCloudColor (double v, bool smooth);
        QRgb   getCloudColor (double v, bool smooth, int colorModeUser);

        int interpolation_param;
        bool mustInterpolateValues;
        double  isobarsStep;          // Ecart entre isobares
        double  isotherms0Step;          // Ecart entre isothermes 0C
};


#endif
