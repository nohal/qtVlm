/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2010 - Christophe Thomas aka Oxygen77

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

#ifndef DATADEF_H
#define DATADEF_H

#include <stdint.h>
#include <cmath>
#include <cassert>
#include <QColor>
#include <QMap>

//#define __QTVLM_WITH_TEST
//#define __QTVLM_SHIFT_INC_MOD
#ifndef QTVLM_PLUGIN
extern QMap<QString,QString> appFolder;
#endif
/* usefull template to store pointer in userData field of widgets */
#include <QVariant>
template <class T> class VPtr
{
public:
    static T* asPtr(QVariant v)
    {
        return  (T *) v.value<void *>();
    }

    static QVariant asQVariant(T* ptr)
    {
        return qVariantFromValue((void *) ptr);
    }
};

/* usefull defines to declare simple getter and setter */
#define FCT_GET(TYPE,VARNAME) \
TYPE get_ ## VARNAME (void) { return VARNAME; }
#define FCT_SET(TYPE,VARNAME) \
void set_ ## VARNAME( TYPE __var ) {this->VARNAME=__var;}
#define FCT_SETGET(TYPE,VARNAME) \
FCT_SET(TYPE,VARNAME) \
FCT_GET(TYPE,VARNAME)

#define FCT_GET_CST(TYPE,VARNAME) \
TYPE get_##VARNAME(void) const { return VARNAME; }
#define FCT_SET_CST(TYPE,VARNAME) \
void set_ ## VARNAME( const TYPE & __var ) {this->VARNAME=__var;}
#define FCT_SETGET_CST(TYPE,VARNAME) \
FCT_SET_CST(TYPE,VARNAME) \
FCT_GET_CST(TYPE,VARNAME)

#define DELETE_PTR(PTR) {if(PTR) delete PTR;}

/* some constant */

#define PI     M_PI
#define PI_2   M_PI_2
#define PI_4   M_PI_4
#define TWO_PI M_PI * 2

#define degToRad(angle) (((angle)/180.0) * PI)
#define radToDeg(angle) (((angle)*180.0) / PI)

#define msToKts_cst     1.9438445
#define msToKts(speed) (msToKts_cst*(speed))
#define ktsToMs(speed) (0.51444444*(speed))

/* Grib Donwld type */
#define GRIB_DWNLD_ZYGRIB   0
#define GRIB_DWNLD_VLM      1
#define GRIB_DWNLD_SAILSDOC 2

/* defines */
#define TYPE_LON 1
#define TYPE_LAT 2
#define TYPE_DATE 1
#define TYPE_TIME 2
#define REC_ACTIVE 1
#define REC_VOID 2

#define BOAT_REAL 0
#define BOAT_VLM  1
#define BOAT_ANY  2
#define BOAT_NOBOAT -1

/* styles */
#ifdef __ANDROID_QTVLM
#define DO_NOT_USE_STYLE
#endif

/* XML file type */
#define DOM_FILE_TYPE     "qtVLM_config"

/* barrier edit mode */
#define BARRIER_EDIT_NO_EDIT      0
#define BARRIER_EDIT_ADD_BARRIER  1
#define BARRIER_EDIT_ADD_POINT    2

#define BARRIER_NO_POINT   -1
#define BARRIER_FIRST_POINT 0
#define BARRIER_LAST_POINT  1

/* request type */
#define VLM_NO_REQUEST     -1
#define VLM_REQUEST_LOGIN  0
#define VLM_DO_REQUEST     1
#define VLM_WAIT_RESULT    2
#define VLM_REQUEST_IDU  3
#define VLM_REQUEST_BOAT 4
#define VLM_REQUEST_TRJ  5
#define VLM_REQUEST_GATE 6
#define VLM_REQUEST_WS    7
#define VLM_REQUEST_PROFILE 8
#define VLM_REQUEST_FLEET   9
#define VLM_REQUEST_POLAR 10
#define VLM_REQUEST_SENDPILOT 11
#define VLM_REQUEST_FLAG 12

/* VLM CMD type */
#define VLM_PILOT_HEADING     0
#define VLM_PILOT_ANGLE    1
#define VLM_PILOT_VMG    3
#define VLM_PILOT_ORTHO  2
#define VLM_PILOT_VBVMG  4

#define PILOT_ADD      0
#define PILOT_UPD      1
#define PILOT_DEL      2

#define BOAT_OWN        0
#define BOAT_BSIT       1

