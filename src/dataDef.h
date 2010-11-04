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

//#define __QTVLM_WITH_TEST

/* some constant */

#define PI     M_PI
#define PI_2   M_PI_2
#define PI_4   M_PI_4
#define TWO_PI M_PI * 2

#define degToRad(angle) (((angle)/180.0) * PI)
#define radToDeg(angle) (((angle)*180.0) / PI)

#define msToKts(speed) (1.9438445*(speed))
#define ktsToMs(speed) (0.51444444*(speed))

/* defines */
#define TYPE_LON 1
#define TYPE_LAT 2
#define TYPE_DATE 1
#define TYPE_TIME 2
#define REC_ACTIVE 1
#define REC_VOID 2

#define BOAT_REAL 0
#define BOAT_VLM  1
#define BOAT_NOBOAT -1

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

/* VLM CMD type */
#define VLM_CMD_HD     1
#define VLM_CMD_ANG    2
#define VLM_CMD_WP     3
#define VLM_CMD_ORTHO  4
#define VLM_CMD_VMG    5
#define VLM_CMD_VBVMG  6

#define PILOT_ADD      0
#define PILOT_UPD      1
#define PILOT_DEL      2

#define BOAT_OWN        0
#define BOAT_BSIT       1

#define ROLE_IDX       Qt::UserRole

/* NB retry when sending data to VLM */
#define MAX_RETRY 5

/* colors */

#define SPEED_COLOR_UPDATE    "color: rgb(100, 200, 0);"
#define SPEED_COLOR_VLM       "color: rgb(255, 0, 0);"
#define SPEED_COLOR_NO_POLAR  "color: rgb(255, 170, 127);"

/* angle helper functions */

#define adjustFloat(VAR) ({ \
    VAR = ((float)((int)qRound(VAR*1000)))/1000; \
    })

#define compFloat(VAR1,VAR2) ({ \
    bool _res;                 \
    float _v1=VAR1;            \
    adjustFloat(_v1);           \
    float _v2=VAR2;            \
    adjustFloat(_v2);           \
    _res = (qAbs(_v1-_v2)<0.0001);         \
    _res;                      \
    })

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

//--------------------------------------------------------
// dataTypes      cf. GribRecord::translateDataType()
//--------------------------------------------------------
#define GRB_PRESSURE        2   /* Pa     */
#define GRB_GEOPOT_HGT      7   /* gpm    */
#define GRB_TEMP           11   /* K      */
#define GRB_TEMP_POT       13   /* K      */
#define GRB_TMAX           15   /* K      */
#define GRB_TMIN           16   /* K      */
#define GRB_DEWPOINT       17   /* K      */
#define GRB_WIND_VX        33   /* m/s    */
#define GRB_WIND_VY        34   /* m/s    */
#define GRB_HUMID_SPEC     51   /* kg/kg  */
#define GRB_HUMID_REL      52   /* %      */
#define GRB_PRECIP_RATE    59   /* l/m2/s */
#define GRB_PRECIP_TOT     61   /* l/m2   */
#define GRB_SNOW_DEPTH     66   /* m      */
#define GRB_CLOUD_TOT      71   /* %      */
#define GRB_FRZRAIN_CATEG 141   /* 1=yes 0=no */
#define GRB_SNOW_CATEG    143   /* 1=yes 0=no */
#define GRB_CAPE 		  157   /* J/kg   */

#define GRB_WIND_DIR       249   /* private: wind direction in degrees */
#define GRB_WIND_XY2D      250   /* private: GRB_WIND_VX+GRB_WIND_VX */
#define GRB_DIFF_TEMPDEW   251   /* private: GRB_TEMP-GRB_DEWPOINT */
#define GRB_THETA_E   	   252   /* K   */
#define GRB_WIND_GUST 	   253   /* m/s */
#define GRB_PRECIP_PROB	   254   /* %   */

#define GRB_TYPE_NOT_DEFINED 0   /* private */

//--------------------------------------------------------
// Levels types (altitude reference)
//--------------------------------------------------------
#define LV_GND_SURF    1
#define LV_ISOTHERM0   4
#define LV_ISOBARIC  100
#define LV_MSL       102
#define LV_ABOV_GND  105
#define LV_SIGMA     107
#define LV_ATMOS_ALL  200
#define LV_ATMOS_LOW  214
#define LV_ATMOS_MID  224
#define LV_ATMOS_HIGH 234


#define LV_TYPE_NOT_DEFINED 0   /* private */

// altitude index in tables
#define H850 0
#define H700 1
#define H500 2
#define H300 3
#define H200 4

#define GEOPOTidx(h) ((h)==850?0:(h)==700?1:(h)==500?2:(h)==300?3:(h)==200?4:-1)
#define GEOPOThgt(i) ((i)==0?850:(i)==1?700:(i)==2?500:(i)==3?300:(i)==4?200:-1)

//--------------------------------------------------------
// Data definition
//--------------------------------------------------------
class Altitude
{
        public:
                Altitude (int levelType=-1, int levelValue=-1)
                        {  this->levelType  = levelType;
                           this->levelValue = levelValue;  }

                int levelType;
                int levelValue;

                void set (int levelType=-1, int levelValue=-1)
                        {  this->levelType  = levelType;
                           this->levelValue = levelValue;  }

                int index () const  { if (levelType==LV_ISOBARIC)
                                                                return GEOPOTidx(levelValue);
                                                          else return -1; }

                bool equals (int levelType, int levelValue) const
                        {  return this->levelType==levelType &&
                                          this->levelValue==levelValue;  }
                bool operator== (const Altitude &alt) const
                        { return alt.levelType==levelType
                                                && alt.levelValue==levelValue; }
                bool operator!= (const Altitude &alt) const
                        { return alt.levelType!=levelType
                                                || alt.levelValue!=levelValue; }
                bool operator< (const Altitude &alt) const
                        { return alt.levelType<levelType
                                                ||
                                        (alt.levelType==levelType && alt.levelValue<levelValue); }
};
//--------------------------------------------------------
class DataCode
{
        public:
                DataCode (int dataType=GRB_TYPE_NOT_DEFINED, int levelType=-1, int levelValue=-1)
                        {  this->dataType   = dataType;
                           this->levelType  = levelType;
                           this->levelValue = levelValue;  }

                DataCode (int dataType, const Altitude &alt)
                        {  this->dataType   = dataType;
                           this->levelType  = alt.levelType;
                           this->levelValue = alt.levelValue;  }

                DataCode (uint32_t v)   { fromInt32 (v); }

                int dataType;
                int levelType;
                int levelValue;

                // int32 = #aabbccdd    aabb=levelValue cc=levelType dd=dataCode
                uint32_t toInt32 () {
                        return ((levelValue&0xFFFF)<<16)+((levelType&0xFF)<<8)+(dataType&0xFF);
                }
                void fromInt32 (uint32_t v) {
                        levelValue = (v>>16) & 0xFFFF;
                        levelType  = (v>>8) & 0xFF;
                        dataType   =  v     & 0xFF;
                }

                void set (int dataType=GRB_TYPE_NOT_DEFINED, int levelType=-1, int levelValue=-1)
                        {  this->dataType   = dataType;
                           this->levelType  = levelType;
                           this->levelValue = levelValue;  }

                Altitude getAltitude () const
                        { return Altitude(levelType, levelValue); }

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
                bool operator< (const DataCode &dtc) const
                        { return dataType<dtc.dataType
                                                ||
                                        (dtc.dataType==dataType && getAltitude()<dtc.getAltitude()); }
};


#endif // DATADEF_H
