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

This code is mainly coming from vlmc module by vlm: www.virtual-loup-de-mer.org

***********************************************************************/

#include <math.h>
#include <complex.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>

#include "wind.h"
#include "defs.h"

#define _check_angle_interp(a)			\
  if (a > PI) {					\
    a -= TWO_PI;				\
  } else if (a < -PI) {				\
    a += TWO_PI;				\
  }

#define _positive_angle(a)			\
  if (a < 0) {					\
    a += TWO_PI;				\
  } else if (a >= TWO_PI) {			\
    a -= TWO_PI;				\
  }

/**********************/
/* TWSA interpolation */
/**********************/

void get_wind_info_latlong_TWSA(double longitude,  double latitude, time_t now, time_t t1,time_t t2,
                                windData * data_prev, windData * data_nxt,
                                int gribHighRes_t1,int gribHighRes_t2
                                ,double * u_res, double * v_res,int debug)
{
    double u1,u2,v1,v2;
    double t_ratio,angle;
    double u,v;

    get_wind_info_latlong_TWSA_compute(longitude,latitude,data_prev,gribHighRes_t1,&u1,&v1,debug);

    if(data_nxt != NULL)
    {
        get_wind_info_latlong_TWSA_compute(longitude,latitude,data_nxt,gribHighRes_t2,&u2,&v2,debug);

        t_ratio = ((double)(now - t1)) / ((double)(t2 - t1));

        u = u1 + (u2 - u1) * t_ratio;
        angle = (v2 - v1);
        _check_angle_interp(angle);
        v = v1 + (angle) * t_ratio;
        _positive_angle(v);
    }
    else
    {
        u=u1;
        v=v1;
    }

    *u_res=u;
    *v_res=v;
}

#if OLD_C
# define _transform_u_v(a, b)                   \
  t_speed = sqrt(a*a+b*b);                      \
  b = acos(-b/t_speed);                         \
  if (a > 0.0) {                                \
    b = TWO_PI - b;                             \
  }                                             \
  a = msToKts(t_speed);
#else
#  define _transform_u_v(a, b)			\
  c = -b - _Complex_I * a;			\
  a = msToKts(cabs(c));				\
  b = carg(c);					\
  if (b < 0) {					\
    b += TWO_PI;				\
  }
#endif

void get_wind_info_latlong_TWSA_compute(double longitude,  double latitude, windData * data,
                                        int gribHighRes, double * u_res,
                                        double * v_res,int debug)
{
  double u0,u1,u2,u3,v0,v1,v2,v3;
  double u01,u23,v01,v23;
  double u,v;
#if OLD_C
    double t_speed;
#else
    double complex c;
#endif
  double d_long,d_lat;
  double angle;

  u0=data->u0;
  u1=data->u1;
  u2=data->u2;
  u3=data->u3;
  v0=data->v0;
  v1=data->v1;
  v2=data->v2;
  v3=data->v3;

  d_long = longitude; /* is there a +180 drift? see grib */
  if (d_long < 0) {
    d_long += 360;
  } else if (d_long >= 360) {
    d_long -= 360;
  }
  d_lat = latitude + 90; /* is there a +90 drift? see grib*/

  if(gribHighRes)
  {	
    d_long = d_long*2.0;
    d_lat = d_lat*2.0;
  }

  /* we reuse u = speed v = angle after conversion */

  _transform_u_v(u0, v0);
  _transform_u_v(u1, v1);
  _transform_u_v(u2, v2);
  _transform_u_v(u3, v3);

  /* speed interpolation */
  u01 = u0 + (u1 - u0) * (d_lat - floor(d_lat));
  u23 = u2 + (u3 - u2) * (d_lat - floor(d_lat));
  u = u01 + (u23 - u01) * (d_long - floor(d_long));

  angle = (v1 - v0);
  _check_angle_interp(angle);
  v01 = v0 + (angle) * (d_lat - floor(d_lat));
  _positive_angle(v01);

  angle =  (v3 - v2);
  _check_angle_interp(angle);
  v23 = v2 + (angle) * (d_lat - floor(d_lat));
  _positive_angle(v23);

  angle = (v23 - v01);
  _check_angle_interp(angle);
  v = v01 + (angle) * (d_long - floor(d_long));
  _positive_angle(v);

  *u_res=u;
  *v_res=v;
}

/********************************/
/* selective TWSA interpolation */
/********************************/

#if OLD_C
# define _transform_back_u_v(a,b)		\
  t_speed = a * sin(b);				\
  a = a * cos(b);				\
  b = t_speed;
#else
# define _transform_back_u_v(a,b,c)		\
  c = a * cos(b) + _Complex_I * a * sin(b)
#endif

