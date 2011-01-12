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

#ifndef INTERPOLATION_H
#define INTERPOLATION_H

#include <time.h>

#include "dataDef.h"

class interpolation
{
    public:
        static void get_wind_info_latlong_TWSA(double longitude,  double latitude, time_t now, time_t t1,time_t t2,
                                    windData * data_prev, windData * data_nxt,
                                    int gribHighRes_t1, int gribHighRes_t2,
                                    double * u_res, double * v_res,int debug);

        static void get_wind_info_latlong_selective_TWSA(double longitude,  double latitude, time_t now, time_t t1,time_t t2,
                                    windData * data_prev, windData * data_nxt,
                                    int gribHighRes_t1, int gribHighRes_t2,
                                    double * u_res, double * v_res,int debug);

        static void get_wind_info_latlong_hybrid(double longitude,  double latitude, time_t now, time_t t1,time_t t2,
                                    windData * data_prev, windData * data_nxt,
                                    int gribHighRes_t1, int gribHighRes_t2,
                                    double * u_res, double * v_res,int debug);

        static void get_wind_info_latlong_TWSA_compute(double longitude,  double latitude, windData * data,int gribHighRes,
                double * u_res, double * v_res,int debug);

        static void get_wind_info_latlong_selective_TWSA_compute(double longitude,  double latitude, windData * data,int gribHighRes,
                double * u_res, double * v_res, int * rot,int debug);

        static void get_wind_info_latlong_hybrid_compute(double longitude,  double latitude, windData * data,int gribHighRes,
                double * u_res, double * v_res, double * ro_res,int debug);
};

#endif // INTERPOLATION_H