#define ROLE_IDX       Qt::UserRole
#define ROLE_IDU        33

/* NB retry when sending data to VLM */
#define MAX_RETRY 5

/* Interpolations */

#define INTERPOLATION_UKN              0
#define INTERPOLATION_TWSA             1
#define INTERPOLATION_SELECTIVE_TWSA   2
#define INTERPOLATION_HYBRID           3

extern int INTERPOLATION_DEFAULT;

#define SHOW_PSEUDO 0
#define SHOW_NAME   1
#define SHOW_IDU    2

#define SHOW_MY_LIST 0
#define SHOW_TEN_FIRST 1
#define SHOW_TEN_CLOSEST_RANKING 2
#define SHOW_TEN_CLOSEST_DISTANCE 3
#define SHOW_NONE 4
#define SHOW_ALL 5

#define RACE_MAX_BOAT 15

struct boatParam {
    QString pseudo;
    QString name;
    QString user_id;
    bool selected;
    QString rank;
    QString statusVLM;
    QString last1h;
    QString last3h;
    QString last24h;
    QString fromFirst;
    QString pavillon;
    double longitude;
    double latitude;
    QString dnm;
    double distNextMark;
    QString nextMark;
    QString ecartMark;
};
Q_DECLARE_TYPEINFO(boatParam,Q_PRIMITIVE_TYPE);

struct raceParam {
    QString id;
    QString name;
    QList <boatParam*> boats;
    QList <boatParam*> arrived;
    bool displayNSZ;
    double latNSZ;
    double widthNSZ;
    QColor colorNSZ;
    int showWhat;
    bool showReal;
    bool hasReal;
    QString realFilter;
};
Q_DECLARE_TYPEINFO(raceParam,Q_PRIMITIVE_TYPE);

struct windData
{
        double u0;
        double u1;
        double u2;
        double u3;
        double v0;
        double v1;
        double v2;
        double v3;
} ;
Q_DECLARE_TYPEINFO(windData,Q_PRIMITIVE_TYPE);

/* Export Import format */

#define ADRENA_FORMAT 0 /*same format used to export from qtvlm*/
#define MS_FORMAT 1
#define SBS_FORMAT 2

/* colors */

#define SPEED_COLOR_UPDATE    "color: rgb(100, 200, 0);"
#define SPEED_COLOR_VLM       "color: rgb(255, 0, 0);"
#define SPEED_COLOR_NO_POLAR  "color: rgb(255, 170, 127);"

#define QWARN qWarning() << "In" << __FILE__ << ", (line" << __LINE__ << "): "

/* angle helper functions */

#define adjustDouble(VAR) ({ \
    VAR = ((double)((int)qRound(VAR*1000)))/1000; \
    })

#define compDouble(VAR1,VAR2) ({ \
    bool _res;                 \
    double _v1=VAR1;            \
    adjustDouble(_v1);           \
    double _v2=VAR2;            \
    adjustDouble(_v2);           \
    _res = (qAbs(_v1-_v2)<0.0001);         \
    _res;                      \
    })

#define calcAngleSign(VAL,ANGLE) { \
    if(qAbs(VAL)>180)              \
    {                              \
        if(VAL>0)                  \
            ANGLE=-ANGLE;          \
    }                              \
    else                           \
    {                              \
        if(VAL<0)                  \
            ANGLE=-ANGLE;          \
    }                              \
}

#define GRIB_NOTDEF -999999999

#define DATA_NOTDEF -1