void get_wind_info_latlong_selective_TWSA(double longitude,  double latitude, time_t now, time_t t1,time_t t2,
                                windData * data_prev, windData * data_nxt,
                                int gribHighRes_t1,int gribHighRes_t2,
                                double * u_res, double * v_res,int debug)
{
    double u1,u2,v1,v2;
    double t_ratio,angle;
    double u,v;
    int rot_1,rot_2;
#if OLD_C
    double t_speed;
#else
    double complex c, c01, c23;
#endif

    get_wind_info_latlong_selective_TWSA_compute(longitude,latitude,data_prev,gribHighRes_t1,&u1,&v1,&rot_1,debug);

    if(debug)
    {
        printf("\nInterpolation t1: vit=%f, ang=%f (rot=%d)\n\n",u1,v1,rot_1);
    }

    if(data_nxt != NULL)
    {
        get_wind_info_latlong_selective_TWSA_compute(longitude,latitude,data_nxt,gribHighRes_t2,&u2,&v2,&rot_2,debug);

        if(debug)
        {
            printf("\nInterpolation t2: vit=%f, ang=%f (rot=%d)\n\n",u2,v2,rot_2);
        }

        t_ratio = ((double)(now - t1)) / ((double)(t2 - t1));

        if(debug)
        {
            printf("tratio=%f (rot1=%d,rot2=%d)\n",t_ratio,rot_1,rot_2);
        }

        if ((rot_1 == rot_2) || (rot_1 < 0) || (rot_2 < 0))
        {
            if(debug)
            {
                printf("interpolation tps std\n");
            }

            u = u1 + (u2 - u1) * t_ratio;
            angle = (v2 - v1);
            _check_angle_interp(angle);
            v = v1 + (angle) * t_ratio;
            _positive_angle(v);

            if(debug)
            {
                printf("u=%f v=%f\n",u,v);
            }
        }
        else
        {
#if OLD_C
            if(debug)
            {
                printf("interpolation tps complexe - OLDC\n");
            }

            _transform_back_u_v(u1, v1);
            _transform_back_u_v(u2, v2);
            u = u1 + (u2 - u1) * t_ratio;
            v = v1 + (v2 - v1) * t_ratio;
            _transform_u_v(u, v);

            if(debug)
            {
                printf("u=%f v=%f\n",u,v);
            }

#else
            if(debug)
            {
                printf("interpolation tps complexe: (%f,%f) - (%f,%f)\n",u1,v1,u2,v2);
            }

            _transform_back_u_v(u1, v1, c01);
            _transform_back_u_v(u2, v2, c23);
            c = c01 + (c23 - c01) * t_ratio;

            if(debug)
            {
                printf("interpolation tps complexe: c01 (%f,%f) - c23 (%f,%f)  - c (%f,%f)\n",creal(c01),cimag(c01)
                       ,creal(c23),cimag(c23)
                       ,creal(c),cimag(c));
            }

            u = cabs(c);
            v = carg(c);
            _positive_angle(v);

            if(debug)
            {
                printf("u=%f v=%f\n",u,v);
            }
#endif
        }
    }
    else
    {
        if(debug)
        {
            printf("Pas interpolation temps\n");
        }
        u=u1;
        v=v1;
    }

    *u_res=u;
    *v_res=v;

    if(debug)
    {
        printf("fin interpolation: vit=%f ang=%f\n",u,radToDeg(v));
        fflush(stdout);
    }

}


