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

#include <QDebug>

#include <complex>
#include "dataDef.h"
#ifdef __QTVLM_WITH_TEST
//extern int nbWarning;
#endif
using namespace std;
typedef complex<double> dcmplx;

#include "interpolation.h"

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

  void interpolation::get_wind_info_latlong_TWSA(double longitude,  double latitude, time_t now, time_t t1, time_t t2,
                                windData * data_prev, windData * data_nxt,
                                double lat_step_t1, double lon_step_t1, double lat_step_t2, double lon_step_t2
                                , double * u_res, double * v_res, bool UV, int debug)
{
    double u1,u2,v1,v2;
    double u,v;

    get_wind_info_latlong_TWSA_compute(longitude,latitude,data_prev,lat_step_t1,lon_step_t1,&u1,&v1,UV,debug);

    if(data_nxt != NULL)
    {
        get_wind_info_latlong_TWSA_compute(longitude,latitude,data_nxt,lat_step_t2,lon_step_t2,&u2,&v2,UV,debug);

        const double t_ratio = ((double)(now - t1)) / ((double)(t2 - t1));

        u = u1 + (u2 - u1) * t_ratio;
        double angle = (v2 - v1);
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
  a = t_speed;
#else
#  define _transform_u_v(a, b)			\
  c=dcmplx(-b,- a);			        \
  a = abs(c);				\
  b = arg(c);					\
  if (b < 0) {					\
    b += TWO_PI;				\
  }
#endif

# define _chk_angle(b)         \
  b=degToRad(b);               \
  if (b < 0) {					\
    b += TWO_PI;				\
  }


void interpolation::get_wind_info_latlong_TWSA_compute(double longitude,  double latitude, windData * data,
                                        double lat_step, double lon_step, double * u_res,
                                        double * v_res,bool UV,int /*debug*/)
{
    double u0,u1,u2,u3,v0,v1,v2,v3;
    double u01,u23,v01,v23;
    double u,v;
#if OLD_C
    double t_speed;
#else
    dcmplx c;
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

    d_long = d_long/lon_step;
    d_lat = d_lat/lat_step;

    /* we reuse u = speed v = angle after conversion */

    if(UV) {
        _transform_u_v(u0, v0);
        _transform_u_v(u1, v1);
        _transform_u_v(u2, v2);
        _transform_u_v(u3, v3);
    }
    else {
        _chk_angle(v0);
        _chk_angle(v1);
        _chk_angle(v2);
        _chk_angle(v3);
    }

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
  c = dcmplx(a * cos(b), a * sin(b))

#endif

void interpolation::get_wind_info_latlong_selective_TWSA(double longitude,  double latitude, time_t now, time_t t1,time_t t2,
                                windData * data_prev, windData * data_nxt,
                                double lat_step_t1, double lon_step_t1, double lat_step_t2, double lon_step_t2,
                                double * u_res, double * v_res,bool UV,int debug)
{
    double u1,u2,v1,v2;
    double u,v;
    int rot_1,rot_2;
#if OLD_C
    double t_speed;
#else
    dcmplx  c, c01, c23;
#endif

    get_wind_info_latlong_selective_TWSA_compute(longitude,latitude,data_prev,lat_step_t1,lon_step_t1,&u1,&v1,&rot_1,UV,debug);

    if(debug)
    {
        qWarning("\nInterpolation t1: vit=%f, ang=%f (rot=%d)\n\n",u1,v1,rot_1);
    }

    if(data_nxt != NULL)
    {
        get_wind_info_latlong_selective_TWSA_compute(longitude,latitude,data_nxt,lat_step_t2,lon_step_t2,&u2,&v2,&rot_2,UV,debug);

        if(debug)
        {
            qWarning("\nInterpolation t2: vit=%f, ang=%f (rot=%d)\n\n",u2,v2,rot_2);
        }

        const double t_ratio = ((double)(now - t1)) / ((double)(t2 - t1));

        if(debug)
        {
            qWarning("tratio=%f (rot1=%d,rot2=%d)\n",t_ratio,rot_1,rot_2);
        }

        if ((rot_1 == rot_2) || (rot_1 < 0) || (rot_2 < 0))
        {
            if(debug)
            {
                qWarning("interpolation tps std\n");
            }

            u = u1 + (u2 - u1) * t_ratio;
            double angle = (v2 - v1);
            _check_angle_interp(angle);
            v = v1 + (angle) * t_ratio;
            _positive_angle(v);

            if(debug)
            {
                qWarning("u=%f v=%f\n",u,v);
            }
        }
        else
        {
#if OLD_C
            if(debug)
            {
                qWarning("interpolation tps complexe - OLDC\n");
            }

            _transform_back_u_v(u1, v1);
            _transform_back_u_v(u2, v2);
            u = u1 + (u2 - u1) * t_ratio;
            v = v1 + (v2 - v1) * t_ratio;
            _transform_u_v(u, v);

            if(debug)
            {
                qWarning("u=%f v=%f\n",u,v);
            }

#else
            if(debug)
            {
                qWarning("interpolation tps complexe: (%f,%f) - (%f,%f)\n",u1,v1,u2,v2);
            }

            _transform_back_u_v(u1, v1, c01);
            _transform_back_u_v(u2, v2, c23);
            c = c01 + (c23 - c01) * t_ratio;

            if(debug)
            {
                qWarning("interpolation tps complexe: c01 (%f,%f) - c23 (%f,%f)  - c (%f,%f)\n",c01.real(),c01.imag()
                       ,c23.real(),c23.imag()
                       ,c.real(),c.imag());
            }

            u = abs(c);
            v = arg(c);
            _positive_angle(v);

            if(debug)
            {
                qWarning("u=%f v=%f\n",u,v);
            }
#endif
        }
    }
    else
    {
        if(debug)
        {
            qWarning("Pas interpolation temps\n");
        }
        u=u1;
        v=v1;
    }

    *u_res=u;
    *v_res=v;

    if(debug)
    {
        qWarning("fin interpolation: vit=%f ang=%f\n",u,radToDeg(v));
        fflush(stdout);
    }

}


void interpolation::get_wind_info_latlong_selective_TWSA_compute(double longitude,  double latitude, windData * data,
                                  double lat_step, double lon_step,double * u_res, double * v_res, int * rot,bool UV,int debug)
{
    double u0,u1,u2,u3,v0,v1,v2,v3;
    double u01,u23,v01,v23;
    double u,v;
#if OLD_C
    double t_speed;
#else
    dcmplx c, c01, c23;
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
        qWarning("Donnée IN (step=%f)\n",lat_step);
        qWarning("Lat= %f (=> %f ), Lon= %f (=> %f )\n",latitude,d_lat,longitude,d_long);
        qWarning("grid : lat= %f, Lon= %f\n",floor(d_lat),floor(d_long));
        qWarning("P0: u= %f, v= %f\n",u0,v0);
        qWarning("P1: u= %f, v= %f\n",u1,v1);
        qWarning("P2: u= %f, v= %f\n",u2,v2);
        qWarning("P3: u= %f, v= %f\n",u3,v3);
    }

    d_long = d_long/lon_step;
    d_lat = d_lat/lat_step;

    if(UV) {
        _transform_u_v(u0, v0);
        _transform_u_v(u1, v1);
        _transform_u_v(u2, v2);
        _transform_u_v(u3, v3);
    }
    else {
        _chk_angle(v0);
        _chk_angle(v1);
        _chk_angle(v2);
        _chk_angle(v3);
    }

    if(debug)
    {
        qWarning("\nAprès transfo en Complexe\n");
        qWarning("P0: vit= %f, ang= %f\n",u0,v0);
        qWarning("P1: vit= %f, ang= %f\n",u1,v1);
        qWarning("P2: vit= %f, ang= %f\n",u2,v2);
        qWarning("P3: vit= %f, ang= %f\n",u3,v3);
    }

    /* speed interpolation */
    u01 = u0 + (u1 - u0) * (d_lat - floor(d_lat));
    u23 = u2 + (u3 - u2) * (d_lat - floor(d_lat));

    if(debug)
    {
        qWarning("Interpolation sur lat de vitesse\n");
        qWarning("P01: vit= %f\n",u01);
        qWarning("P23: vit= %f\n",u23);
    }

    angle = (v1 - v0);

    if(debug)
    {
        qWarning("Interpolation 1 sur lat de ang (angle= %f)\n",angle);
    }

    _check_angle_interp(angle);
    rot_step1a = (angle > 0.0);
    v01 = v0 + (angle) * (d_lat - floor(d_lat));

    if(debug)
    {
        qWarning("rot_step1A= %d, angle (après chk interp)= %f, v01= %f\n",rot_step1a,angle,v01);
    }

    _positive_angle(v01);

    if(debug)
    {
        qWarning("positive v01=%f\n",v01) ;
    }

    angle =  (v3 - v2);

    if(debug)
    {
        qWarning("Interpolation 2 sur lat de ang (angle= %f)\n",angle);
    }

    _check_angle_interp(angle);
    rot_step1b = (angle > 0.0);
    v23 = v2 + (angle) * (d_lat - floor(d_lat));

    if(debug)
    {
        qWarning("rot_step1B= %d, angle (après chk interp)= %f, v23= %f\n",rot_step1a,angle,v23);
    }

    _positive_angle(v23);

    if(debug)
    {
        qWarning("positive v23=%f\n",v23) ;
    }


    if (rot_step1a == rot_step1b)
    {
        u = u01 + (u23 - u01) * (d_long - floor(d_long));

        if(debug)
        {
            qWarning("Interpolation std sur lon, u=%f\n",u);
        }

        angle = (v23 - v01);

        if(debug)
        {
            qWarning( "Interpolation sur lon de ang (angle= %f)\n",angle);
        }

        _check_angle_interp(angle);
        rot_step2 = (angle > 0.0);
        v = v01 + (angle) * (d_long - floor(d_long));

        if(debug)
        {
            qWarning("rot_step2= %d, angle (après chk interp)= %f, v= %f\n",rot_step2,angle,v);
        }

        _positive_angle(v);

        if(debug)
        {
            qWarning("positive v=%f\n",v) ;
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
            qWarning("Nv Interpolation sur lon\n");
        }
        _transform_back_u_v(u01, v01, c01);
        _transform_back_u_v(u23, v23, c23);
        c = c01 + (c23 - c01) * (d_long - floor(d_long));
        u = abs(c);
        v = arg(c);
        _positive_angle(v);
        if(debug)
        {
            qWarning("u=%f, v=%f\n",u,v);
        }
#endif
    }

    *u_res=u;
    *v_res=v;
    *rot=rot_step2;

    if(debug)
    {
        qWarning("Fin interpolation geo: vit=%f ang=%f\n",u,radToDeg(v));
    }

}

/************************/
/* hybrid interpolation */
/************************/

/* we reuse u = speed v = angle after conversion */
#ifdef OLD_C

#define _speed_u_v(a, b, ro)	   \
    ro = sqrt(a*a+b*b);

 #define _hybrid_comp(u,v,angle,ro) { \
    double t_speed;                \
    t_speed = sqrt(u*u+v*v);       \
    angle = acos(-v / t_speed);    \
    if (u > 0.0)                   \
        angle = TWO_PI - angle;    \
}

#define SA_2_UV(u,v) {            \
    s=u;                          \
    _transform_back_u_v(u,v);     \
}

