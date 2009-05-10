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

#include "GribPointInfo.h"

//==========================================================================
GribPointInfo::GribPointInfo(GribReader *gribReader, double x, double y, time_t  date)
	{
		this->x = x;
		this->y = y;
		this->date = date;
		this->gribReader = gribReader;
		
		vx = GRIB_NOTDEF;
		vy = GRIB_NOTDEF;
		pressure = GRIB_NOTDEF;
		temp = GRIB_NOTDEF;
		tempPot = GRIB_NOTDEF;
		tempMin = GRIB_NOTDEF;
		tempMax = GRIB_NOTDEF;
		rain    = GRIB_NOTDEF;
		cloud   = GRIB_NOTDEF;
		humid   = GRIB_NOTDEF;
		humidSpec = GRIB_NOTDEF;
		dewPoint  = GRIB_NOTDEF;
		isotherm0HGT = GRIB_NOTDEF;
		snowDepth    = GRIB_NOTDEF;
		snowCateg    = GRIB_NOTDEF;
		frzRainCateg = GRIB_NOTDEF;

		initGribPointInfo();
	}


//==========================================================================
void GribPointInfo::initGribPointInfo()
{
    GribRecord *rec;
    if (gribReader != NULL)
    {
        if ( (rec = gribReader->getGribRecord(GRB_WIND_VX,LV_ABOV_GND,10,date)) != NULL) {
            vx = rec->getInterpolatedValue(x,y);
        }
        if ( (rec = gribReader->getGribRecord(GRB_WIND_VY,LV_ABOV_GND,10,date)) != NULL) {
            vy = rec->getInterpolatedValue(x,y);
        }
        if ( (rec = gribReader->getGribRecord(GRB_PRESSURE,LV_MSL,0,date)) != NULL) {
            pressure = rec->getInterpolatedValue(x,y);
        }
        
        if ( (rec = gribReader->getGribRecord(GRB_TEMP,LV_ABOV_GND,2,date)) != NULL) {
            temp = rec->getInterpolatedValue(x,y);
        }
        if ( (rec = gribReader->getGribRecord(GRB_TPOT,LV_SIGMA,9950,date)) != NULL) {
            tempPot = rec->getInterpolatedValue(x,y);
        }
        if ( (rec = gribReader->getGribRecord(GRB_TMIN,LV_ABOV_GND,2,date)) != NULL) {
            tempMin = rec->getInterpolatedValue(x,y);
        }
        if ( (rec = gribReader->getGribRecord(GRB_TMAX,LV_ABOV_GND,2,date)) != NULL) {
            tempMax = rec->getInterpolatedValue(x,y);
        }        
        if ( (rec = gribReader->getGribRecord(GRB_PRECIP_TOT,LV_GND_SURF,0,date)) != NULL) {
            rain = rec->getInterpolatedValue(x,y);
        }
        if ( (rec = gribReader->getGribRecord(GRB_CLOUD_TOT,LV_ATMOS_ALL,0,date)) != NULL) {
            cloud = rec->getInterpolatedValue(x,y);
        }
        if ( (rec = gribReader->getGribRecord(GRB_HUMID_REL,LV_ABOV_GND,2,date)) != NULL) {
            humid = rec->getInterpolatedValue(x,y);
        }
        
        if ( (rec = gribReader->getGribRecord(GRB_HUMID_SPEC,LV_ABOV_GND,2,date)) != NULL) {
            humidSpec = rec->getInterpolatedValue(x,y);
        }
		
		// Point de rosée : formule de Magnus-Tetens
		dewPoint = gribReader->computeDewPoint(x, y, date);
        if ( (rec = gribReader->getGribRecord(GRB_GEOPOT_HGT,LV_ISOTHERM0,0,date)) != NULL)
        {
            isotherm0HGT = rec->getInterpolatedValue(x,y);
            if (isotherm0HGT != GRIB_NOTDEF)
            	isotherm0HGT = 10* ((int)(isotherm0HGT/10+0.5));
    	}
        
        if ( (rec = gribReader->getGribRecord(GRB_SNOW_DEPTH,LV_GND_SURF,0,date)) != NULL) {
            snowDepth = rec->getInterpolatedValue(x,y);
        }
        if ( (rec = gribReader->getGribRecord(GRB_SNOW_CATEG,LV_GND_SURF,0,date)) != NULL) {
            snowCateg = rec->getInterpolatedValue(x,y);
        }
        if ( (rec = gribReader->getGribRecord(GRB_FRZRAIN_CATEG,LV_GND_SURF,0,date)) != NULL) {
            frzRainCateg = rec->getInterpolatedValue(x,y);
        }


// double p = pressure/100.0;
// double p0 = 1000.0;
// double Lv = 2500.0;
// double Cp = 1004.0;
// double Rd = 287.0;
// double r = humidSpec/(1.0-humidSpec);
// 
// double thetae = (temp + r*Lv/Cp)*pow(p0/p, Rd/Cp);
//if (p>0) printf("humidSpec=%f  thetae=%f\n", humidSpec,thetae-273.15);


    }
}

