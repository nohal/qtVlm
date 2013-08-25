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
Lecture mise en memoire d'un fichier GRIB

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

#define MUST_INTERPOLATE_VALUE true

//===============================================================
class Grib
{
    public:
        Grib();
        ~Grib();

        void  loadGribFile(QString fileName);
        bool  isOk()                 {return ok;}
        long  getFileSize()          {return fileSize;}
        std::string getFileName()    {return fname;}

        FCT_GET(bool,mustInterpolateValues)


        int          getNumberOfGribRecords(int dataType,int levelType,int levelValue);
        int          getTotalNumberOfGribRecords();
        QString      get_cartoucheData(void);

        GribRecord * getFirstGribRecord();

        std::vector<GribRecord *> * getListOfGribRecords(int dataType,int levelType,int levelValue);

        double		 getHoursBeetweenGribRecords()  {return hoursBetweenRecords;}
        GribRecord * getGribRecord(int dataType,int levelType,int levelValue, time_t date);
        int getDewpointDataStatus(int /*levelType*/,int /*levelValue*/);

        std::set<time_t>  * getListDates()   {return &setAllDates;}

        int        getNumberOfDates()      {return (int)setAllDates.size();}
        time_t     getRefDate()            {return setAllDates.empty() ?
                                                       0 : *setAllDates.begin();}
        time_t     getMinDate()            {return ok?minDate:-1; }
        time_t     getMaxDate()            {return ok?maxDate:-1; }

        int getInterpolationMode(void) { return interpolation_param; }
        void setInterpolationMode(int val) { interpolation_param=val; }

        // Valeur pour un point et une date quelconques

        bool getInterpolatedValue_byDates(double d_long, double d_lat, time_t now,double * u, double * v,
                                          int interpolation_type=INTERPOLATION_UKN,bool debug=false);
        bool getInterpolatedValueCurrent_byDates(double d_long, double d_lat, time_t now,double * u, double * v,
                                          int interpolation_type=INTERPOLATION_UKN,bool debug=false);

        bool getInterpolatedValue_byDates(double d_long, double d_lat, time_t now, time_t t1,time_t t2,
                                              GribRecord *recU1,GribRecord *recV1,GribRecord *recU2,GribRecord *recV2,
                                              double * u, double * v,int interpolation_type=INTERPOLATION_UKN,bool debug=false);
        // Determine les GribRecord qui encadrent une date
        void 	findGribsAroundDate (int dataType,int levelType,int levelValue, time_t date,
                                     GribRecord **before, GribRecord **after);
        bool getInterpolationParam(time_t now,time_t * t1,time_t * t2,GribRecord ** recU1,GribRecord ** recV1,
                           GribRecord ** recU2,GribRecord ** recV2,bool debug=false);
        bool getInterpolationParamCurrent(time_t now,time_t * t1,time_t * t2,GribRecord ** recU1,GribRecord ** recV1,
                           GribRecord ** recU2,GribRecord ** recV2,bool debug=false);

        bool getGribRecordArroundDates(int dataType,int levelType,int levelValue,
                                        time_t now,time_t * tPrev,time_t * tNxt,
                                        GribRecord ** recPrev,GribRecord ** recNxt);
        double getInterpolatedValue_byDates(int dataType,int levelType,int levelValue,double d_long, double d_lat, time_t now);
        double getInterpolatedValue_byDates(double d_long, double d_lat, time_t now,time_t tPrev,time_t tNxt,
                                            GribRecord * recPrev,GribRecord * recNxt);
        // Rectangle de la zone couverte par les donnees
        bool getZoneExtension (double *x0,double *y0, double *x1,double *y1);

        void setCurrentDate(time_t t);
        time_t      getCurrentDate()         {if(isOk()) return currentDate; else return 0;}

        enum GribFileDataStatus { DATA_IN_FILE, NO_DATA_IN_FILE, COMPUTED_DATA };

        void setIsCurrentGrib(){this->isCurrentGrib=true;}
        void setGribCurrent(Grib * g){this->gribCurrent=g;}

        void load_forcedParam();

        void    setIsobarsStep(double step);
        double  getIsobarsStep() const {return isobarsStep;}
        void    setIsotherms0Step(double step);
        double  getIsotherms0Step() const {return isotherms0Step;}
        std::list<IsoLine *> * get_isobars(void) { return &listIsobars; }
        std::list<IsoLine *> * get_isotherms0(void) { return &listIsotherms0; }

private:
        bool   ok;
        bool   isCurrentGrib;
        std::string fname;
        ZUFILE *file;
        long    fileSize;
        double  hoursBetweenRecords;
        int	dewpointDataStatus;

        std::map <long int,std::vector<GribRecord *>* >  mapGribRecords;

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


        QString 	fileName;

        time_t  	currentDate;
        time_t          minDate;
        time_t          maxDate;

        void initIsobars();
        void initIsotherms0();
        std::list<IsoLine *> listIsobars;      // liste d'isobares precalculees
        std::list<IsoLine *> listIsotherms0;   // liste d'isothermes 0C precalculees
        double  isobarsStep;          // Ecart entre isobares
        double  isotherms0Step;          // Ecart entre isothermes 0C

        bool	mustDuplicateFirstCumulativeRecord;

        int interpolation_param;
        bool mustInterpolateValues;

        Grib * gribCurrent;
        bool findCompression();        

        bool forceWind;
        double forcedTWS;
        double forcedTWD;
        bool forceCurrents;
        double forcedCS;
        double forcedCD;
};
Q_DECLARE_TYPEINFO(Grib,Q_MOVABLE_TYPE);


#endif