#else

#define _speed_u_v(a, b, ro)	  \
    ro = hypot(a, b);

#define _hybrid_comp(u,v,angle,ro) { \
    dcmplx __c(- v, -u);     \
    angle = arg(__c);              \
    if(angle<0) {         \
        angle += TWO_PI;   \
    }                     \
}

#define SA_2_UV(u,v,s) {           \
    dcmplx  _c;                    \
    s=u;                           \
    v=degToRad(v);                 \
    _transform_back_u_v(u,v,_c);   \
    u=_c.real();                   \
    v=_c.imag();                   \
}

#endif

/*
    if(angle<0) {         \
        angle += TWO_PI;   \
    }                     \
    ro=cabs(c);               \
    */


void interpolation::get_wind_info_latlong_hybrid(double longitude,  double latitude, time_t now, time_t t1,time_t t2,
                                windData * data_prev, windData * data_nxt,
                                double lat_step_t1, double lon_step_t1, double lat_step_t2, double lon_step_t2,
                                double * u_res, double * v_res,
                                double gridOriginLat_1,double gridOriginLon_1,double gridOriginLat_2,double gridOriginLon_2,
                                                 bool UV,int debug)
{
    double u1,u2,v1,v2,ro1,ro2;
    double angle;
    double ro;

    get_wind_info_latlong_hybrid_compute(longitude,latitude,data_prev,lat_step_t1,lon_step_t1,&u1,&v1,&ro1,gridOriginLat_1,gridOriginLon_1,UV,debug);

    if(data_nxt != NULL)
    {
        get_wind_info_latlong_hybrid_compute(longitude,latitude,data_nxt,lat_step_t2,lon_step_t2,&u2,&v2,&ro2,gridOriginLat_2,gridOriginLon_2,UV,debug);

        const double t_ratio = ((double)(now - t1)) / ((double)(t2 - t1));

        const double u = u1 + (u2 - u1) * t_ratio;
        const double v = v1 + (v2 - v1) * t_ratio;

        ro = ro1 + (ro2 - ro1) * t_ratio;

        _hybrid_comp(u,v,angle,ro);
    }
    else
    {
        ro=ro1;
        _hybrid_comp(u1,v1,angle,ro);
        //ro=ro1;
    }

    *u_res = ro;
    *v_res= angle;
}

