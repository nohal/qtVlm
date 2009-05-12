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
    }
}

