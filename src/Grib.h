/**********************************************************************
zyGrib: meteorological GRIB file viewer
Copyright (C) 2008 - Jacques Zaninetti - http://www.zygrib.org

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

#include "Projection.h"
#include "GribRecord.h"
#include "zuFile.h"
#include "Util.h"

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
        
        std::set<time_t>  * getListDates()   {return &setAllDates;}
        
        int        getNumberOfDates()      {return setAllDates.size();}
        time_t     getRefDate()            {return setAllDates.size()>0 ?
                                                       *setAllDates.begin() : 0;}
        time_t     getMinDate()            {return ok?minDate:-1; }
        time_t     getMaxDate()            {return ok?maxDate:-1; }

        // Valeur pour un point et une date quelconques

        bool getInterpolatedValue_byDates(double d_long, double d_lat, time_t now,double * u, double * v);

        // Rectangle de la zone couverte par les données
        bool getZoneExtension (double *x0,double *y0, double *x1,double *y1);

        void setCurrentDate(time_t t) { currentDate = t; }
        time_t      getCurrentDate()         {return currentDate;}

        // Carte de couleurs du vent
        void draw_WIND_Color(QPainter &pnt, const Projection *proj, bool smooth,
                               bool showWindColorMap, bool showWindArrows,bool barbules);
        void drawCartouche(QPainter &pnt);

        enum GribFileDataStatus { DATA_IN_FILE, NO_DATA_IN_FILE, COMPUTED_DATA };

    private:
        bool   ok;
        std::string fname;
        ZUFILE *file;
        long    fileSize;
        double  hoursBetweenRecords;

        std::map < std::string,
        		   std::vector<GribRecord *>* >  mapGribRecords;
        void initNewGrib();
        void storeRecordInMap(GribRecord *rec);
		
        void   readGribFileContent();
        void   readAllGribRecords();
        void   createListDates();
        double computeHoursBeetweenGribRecords();
        std::set<time_t> setAllDates;
        
        void clean_vector(std::vector<GribRecord *> &ls);
        void clean_all_vectors();
        std::vector<GribRecord *> * getFirstNonEmptyList();

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
                           GribRecord ** recU2,GribRecord ** recV2);
        bool getInterpolatedValue_record(double px, double py,
                                                    GribRecord *recU, GribRecord *recV, double * u, double * v);
        bool getInterpolatedValue_byDates(double d_long, double d_lat, time_t now, time_t t1,time_t t2,
                                              GribRecord *recU1,GribRecord *recV1,GribRecord *recU2,GribRecord *recV2,
                                              double * u, double * v);

        void drawWindArrow(QPainter &pnt, int i, int j, double ang);
        void drawWindArrowWithBarbs(
                                QPainter &pnt, int i, int j,
                                double vkn, double ang,
                                bool south);

        void drawTransformedLine( QPainter &pnt,
                double si, double co,int di, int dj, int i,int j, int k,int l);

        void drawPetiteBarbule(QPainter &pnt, bool south,
                    double si, double co, int di, int dj, int b);
        void drawGrandeBarbule(QPainter &pnt,  bool south,
                    double si, double co, int di, int dj, int b);
        void drawTriangle(QPainter &pnt, bool south,
                    double si, double co, int di, int dj, int b);

        QRgb   getWindColor(double v, bool smooth);
};


#endif