void interpolation::get_wind_info_latlong_hybrid_compute(double longitude,  double latitude, windData * data,
                      double lat_step, double lon_step, double * u_res, double * v_res, double * ro_res,
                                                         double gridOriginLat,double gridOriginLon,
                                                         bool UV,int debug)
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

    double ratioLat,ratioLon;
#ifdef __QTVLM_WITH_TEST
#if 0
    double ratioLonDebug,ratioLatDebug;
    d_long = longitude; /* is there a +180 drift? see grib */
    if (d_long < 0) {
        d_long += 360;
    } else if (d_long >= 360) {
        d_long -= 360;
    }
    d_lat = latitude + 90; /* is there a +90 drift? see grib*/
    d_long = (d_long-gridOriginLon)/lon_step;
    d_lat = (d_lat-gridOriginLat)/lat_step;
    ratioLatDebug=d_lat - floor(d_lat);
    ratioLonDebug=d_long - floor(d_long);
#endif
#endif
    int i0 = (int) floor ((longitude-gridOriginLon)/lon_step);
    int j0 = (int) floor ((latitude-gridOriginLat)/lat_step);
    if(((latitude-gridOriginLat)/lat_step)-j0!=0.0)
    {
        if(lat_step<0)
        {
            ++j0;
        }
    }
    d_long=gridOriginLon+(i0*lon_step);
    d_lat=gridOriginLat+(j0*lat_step);
    ratioLon=(longitude-d_long)/lon_step;
    ratioLat=(latitude-d_lat)/lat_step;
    if(qAbs(qRound(ratioLon*10e7))==0)
        ratioLon=0;
    if(qAbs(qRound(ratioLat*10e7))==0)
        ratioLat=0;
    if(ratioLon<0)
        ratioLon=1.0+ratioLon;
    if(ratioLat<0)
        ratioLat=1.0+ratioLat;
    if(debug)
    {
        qWarning("Donnee IN (step=%fx%f)",lat_step,lon_step);
        qWarning("Lat= %f (=> %f ), Lon= %f (=> %f )",latitude,d_lat,longitude,d_long);
        qWarning("grid : Lat= %f, Lon= %f",gridOriginLat,gridOriginLon);
        qWarning("ratios : Lat= %f, Lon= %f",ratioLat,ratioLon);
        qWarning("P0: u0= %f, v0= %f",u0,v0);
        qWarning("P1: u1= %f, v1= %f",u1,v1);
        qWarning("P2: u2= %f, v2= %f",u2,v2);
        qWarning("P3: u3= %f, v3= %f",u3,v3);
    }