enum {
    DATA_PRESSURE=0,          /* 0:  Pa     */
    DATA_GEOPOT_HGT,          /* 1:  gpm    */
    DATA_TEMP,                /* 2:  K      */
    DATA_TEMP_POT,            /* 3:  K      */
    DATA_TMAX,                /* 4:  K      */
    DATA_TMIN,                /* 5:  K      */
    DATA_DEWPOINT,            /* 6:  K      */
    DATA_WIND_VX,             /* 7:  m/s    */
    DATA_WIND_VY,             /* 8:  m/s    */
    DATA_CURRENT_VX ,         /* 9:  m/s    */
    DATA_CURRENT_VY,          /* 10: m/s    */
    DATA_HUMID_SPEC,          /* 11: kg/kg  */
    DATA_HUMID_REL,           /* 12: %      */
    DATA_PRECIP_RATE,         /* 13: kg/m2/s */
    DATA_PRECIP_TOT,          /* 14: kg/m2   */
    DATA_SNOW_DEPTH,          /* 15: m      */
    DATA_CLOUD_TOT,           /* 16: %      */
    DATA_FRZRAIN_CATEG,       /* 17: 1=yes 0=no */
    DATA_SNOW_CATEG,          /* 18: 1=yes 0=no */
    DATA_CIN, 		          /* 19: J/kg   */
    DATA_CAPE, 		          /* 20: J/kg   */
    DATA_WAVES_SIG_HGT_COMB,  /* 21: m */
    DATA_WAVES_WND_DIR,       /* 22: deg */
    DATA_WAVES_WND_HGT,       /* 23: m */
    DATA_WAVES_WND_PERIOD,    /* 24: s */
    DATA_WAVES_SWL_DIR,       /* 25: deg */
    DATA_WAVES_SWL_HGT,       /* 26: m */
    DATA_WAVES_SWL_PERIOD,    /* 27: s */
    DATA_WAVES_PRIM_DIR,      /* 28: deg */
    DATA_WAVES_PRIM_PERIOD,   /* 29: s */
    DATA_WAVES_SEC_DIR,       /* 30: deg */
    DATA_WAVES_SEC_PERIOD,    /* 31: s */
    DATA_WAVES_WHITE_CAP,     /* 32: % */
    DATA_WAVES_MAX_DIR,       /* 33: deg */
    DATA_WAVES_MAX_HGT,       /* 34: m */
    DATA_WAVES_MAX_PERIOD,    /* 35: s */
    DATA_ICE_CONCENTRATION,   /* 36: 1=ice, 0=no Ice */
    DATA_MAX

};

#define DATA_LV_NOTDEF     -1

enum {
    DATA_LV_GND_SURF=0,
    DATA_LV_ISOTHERM0,
    DATA_LV_ISOBARIC,
    DATA_LV_MSL,
    DATA_LV_ABOV_GND,
    DATA_LV_SIGMA,
    DATA_LV_ATMOS_ALL,
    DATA_LV_ORDERED_SEQUENCE_DATA,
    DATA_LV_MAX
};

class Couple {

    public:
        Couple(void)        { init(DATA_NOTDEF,0);  }
        Couple(int a,int b) { init(a,b); }
        int a;
        int b;

        void init(int a,int b) { this->a=a; this->b=b; }

       // QDataStream & operator<<(QDataStream & stream, const Couple & couple);
};

inline bool operator<(const Couple &c1, const Couple &c2) {
    if(c1.a == c2.a)
        return c1.b < c2.b;
    else
        return c1.a < c2.a;
}


#ifndef M_PI
#define M_E        2.71828182845904523536
#define M_LOG2E    1.44269504088896340736
#define M_LOG10E   0.434294481903251827651
#define M_LN2      0.693147180559945309417
#define M_LN10     2.30258509299404568402
#define M_PI       3.14159265358979323846
#define M_PI_2     1.57079632679489661923
#define M_PI_4     0.785398163397448309616
#define M_1_PI     0.318309886183790671538
#define M_2_PI     0.636619772367581343076
#define M_2_SQRTPI 1.12837916709551257390
#define M_SQRT2    1.41421356237309504880
#define M_SQRT1_2  0.707106781186547524401
#endif

//--------------------------------------------------------
// Data definition
//--------------------------------------------------------

//--------------------------------------------------------
class DataCode
{
        public:
                DataCode (int dataType=DATA_NOTDEF, int levelType=-1, int levelValue=-1)
                        {  this->dataType   = dataType;
                           this->levelType  = levelType;
                           this->levelValue = levelValue;  }

                int dataType;
                int levelType;
                int levelValue;


                void set (int dataType=DATA_NOTDEF, int levelType=-1, int levelValue=-1)
                        {  this->dataType   = dataType;
                           this->levelType  = levelType;
                           this->levelValue = levelValue;  }

                bool equals (int dataType, int levelType, int levelValue) const
                        {  return this->dataType==dataType &&
                                  this->levelType==levelType &&
                                          this->levelValue==levelValue;  }
                bool operator== (const DataCode &dtc) const
                        { return dtc.dataType==dataType
                                                && dtc.levelType==levelType
                                                && dtc.levelValue==levelValue; }
                bool operator!= (const DataCode &dtc) const
                        { return dtc.dataType!=dataType
                                                || dtc.levelType!=levelType
                                                || dtc.levelValue!=levelValue; }
};
Q_DECLARE_TYPEINFO(DataCode,Q_MOVABLE_TYPE);


#endif // DATADEF_H