void get_wind_info_latlong_selective_TWSA_compute(double longitude,  double latitude, windData * data,
                                  int gribHighRes,double * u_res, double * v_res, int * rot,int debug)
{
    double u0,u1,u2,u3,v0,v1,v2,v3;
    double u01,u23,v01,v23;
    double u,v;
#if OLD_C
    double t_speed;
#else
    double complex c, c01, c23;
#endif
    double d_long,d_lat;
    double angle;
    int rot_step1a, rot_step1b, rot_step2;

    u0=data->u0;
    u1=data->u1;
    u2=data->u2;
    u3=data->u3;
    v0=data->v0;
    v1=data->v1;
    v2=data->v2;
    v3=data->v3;

    d_long = longitude; /* is there a +180 drift? see grib */
    if (d_long < 0) {
        d_long += 360;
    } else if (d_long >= 360) {
        d_long -= 360;
    }
    d_lat = latitude + 90; /* is there a +90 drift? see grib*/

    if(debug)
    {
        printf("Donnée IN (highRes=%d)\n",gribHighRes);
        printf("Lat= %f (=> %f ), Lon= %f (=> %f )\n",latitude,d_lat,longitude,d_long);
        printf("grid : lat= %f, Lon= %f\n",floor(d_lat),floor(d_long));
        printf("P0: u= %f, v= %f\n",u0,v0);
        printf("P1: u= %f, v= %f\n",u1,v1);
        printf("P2: u= %f, v= %f\n",u2,v2);
        printf("P3: u= %f, v= %f\n",u3,v3);
    }

    if(gribHighRes)
    {
        d_long = d_long*2.0;
        d_lat = d_lat*2.0;
    }

    _transform_u_v(u0, v0);
    _transform_u_v(u1, v1);
    _transform_u_v(u2, v2);
    _transform_u_v(u3, v3);

    if(debug)
    {
        printf("\nAprès transfo en Complexe\n");
        printf("P0: vit= %f, ang= %f\n",u0,v0);
        printf("P1: vit= %f, ang= %f\n",u1,v1);
        printf("P2: vit= %f, ang= %f\n",u2,v2);
        printf("P3: vit= %f, ang= %f\n",u3,v3);
    }

    /* speed interpolation */
    u01 = u0 + (u1 - u0) * (d_lat - floor(d_lat));
    u23 = u2 + (u3 - u2) * (d_lat - floor(d_lat));

    if(debug)
    {
        printf("Interpolation sur lat de vitesse\n");
        printf("P01: vit= %f\n",u01);
        printf("P23: vit= %f\n",u23);
    }

    angle = (v1 - v0);

    if(debug)
    {
        printf("Interpolation 1 sur lat de ang (angle= %f)\n",angle);
    }

    _check_angle_interp(angle);
    rot_step1a = (angle > 0.0);
    v01 = v0 + (angle) * (d_lat - floor(d_lat));

    if(debug)
    {
        printf("rot_step1A= %d, angle (après chk interp)= %f, v01= %f\n",rot_step1a,angle,v01);
    }

    _positive_angle(v01);

    if(debug)
    {
        printf("positive v01=%f\n",v01) ;
    }

    angle =  (v3 - v2);

    if(debug)
    {
        printf("Interpolation 2 sur lat de ang (angle= %f)\n",angle);
    }

    _check_angle_interp(angle);
    rot_step1b = (angle > 0.0);
    v23 = v2 + (angle) * (d_lat - floor(d_lat));

    if(debug)
    {
        printf("rot_step1B= %d, angle (après chk interp)= %f, v23= %f\n",rot_step1a,angle,v23);
    }

    _positive_angle(v23);

    if(debug)
    {
        printf("positive v23=%f\n",v23) ;
    }


    if (rot_step1a == rot_step1b)
    {
        u = u01 + (u23 - u01) * (d_long - floor(d_long));

        if(debug)
        {
            printf("Interpolation std sur lon, u=%f\n",u);
        }


        angle = (v23 - v01);

        if(debug)
        {
            printf( "Interpolation sur lon de ang (angle= %f)\n",angle);
        }

        _check_angle_interp(angle);
        rot_step2 = (angle > 0.0);
        v = v01 + (angle) * (d_long - floor(d_long));

        if(debug)
        {
            printf("rot_step2= %d, angle (après chk interp)= %f, v= %f\n",rot_step2,angle,v);
        }

        _positive_angle(v);

        if(debug)
        {
            printf("positive v=%f\n",v) ;
        }

    } else {
        rot_step2 = -1;
#if OLD_C
    _transform_back_u_v(u01, v01);
    _transform_back_u_v(u23, v23);
    u = u01 + (u23 - u01) * (d_long - floor(d_long));
    v = v01 + (v23 - v01) * (d_long - floor(d_long));
    _transform_u_v(u, v);
#else
        if(debug)
        {
            printf("Nv Interpolation sur lon\n");
        }
        _transform_back_u_v(u01, v01, c01);
        _transform_back_u_v(u23, v23, c23);
        c = c01 + (c23 - c01) * (d_long - floor(d_long));
        u = cabs(c);
        v = carg(c);
        _positive_angle(v);
        if(debug)
        {
            printf("u=%f, v=%f\n",u,v);
        }
#endif
    }

    *u_res=u;
    *v_res=v;
    *rot=rot_step2;

    if(debug)
    {
        printf("Fin interpolation geo: vit=%f ang=%f\n",u,radToDeg(v));
    }

}

/************************/
/* hybrid interpolation */
/************************/

/* we reuse u = speed v = angle after conversion */
#ifdef OLD_C

#define _speed_u_v(a, b, ro)	   \
    ro = msToKts(sqrt(a*a+b*b));

 #define _hybrid_comp(u,v,angle,ro) { \
    double t_speed;                \
    t_speed = sqrt(u*u+v*v);       \
    angle = acos(-v / t_speed);    \
    if (u > 0.0)                   \
        angle = TWO_PI - angle;    \
}

#else

#define _speed_u_v(a, b, ro)	  \
    ro = msToKts(hypot(a, b));