#ifdef __QTVLM_WITH_TEST
//    if(qAbs(ratioLonDebug-ratioLon)>10e-10 || qAbs(ratioLatDebug-ratioLat)>10e-10)
//    {
//        ++nbWarning;
//        if(nbWarning<100)
//        {
//            qWarning("DIFFERENCE in RATIOS!! rLonD=%.7f rLon=%.7f rLatD=%.7f rLat=%.7f",ratioLonDebug,ratioLon,ratioLatDebug,ratioLat);
//            qWarning("rLonD=%d rLon=%d rLatD=%d rLat=%d",qRound(ratioLonDebug*10e7),qRound(ratioLon*10e7),qRound(ratioLatDebug*10e7),qRound(ratioLat*10e7));
//            qWarning()<<qAbs(ratioLon-ratioLonDebug)<<qAbs(ratioLat-ratioLatDebug);
//            qWarning("Donnee IN (step=%fx%f)",lat_step,lon_step);
//            qWarning("Lat= %f (=> %f ), Lon= %f (=> %f )",latitude,d_lat,longitude,d_long);
//            qWarning("grid : Lat= %f, Lon= %f",gridOriginLat,gridOriginLon);
//            qWarning("ratios : Lat= %f, Lon= %f",ratioLat,ratioLon);
//            qWarning("i0= %d, j0= %d",i0,j0);
//        }
//        else if(nbWarning==100)
//            qWarning()<<"stopping qWarning() messages concerning ratios";

