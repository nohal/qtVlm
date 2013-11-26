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

#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <set>
#include <QObject>

#include "dataDef.h"
#include "class_list.h"

class DataManager: public QObject
{ Q_OBJECT
    public:
        DataManager();
        ~DataManager();

        bool load_data(QString fileName,int gribType);
        void close_data(int gribType);

        void set_currentDate(time_t t);
        FCT_GET(time_t,currentDate)
        FCT_GET(time_t,minDate)
        FCT_GET(time_t,maxDate)

        Grib * get_grib(int gribType);
        Grib * get_grib(int dataType,int levelType, int levelValue);

        QMap<int, QList<int> *> *get_levelList(int dataType);
        bool hasDataType(int dataType);
        int get_firstDataType(void);

        std::set<time_t> * get_dateList(void) { return &dateList; }

        QString get_cartoucheData(void);

        QString get_fileName(int gribType);

        bool isOk(int gribType=GRIB_ANY);

        int hasData(int dataType,int levelType, int levelValue);
        bool hasData(int dataType, int levelType, int levelValue,int gribType);

        bool get_data1D(int dataType,int levelType, int levelValue,time_t now,time_t * tPrev,time_t * tNxt,
                                     GribRecord ** recPrev,GribRecord ** recNxt);
        bool get_data2D(int dataType1,int dataType2,int levelType, int levelValue,time_t now,time_t * tPrev,time_t * tNxt,
                                     GribRecord ** recU1,GribRecord ** recV1,GribRecord ** recU2,GribRecord ** recV2);

        double getInterpolatedValue_1D(int dataType,int levelType,int levelValue,
                                       double d_long, double d_lat, time_t now);
        bool getInterpolatedValue_2D(int dataType1, int dataType2, int levelType, int levelValue,
                                           double d_long, double d_lat, time_t now, double * u, double * v,
                                           int interpolation_type=INTERPOLATION_UKN, bool UV=true, bool debug=false);
        bool getInterpolatedWind(double d_long, double d_lat, time_t now,double * u, double * v,
                                              int interpolation_type=INTERPOLATION_UKN,bool debug=false);
        bool getInterpolatedCurrent(double d_long, double d_lat, time_t now,double * u, double * v,
                                              int interpolation_type=INTERPOLATION_UKN,bool debug=false);


        bool getZoneExtension (int gribType,double *x0,double *y0, double *x1,double *y1);

        FCT_SETGET(int,interpolationMode)

        void set_isoBarsStep(double step);
        void set_isoTherms0Step(int step);


        FCT_GET(double,isoBarsStep)
        FCT_GET(int,isoTherms0Step)

        QMap<int,QStringList> * get_levelTypes(void) { return &levelTypes; }
        QMap<int,QStringList> * get_dataTypes(void) { return &dataTypes; }
        QMap<int,QStringList> * get_arrowTypesFst(void) { return &arrowTypesFst; }
        QMap<int,QStringList> * get_arrowTypesSec(void) { return &arrowTypesSec; }

        void load_forcedParam();

        enum { GRIB_NONE=0,
               GRIB_GRIB,
               GRIB_CURRENT,
               GRIB_ANY
             };

        void print_firstRecord_bmap(void);
        void print_firstRecord_info(void);
        Couple get_defaultLevel(int type);

    private:

        Grib * grib;
        Grib * gribCurrent;

        Grib ** get_gribPtr(int gribType);

        time_t currentDate;
        void update_dateList(void);
        std::set<time_t> dateList;
        time_t minDate;
        time_t maxDate;

        void update_levelMap(void);
        void clear_levelMap(void);
        QMap<int,QMap<int, QList<int>*> *> levelMap;

        QMap<int,QStringList> levelTypes;
        QMap<int,QStringList> dataTypes;
        QMap<int,QStringList> arrowTypesFst;
        QMap<int,QStringList> arrowTypesSec;
        void init_stringList(void);

        Couple * defaultLevel;
        void init_defaultLevel(void);

        double isoBarsStep;
        int isoTherms0Step;

        bool forceWind;
        double forcedTWS;
        double forcedTWD;
        bool forceCurrents;
        double forcedCS;
        double forcedCD;

        int interpolationMode;
};

#endif // DATAMANAGER_H
