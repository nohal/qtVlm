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


//===============================================================
class Grib
{
    public:
        Grib(DataManager * dataManager);
        ~Grib();

        bool  isOk()                 {return ok;}

        static Grib * loadGrib(QString fileName,DataManager *dataManager);

        virtual bool loadFile(QString fileName) = 0;

        void init_isos(time_t t);

        FCT_GET(int,version)
        FCT_GET(QString,fileName)

        FCT_GET(int,dewpointDataStatus)
        enum GribFileDataStatus { DATA_IN_FILE, NO_DATA_IN_FILE, COMPUTED_DATA };

        void update_dateList(std::set<time_t> * dateList);
        void update_levelMap(QMap<int, QMap<int, QList<int> *> *> *levelMap);

        bool getZoneExtension (double *x0,double *y0, double *x1,double *y1);

        bool hasData(int dataType, int levelType, int levelValue);

        GribRecord * getGribRecord(int dataType,int levelType,int levelValue, time_t date);
        GribRecord * getFirstRecord(void);
        int getNumberOfGribRecords(int dataType,int levelType,int levelValue);

        /* iso management */
        std::list<IsoLine *> * get_isobars(void) { return &listIsobars; }
        std::list<IsoLine *> * get_isotherms0(void) { return &listIsotherms0; }
        void init_isoBars(time_t t);
        void init_isoTherms0(time_t t);

        /* get records arround a date */
        void find_recordsAroundDate(int dataType,int levelType,int levelValue, time_t date,
                                                                GribRecord **before, GribRecord **after);
        bool get_recordsAndTime_1D(int dataType,int levelType,int levelValue,
                                        time_t now,time_t * tPrev,time_t * tNxt,
                                        GribRecord ** recPrev,GribRecord ** recNxt);
        bool get_recordsAndTime_2D(int dataType_1,int dataType_2,int levelType,int levelValue,
                                         time_t now,time_t * t1,time_t * t2,GribRecord ** recU1,GribRecord ** recV1,
                                   GribRecord ** recU2,GribRecord ** recV2,bool debug=false);

        /* interpolation */
        static bool interpolateValue_1D(double d_long, double d_lat, time_t now, time_t tPrev, time_t tNxt,
                                            GribRecord * recPrev, GribRecord * recNxt, double *res);
        bool getInterpolatedValue_1D(int dataType,int levelType,int levelValue,
                                       double d_long, double d_lat, time_t now, double *res);
        bool getInterpolatedValue_2D(int dataType1,int dataType2,int levelType,int levelValue,
                                           double d_long, double d_lat, time_t now,double * u, double * v,
                                           int interpolation_type,bool UV,bool debug=false);
        static bool interpolateValue_2D(double d_long, double d_lat, time_t now, time_t t1,time_t t2,
                                                      GribRecord *recU1,GribRecord *recV1,GribRecord *recU2,GribRecord *recV2,
                                                      double * u, double * v,int interpolation_type,bool UV,bool debug=false);

    protected:
        bool   ok;
        DataManager * dataManager;
        int version;
        QString fileName;

        std::map <long int,std::vector<GribRecord *>* >  mapGribRecords;
        void addRecord(GribRecord * rec);
        void clean_vector(std::vector<GribRecord *> &ls);
        void clean_all_vectors();


        std::list<IsoLine *> listIsobars;      // liste d'isobares precalculees
        std::list<IsoLine *> listIsotherms0;   // liste d'isothermes 0C precalculees

        void createDewPointData(void);
        int	dewpointDataStatus;
        double computeDewPoint(double lon, double lat, time_t now);

        std::vector<GribRecord *> * getFirstNonEmptyList();
        std::vector<GribRecord *> * getListOfGribRecords(int dataType,int levelType,int levelValue);
};
Q_DECLARE_TYPEINFO(Grib,Q_MOVABLE_TYPE);


#endif