//    }
#endif
    /*
      simple bilinear interpolation, we might factor the cos(lat) in
      the computation to tackle the shape of the pseudo square

      Doing interpolation on angle/speed might be better
    */

    if(UV) {
        _speed_u_v(u0, v0, ro0);
        _speed_u_v(u1, v1, ro1);
        _speed_u_v(u2, v2, ro2);
        _speed_u_v(u3, v3, ro3);
    }
    else {
        SA_2_UV(u0,v0,ro0);
        SA_2_UV(u1,v1,ro1);
        SA_2_UV(u2,v2,ro2);
        SA_2_UV(u3,v3,ro3);
    }

    /* UV for geting the angle without too much hashing */
    u01 = u0 + (u1 - u0) * ratioLat;
    v01 = v0 + (v1 - v0) * ratioLat;
    u23 = u2 + (u3 - u2) * ratioLat;
    v23 = v2 + (v3 - v2) * ratioLat;

    u = u01 + (u23 - u01) * ratioLon;
    v = v01 + (v23 - v01) * ratioLon;

    /* now the speed part */
    ro01 = ro0 + (ro1 - ro0) * ratioLat;
    ro23 = ro2 + (ro3 - ro2) * ratioLat;

    ro = ro01 + (ro23 - ro01) * ratioLon;

    if(debug)
    {
        qWarning("-> u01prev: U=%.2f m/s, V=%.2f m/s\n", u01, v01);
        qWarning("-> u23prev: U=%.2f m/s, V=%.2f m/s\n", u23, v23);
        qWarning("=>   uprev: U=%.2f m/s, V=%.2f m/s\n", u, v);
        qWarning("->  roprev: %.2f kts\n", ro);
    }

    *u_res=u;
    *v_res=v;
    *ro_res=ro;
}