#define _hybrid_comp(u,v,angle,ro) { \
    double complex c;             \
    c = - v - _Complex_I * u;     \
    angle = carg(c);              \
    if (angle < 0)                \
        angle += TWO_PI;          \
    ro=msToKts(cabs(c));          \
}
#endif /* OLD_C */



void get_wind_info_latlong_hybrid(double longitude,  double latitude, time_t now, time_t t1,time_t t2,
                                windData * data_prev, windData * data_nxt,
                                int gribHighRes_t1, int gribHighRes_t2,
                                double * u_res, double * v_res,int debug)
{
    double u1,u2,v1,v2,ro1,ro2;
    double t_ratio,angle;
    double u,v,ro;

    get_wind_info_latlong_hybrid_compute(longitude,latitude,data_prev,gribHighRes_t1,&u1,&v1,&ro1,debug);

    if(data_nxt != NULL)
    {
        get_wind_info_latlong_hybrid_compute(longitude,latitude,data_nxt,gribHighRes_t2,&u2,&v2,&ro2,debug);

        t_ratio = ((double)(now - t1)) / ((double)(t2 - t1));

        u = u1 + (u2 - u1) * t_ratio;
        v = v1 + (v2 - v1) * t_ratio;

        ro = ro1 + (ro2 - ro1) * t_ratio;

        _hybrid_comp(u,v,angle,ro);
    }
    else
    {
         _hybrid_comp(u1,v1,angle,ro);
         ro=ro1;
    }

    *u_res = ro;
    *v_res= angle;
}

void get_wind_info_latlong_hybrid_compute(double longitude,  double latitude, windData * data,
                      int gribHighRes, double * u_res, double * v_res, double * ro_res,int debug)
{
    double u0,u1,u2,u3,v0,v1,v2,v3;
    double ro0,ro1,ro2,ro3;
    double u01,u23,v01,v23;
    double ro01,ro23;
    double u,v;
    double ro;
    double d_long,d_lat;

    u0=data->u0;
    u1=data->u1;
    u2=data->u2;
    u3=data->u3;
    v0=data->v0;
    v1=data->v1;
    v2=data->v2;
    v3=data->v3;

    d_long = longitude; /* is there a +180 drift? see grib */
    if (d_long < 0) {
        d_long += 360;
    } else if (d_long >= 360) {
        d_long -= 360;
    }
    d_lat = latitude + 90; /* is there a +90 drift? see grib*/

    if(debug)
    {
        printf("Donnée IN (highRes=%d)\n",gribHighRes);
        printf("Lat= %f (=> %f ), Lon= %f (=> %f )\n",latitude,d_lat,longitude,d_long);
        printf("grid : lat= %f, Lon= %f\n",floor(d_lat),floor(d_long));
        printf("P0: u= %f, v= %f\n",u0,v0);
        printf("P1: u= %f, v= %f\n",u1,v1);
        printf("P2: u= %f, v= %f\n",u2,v2);
        printf("P3: u= %f, v= %f\n",u3,v3);
    }

    if(gribHighRes)
    {
        d_long = d_long*2.0;
        d_lat = d_lat*2.0;
    }

    /*
      simple bilinear interpolation, we might factor the cos(lat) in
      the computation to tackle the shape of the pseudo square

      Doing interpolation on angle/speed might be better
    */

    _speed_u_v(u0, v0, ro0);
    _speed_u_v(u1, v1, ro1);
    _speed_u_v(u2, v2, ro2);
    _speed_u_v(u3, v3, ro3);

    /* UV for geting the angle without too much hashing */
    u01 = u0 + (u1 - u0) * (d_lat - floor(d_lat));
    v01 = v0 + (v1 - v0) * (d_lat - floor(d_lat));
    u23 = u2 + (u3 - u2) * (d_lat - floor(d_lat));
    v23 = v2 + (v3 - v2) * (d_lat - floor(d_lat));

    u = u01 + (u23 - u01) * (d_long - floor(d_long));
    v = v01 + (v23 - v01) * (d_long - floor(d_long));

    /* now the speed part */
    ro01 = ro0 + (ro1 - ro0) * (d_lat - floor(d_lat));
    ro23 = ro2 + (ro3 - ro2) * (d_lat - floor(d_lat));

    ro = ro01 + (ro23 - ro01) * (d_long - floor(d_long));

    if(debug)
    {
        printf("-> u01prev: U=%.2f m/s, V=%.2f m/s\n", u01, v01);
        printf("-> u23prev: U=%.2f m/s, V=%.2f m/s\n", u23, v23);
        printf("=>   uprev: U=%.2f m/s, V=%.2f m/s\n", u, v);
        printf("->  roprev: %.2f kts\n", ro);
    }

    *u_res=u;
    *v_res=v;
    *ro_res=ro;
}
