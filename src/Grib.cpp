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

#include <complex>
#include <math.h>

#include <cassert>
#include <QDebug>

#include "Grib.h"
#include "Util.h"

//-------------------------------------------------------------------------------
Grib::Grib()
{
    initNewGrib();
}

//----------------------------------------------------
Grib::Grib(const Grib &model)
{
    initNewGrib ();
    loadGribFile (model.fileName);
}

//----------------------------------------------------
void Grib::initNewGrib()
{
    ok = false;

//    mapColorTransp = 210;
    mapColorTransp = 255;

    windArrowSpace = 28;      // distance mini entre flèches
    windBarbuleSpace = 34;    // distance mini entre flèches

    windArrowSpaceOnGrid = 20;      // distance mini entre flèches
    windBarbuleSpaceOnGrid = 28;    // distance mini entre flèches

    windArrowSize = 14;       // longueur des flèches
    windBarbuleSize = 26;     // longueur des flèches avec barbules

    // Color scale for wind in beaufort
    windColor[ 0].setRgba(qRgba(   0,  80, 255,  mapColorTransp));
    windColor[ 1].setRgba(qRgba(   0, 150, 255,  mapColorTransp));
    windColor[ 2].setRgba(qRgba(   0, 200, 255,  mapColorTransp));
    windColor[ 3].setRgba(qRgba(   0, 250, 180,  mapColorTransp));
    windColor[ 4].setRgba(qRgba(   0, 230, 150,  mapColorTransp));
    windColor[ 5].setRgba(qRgba( 255, 255,   0,  mapColorTransp));
    windColor[ 6].setRgba(qRgba( 255, 220,   0,  mapColorTransp));
    windColor[ 7].setRgba(qRgba( 255, 180,   0,  mapColorTransp));
    windColor[ 8].setRgba(qRgba( 255, 120,   0,  mapColorTransp));
    windColor[ 9].setRgba(qRgba( 230, 120,   0,  mapColorTransp));
    windColor[10].setRgba(qRgba( 220,  80,   0,  mapColorTransp));
    windColor[11].setRgba(qRgba( 200,  50,  30,  mapColorTransp));
    windColor[12].setRgba(qRgba( 170,   0,  50,  mapColorTransp));
    windColor[13].setRgba(qRgba( 150,   0,  30,  mapColorTransp));
}

//-------------------------------------------------------------------------------
Grib::~Grib()
{
    if(ok)
        clean_all_vectors();
}

void Grib::loadGribFile(QString fileName)
{
    this->fileName = fileName;

    ok=false;
    fname = qPrintable(fileName);
    if (fname != "")
    {
        debug("Open file: %s", fname.c_str());
        clean_all_vectors();
        //--------------------------------------------------------
        // Ouverture du fichier
        //--------------------------------------------------------
        file = zu_open(fname.c_str(), "rb", ZU_COMPRESS_AUTO);
        if (file == NULL) {
            erreur("Can't open file: %s", fname.c_str());
            return;
        }
        readGribFileContent();

        // Essaie d'autres compressions (extension non reconnue ?)
        if (! ok) {
            if (file != NULL)
                zu_close(file);
            file = zu_open(fname.c_str(), "rb", ZU_COMPRESS_BZIP);
            if (file != NULL)
                readGribFileContent();
        }
        if (! ok) {
            if (file != NULL)
                zu_close(file);
            file = zu_open(fname.c_str(), "rb", ZU_COMPRESS_GZIP);
            if (file != NULL)
                readGribFileContent();
        }
        if (! ok) {
            if (file != NULL)
                zu_close(file);
            file = zu_open(fname.c_str(), "rb", ZU_COMPRESS_NONE);
            if (file != NULL)
                readGribFileContent();
        }
        setCurrentDate ( setAllDates.size()>0 ? *(setAllDates.begin()) : 0);
        if (file != NULL)
            zu_close(file);
    }
}

//-------------------------------------------------------------------------------
void Grib::clean_all_vectors()
{
        std::map < std::string, std::vector<GribRecord *>* >::iterator it;
        for (it=mapGribRecords.begin(); it!=mapGribRecords.end(); it++) {
                std::vector<GribRecord *> *ls = (*it).second;
                clean_vector( *ls );
                delete ls;
        }
        mapGribRecords.clear();
}
//-------------------------------------------------------------------------------
void Grib::clean_vector(std::vector<GribRecord *> &ls)
{
    std::vector<GribRecord *>::iterator it;
    for (it=ls.begin(); it!=ls.end(); it++) {
        delete *it;
        *it = NULL;
    }
    ls.clear();
}

//---------------------------------------------------------------------------------
void Grib::storeRecordInMap(GribRecord *rec)
{
        std::map <std::string, std::vector<GribRecord *>* >::iterator it;
        it = mapGribRecords.find(rec->getKey());
        if (it == mapGribRecords.end())
        {
                mapGribRecords[rec->getKey()] = new std::vector<GribRecord *>;
                assert(mapGribRecords[rec->getKey()]);
        }
        mapGribRecords[rec->getKey()]->push_back(rec);
}

//---------------------------------------------------------------------------------
void Grib::readAllGribRecords()
{
    //--------------------------------------------------------
    // Lecture de l'ensemble des GribRecord du fichier
    // et stockage dans les listes appropriées.
    //--------------------------------------------------------
    GribRecord *rec;
    int id = 0;
    time_t firstdate = -1;
    do {
        id ++;
        rec = new GribRecord(file, id);
        assert(rec);
        if (rec->isOk())
        {
            ok = true;   // au moins 1 record ok

            if (firstdate== -1)
                firstdate = rec->getRecordCurrentDate();

            if ((rec->getDataType()==GRB_WIND_VX || rec->getDataType()==GRB_WIND_VY)
                      && rec->getLevelType()==LV_ABOV_GND && rec->getLevelValue()==10)
                storeRecordInMap(rec);
        }
        else {    // ! rec-isOk
            delete rec;
            rec = NULL;
        }
    } while (rec != NULL &&  !rec->isEof());
}

//---------------------------------------------------------------------------------
void Grib::readGribFileContent()
{
    fileSize = zu_filesize(file);
    readAllGribRecords();

    createListDates();
    hoursBetweenRecords = computeHoursBeetweenGribRecords();
}

//---------------------------------------------------
int Grib::getTotalNumberOfGribRecords() {
        int nb=0;
        std::map < std::string, std::vector<GribRecord *>* >::iterator it;
        for (it=mapGribRecords.begin(); it!=mapGribRecords.end(); it++)
        {
                nb += (*it).second->size();
        }
        return nb;
}

//---------------------------------------------------
std::vector<GribRecord *> * Grib::getFirstNonEmptyList()
{
    std::vector<GribRecord *> *ls = NULL;
        std::map < std::string, std::vector<GribRecord *>* >::iterator it;
        for (it=mapGribRecords.begin(); ls==NULL && it!=mapGribRecords.end(); it++)
        {
                if ((*it).second->size()>0)
                        ls = (*it).second;
        }
        return ls;
}

//---------------------------------------------------
int Grib::getNumberOfGribRecords(int dataType,int levelType,int levelValue)
{
        std::vector<GribRecord *> *liste = getListOfGribRecords(dataType,levelType,levelValue);
        if (liste != NULL)
                return liste->size();
        else
                return 0;
}

//---------------------------------------------------------------------
std::vector<GribRecord *> * Grib::getListOfGribRecords(int dataType,int levelType,int levelValue)
{
        std::string key = GribRecord::makeKey(dataType,levelType,levelValue);
        if (mapGribRecords.find(key) != mapGribRecords.end())
                return mapGribRecords[key];
        else
                return NULL;
}

//------------------------------------------------------------------
void Grib::findGribsAroundDate (int dataType,int levelType,int levelValue, time_t date,
                                                        GribRecord **before, GribRecord **after)
{
        // Cherche les GribRecord qui encadrent la date
        std::vector<GribRecord *> *ls = getListOfGribRecords(dataType,levelType,levelValue);
        *before = NULL;
        *after  = NULL;
        zuint nb = ls->size();
        for (zuint i=0; i<nb && /**before==NULL &&*/ *after==NULL; i++)
        {
                GribRecord *rec = (*ls)[i];
                if (rec->getRecordCurrentDate() == date) {
                        *before = rec;
                        *after = rec;
                }
                else if (rec->getRecordCurrentDate() < date) {
                        *before = rec;
                }
                else if (rec->getRecordCurrentDate() > date  &&  *before != NULL) {
                        *after = rec;
                }
        }
}

bool Grib::getInterpolatedValue_byDates(double d_long, double d_lat, time_t now,double * u, double * v)
{
    GribRecord *recU1,*recV1,*recU2,*recV2;
    time_t t1,t2;

    if(u) *u=0;
    if(v) *v=0;

    if(!isOk())
        return false;

    if(getInterpolationParam(now,&t1,&t2,&recU1,&recV1,&recU2,&recV2))
    {
        return getInterpolatedValue_byDates(d_long,d_lat,now,t1,t2,recU1,recV1,recU2,recV2,u,v);
    }
    return false;
}

bool Grib::getInterpolationParam(time_t now,time_t * t1,time_t * t2,GribRecord ** recU1,GribRecord ** recV1,
                           GribRecord ** recU2,GribRecord ** recV2)
{
    if(t1 && t2 && recU1 && recV1 && recU2 && recV2)
    {
        findGribsAroundDate(GRB_WIND_VX,LV_ABOV_GND,10,now,recU1,recU2);
        findGribsAroundDate(GRB_WIND_VY,LV_ABOV_GND,10,now,recV1,recV2);
        if(*recU1 && *recV1)
        {
            if(*recU1==*recU2)
            {
                *recU2=NULL;
                *recV2=NULL;
            }
            else
            {
                *t1=(*recU1)->getRecordCurrentDate();
                if(*recU2!=NULL && *recV2!=0)
                    *t2=(*recU2)->getRecordCurrentDate();
            }
            return true;
        }
    }
    return false;
}

#  define _transform_u_v(a, b)	{		\
  std::complex<double> c(-b,-a);		\
  a = msToKts(std::abs(c));                     \
  b = std::arg(c);			        \
  if (b < 0) {					\
    b += TWO_PI;				\
  }                                             \
}

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

# define _transform_back_u_v(a,b,c)		\
   c= std::complex<double>(a * cos(b) , a * sin(b));

bool Grib::getInterpolatedValue_byDates(double d_long, double d_lat, time_t now, time_t t1,time_t t2,
                                              GribRecord *recU1,GribRecord *recV1,GribRecord *recU2,GribRecord *recV2,
                                              double * u, double * v)
{
    double u1,u2,v1,v2;
    double t_ratio,angle;
    double u_res,v_res;
    int rot_1,rot_2;
    std::complex<double> c, c01, c23;

    /*sanity check */
    if(!u || !v || !recU1 || !recV1)
        return false;

    /*interpolation for t1*/
    if(!getInterpolatedValue_record(d_long,d_lat,recU1,recV1,&u1,&v1,&rot_1))
        return false;

    if(recU2 && recV2) /* second interpolation*/
    {
        if(!getInterpolatedValue_record(d_long,d_lat,recU2,recV2,&u2,&v2,&rot_2))
            return false;
        /* time interpolation*/
#if 1
        t_ratio = ((double)(now - t1)) / ((double)(t2 - t1));

        u_res = u1 + (u2 - u1) * t_ratio;
        angle = (v2 - v1);
        _check_angle_interp(angle);
        v_res = v1 + (angle) * t_ratio;
        _positive_angle(v_res);
        *u=u_res;
        *v=v_res;
#else
        t_ratio = ((double)(now - t1)) / ((double)(t2 - t1));
        if ((rot_1 == rot_2) || (rot_1 < 0) || (rot_2 < 0))
        {
            u_res = u1 + (u2 - u1) * t_ratio;
            angle = (v2 - v1);
            _check_angle_interp(angle);
            v_res = v1 + (angle) * t_ratio;
            _positive_angle(v_res);
            *u=u_res;
            *v=v_res;
        }
        else
        {
            _transform_back_u_v(u1, v1, c01);
            _transform_back_u_v(u2, v2, c23);
            c = c01 + (c23 - c01) * t_ratio;
            u_res = std::abs(c);
            v_res = std::arg(c);
            *u=u_res;
            *v=v_res;
        }
#endif
    }
    else
    {
        *u=u1;
        *v=v1;
    }
    return true;
}
#if 1
bool Grib::getInterpolatedValue_record(double d_long, double d_lat, GribRecord *recU, GribRecord *recV,
                                       double * u, double * v, int * )
{
    double u0,u1,u2,u3,v0,v1,v2,v3;
    double u01,u23,v01,v23;
    double u_val,v_val;
    double angle;

    if(!u || !v || !recU || !recV)
        return false;

    if(!recU->getValue_TWSA(d_long,d_lat,&u0,&u1,&u2,&u3))
        return false;
    if(!recV->getValue_TWSA(d_long,d_lat,&v0,&v1,&v2,&v3))
        return false;

    /* is there a +180 drift? see grib */
    if (d_long < 0) {
        d_long += 360;
    } else if (d_long >= 360) {
        d_long -= 360;
    }
    d_lat += 90; /* is there a +90 drift? see grib*/

    if(recU->getDi()==0.5 || recU->getDi()==-0.5) /* is it 0.5 grib otherwise assume it is a grib with step of 1°*/
    {
        d_long = d_long*2.0;
        d_lat = d_lat*2.0;
    }

    _transform_u_v(u0, v0);
    _transform_u_v(u1, v1);
    _transform_u_v(u2, v2);
    _transform_u_v(u3, v3);

    /* speed interpolation */
    u01 = u0 + (u1 - u0) * (d_lat - floor(d_lat));
    u23 = u2 + (u3 - u2) * (d_lat - floor(d_lat));
    u_val = u01 + (u23 - u01) * (d_long - floor(d_long));

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
    v_val = v01 + (angle) * (d_long - floor(d_long));
    _positive_angle(v_val);

    *u=u_val;
    *v=v_val;

    return true;
}
#else

bool Grib::getInterpolatedValue_record(double d_long, double d_lat, GribRecord *recU, GribRecord *recV,
                                       double * u, double * v, int * rot)
{
    double u0,u1,u2,u3,v0,v1,v2,v3;
    double u01,u23,v01,v23;
    double u_val,v_val;
    double angle;
    int rot_step1a, rot_step1b;
    std::complex<double> c;
    std::complex<double> c01;
    std::complex<double> c23;

    if(!u || !v || !recU || !recV || !rot)
        return false;

    if(!recU->getValue_TWSA(d_long,d_lat,&u0,&u1,&u2,&u3))
        return false;
    if(!recV->getValue_TWSA(d_long,d_lat,&v0,&v1,&v2,&v3))
        return false;

    /* is there a +180 drift? see grib */
    if (d_long < 0) {
        d_long += 360;
    } else if (d_long >= 360) {
        d_long -= 360;
    }
    d_lat += 90; /* is there a +90 drift? see grib*/

    if(recU->getDi()==0.5 || recU->getDi()==-0.5) /* is it 0.5 grib otherwise assume it is a grib with step of 1°*/
    {
        d_long = d_long*2.0;
        d_lat = d_lat*2.0;
    }

    _transform_u_v(u0, v0);
    _transform_u_v(u1, v1);
    _transform_u_v(u2, v2);
    _transform_u_v(u3, v3);

    /* speed interpolation */
    u01 = u0 + (u1 - u0) * (d_lat - floor(d_lat));
    u23 = u2 + (u3 - u2) * (d_lat - floor(d_lat));


    angle = (v1 - v0);
    _check_angle_interp(angle);
    rot_step1a = (angle > 0.0);
    v01 = v0 + (angle) * (d_lat - floor(d_lat));
    _positive_angle(v01);

    angle =  (v3 - v2);
    _check_angle_interp(angle);
    rot_step1b = (angle > 0.0);
    v23 = v2 + (angle) * (d_lat - floor(d_lat));
    _positive_angle(v23);

    if (rot_step1a == rot_step1b)
    {
        u_val = u01 + (u23 - u01) * (d_long - floor(d_long));

        angle = (v23 - v01);
        _check_angle_interp(angle);
        *rot = (angle > 0.0);
        v_val = v01 + (angle) * (d_long - floor(d_long));
        _positive_angle(v_val);
    }
    else
    {
        *rot = -1;
        /* rotations are in contrary motion, let's use UV for this */
        _transform_back_u_v(u01, v01, c01);
        _transform_back_u_v(u23, v23, c23);
        c = c01 + (c23 - c01) * (d_long - floor(d_long));
        u_val = std::abs(c);
        v_val = std::arg(c);
    }

    *u=u_val;
    *v=v_val;

    return true;
}
#endif

//---------------------------------------------------
// Rectangle de la zone couverte par les données
bool Grib::getZoneExtension(double *x0,double *y0, double *x1,double *y1)
{
    if(!isOk())
        return false;

    std::vector<GribRecord *> *ls = getFirstNonEmptyList();
    if (ls != NULL) {
        GribRecord *rec = ls->at(0);
        if (rec != NULL) {
            if(rec->getIsFull())
                return false;

            *x0 = rec->getX(0);
            *x1 = rec->getX( rec->getNi()-1 );
            *y0 = rec->getY(0);
            *y1 = rec->getY( rec->getNj()-1 );
            if (*x0 > *x1) {
                double tmp = *x0;
                *x0 = *x1;
                *x1 = tmp;
            }
            if (*y0 > *y1) {
                double tmp = *y0;
                *y0 = *y1;
                *y1 = tmp;
            }
        }
        return true;
    }
    else {
        return false;
    }
}
//---------------------------------------------------
// Premier GribRecord trouvé (pour récupérer la grille)
GribRecord * Grib::getFirstGribRecord()
{
    std::vector<GribRecord *> *ls = getFirstNonEmptyList();
    if (ls != NULL) {
        return ls->at(0);
    }
    else {
        return NULL;
    }
}
//---------------------------------------------------
// Délai en heures entre 2 records
// On suppose qu'il est fixe pour tout le fichier !!!
double Grib::computeHoursBeetweenGribRecords()
{
        double res = 1;
    std::vector<GribRecord *> *ls = getFirstNonEmptyList();
    if (ls != NULL) {
        time_t t0 = (*ls)[0]->getRecordCurrentDate();
        time_t t1 = (*ls)[1]->getRecordCurrentDate();
        res = abs(t1-t0) / 3600.0;
        if (res < 1)
                res = 1;
    }
    return res;
}

//-------------------------------------------------------
// Génère la liste des dates pour lesquelles des prévisions existent
void Grib::createListDates()
{   // Le set assure l'ordre et l'unicité des dates
    minDate=-1;
    maxDate=-1;
    setAllDates.clear();
    std::map < std::string, std::vector<GribRecord *>* >::iterator it;
    for (it=mapGribRecords.begin(); it!=mapGribRecords.end(); it++)
    {
        std::vector<GribRecord *> *ls = (*it).second;
        for (zuint i=0; i<ls->size(); i++)
        {
            time_t cur=ls->at(i)->getRecordCurrentDate();
            if(minDate==-1)
                minDate=cur;
            else if(minDate > cur)
                    minDate=cur;
            if(maxDate==-1)
                maxDate = cur;
            else
                if(maxDate < cur)
                    maxDate=cur;
            setAllDates.insert( cur );
        }
    }
}

/* Plot */
//--------------------------------------------------------------------------
QRgb Grib::getWindColor(double v, bool smooth)
{
    QRgb rgb = 0;
    int beauf;
    if (! smooth) {
        beauf = Util::kmhToBeaufort(v);
        rgb = windColor[beauf].rgba();
    }
    else {
        // Interpolation de couleur
        double fbeauf = Util::kmhToBeaufort_F(v);
        QColor c1 = windColor[ (int) fbeauf ];
        QColor c2 = windColor[ (int) fbeauf +1 ];
        double dcol = fbeauf-floor(fbeauf);
        rgb = qRgba(
                (int)( c1.red()  *(1.0-dcol) + dcol*c2.red()   +0.5),
                (int)( c1.green()*(1.0-dcol) + dcol*c2.green() +0.5),
                (int)( c1.blue() *(1.0-dcol) + dcol*c2.blue()  +0.5),
                mapColorTransp
                );
    }
    return rgb;
}

//--------------------------------------------------------------------------
// Carte de couleurs du vent
//--------------------------------------------------------------------------
void Grib::draw_WIND_Color(QPainter &pnt, const Projection *proj, bool smooth,
                               bool showWindColorMap, bool showWindArrows,bool barbules)
{

    int i, j, k, l;
    double u,v,x,y;
    double x0,y0, x1,y1;
    double * u_tab=NULL, * v_tab=NULL;
    bool * y_tab=NULL;
    int W = proj->getW();
    int H = proj->getH();
    int space=0;
    int W_s=0,H_s=0;
    QRgb   rgb;
    QImage *image= new QImage(W,H,QImage::Format_ARGB32);

    GribRecord *recU1,*recV1,*recU2,*recV2;
    time_t t1,t2;

    if (!ok) {
        return;
    }

    if(!getInterpolationParam(currentDate,&t1,&t2,&recU1,&recV1,&recU2,&recV2))
        return;

    if(showWindArrows)
    {
        if (barbules)
            space =  windBarbuleSpace;
        else
            space =  windArrowSpace;

        W_s=W/space+1;
        H_s=H/space+1;

        u_tab = new double[W_s*H_s];
        v_tab = new double[W_s*H_s];
        y_tab = new bool[W_s*H_s];
    }

    /* draw cover zone */
    if (getZoneExtension(&x0,&y0, &x1,&y1))
    {
        pnt.setPen(QColor(120, 120,120));
        pnt.setBrush(QColor(255,255,255,40));
        proj->map2screen(x0,y0, &i, &j);
        proj->map2screen(x1,y1, &k, &l);
        pnt.drawRect(i, j, k-i, l-j);
        proj->map2screen(x0-360.0,y0, &i, &j);
        proj->map2screen(x1-360.0,y1, &k, &l);
        pnt.drawRect(i, j, k-i, l-j);
    }

    if(showWindColorMap)
        image->fill( qRgba(0,0,0,0));

    for (i=0; i<W-1; i+=2)
    {
        for (j=0; j<H-1; j+=2)
        {
            proj->screen2map(i,j, &x, &y);
            if(getInterpolatedValue_byDates(x,y,currentDate,t1,t2,recU1,recV1,recU2,recV2,&u,&v))
            {
                if(showWindArrows && i%space==0 && j%space==0)
                {
                    int i_s=i/space;
                    int j_s=j/space;
                    u_tab[i_s*H_s+j_s]=u;
                    v_tab[i_s*H_s+j_s]=v;
                    y_tab[i_s*H_s+j_s]=(y<0);
                }
                if(showWindColorMap)
                {
                    rgb=getWindColor(u, smooth);
                    image->setPixel(i,  j,rgb);
                    image->setPixel(i+1,j,rgb);
                    image->setPixel(i,  j+1,rgb);
                    image->setPixel(i+1,j+1,rgb);
                }
            }
            else
            {
                if(showWindArrows && i%space==0 && j%space==0)
                {
                    int i_s=i/space;
                    int j_s=j/space;
                    u_tab[i_s*H_s+j_s]=-1;
                }
            }
        }
    }

    if(showWindColorMap)
        pnt.drawImage(0,0,*image);

    delete image;


    if(showWindArrows)
    {
        for (i=0; i<W_s; i++)
        {
            for (j=0; j<H_s; j++)
            {
                u=u_tab[i*H_s+j];
                v=v_tab[i*H_s+j];

                if(u==-1)
                    continue;
                if (barbules)
                    drawWindArrowWithBarbs(pnt, i*space,j*space, u,v, y_tab[i*H_s+j]);
                else
                    drawWindArrow(pnt, i*space,j*space, v);

            }
        }
        delete u_tab;
        delete v_tab;
        delete y_tab;
    }
}

void Grib::drawCartouche(QPainter &pnt)
{
    if (!ok) return;
    int fSize=12;
    QFont fontbig("TypeWriter", fSize, QFont::Bold, false);
    fontbig.setStyleHint(QFont::TypeWriter);
    fontbig.setStretch(QFont::Condensed);
    QColor   transpcolor(255,255,255,120);
    QColor   textcolor(20,20,20,255);
    pnt.setBrush(transpcolor);
    pnt.setFont(fontbig);
    pnt.setPen(transpcolor);
    pnt.drawRect(3,3,190,fSize+3+4);
    pnt.setPen(textcolor);
    pnt.drawText(10, fSize+6, Util::formatDateTimeLong(currentDate));// forecast validity date
}

//-----------------------------------------------------------------------------
void Grib::drawTransformedLine( QPainter &pnt,
        double si, double co,int di, int dj, int i,int j, int k,int l)
{
    int ii, jj, kk, ll;
    ii = (int) (i*co-j*si +0.5) + di;
    jj = (int) (i*si+j*co +0.5) + dj;
    kk = (int) (k*co-l*si +0.5) + di;
    ll = (int) (k*si+l*co +0.5) + dj;
    // Clip forcé �  cause d'un bug qpixmap sous windows
    int w = pnt.device()->width();
    int h = pnt.device()->height();
    if (       Util::isInRange(ii, 0, w)
            && Util::isInRange(kk, 0, w)
            && Util::isInRange(jj, 0, h)
            && Util::isInRange(ll, 0, h) )
        pnt.drawLine(ii, jj, kk, ll);
}
//-----------------------------------------------------------------------------
void Grib::drawWindArrow(QPainter &pnt, int i, int j, double ang)
{
    ang-=PI_2;
    double si=sin(ang),  co=cos(ang);
    QPen pen( QColor(255, 255, 255));
    pen.setWidth(2);
    pnt.setPen(pen);
    drawTransformedLine(pnt, si,co, i-windArrowSize/2,j,  0,0, windArrowSize, 0);   // hampe
    drawTransformedLine(pnt, si,co, i-windArrowSize/2,j,  0,0,  5, 2);   // flèche
    drawTransformedLine(pnt, si,co, i-windArrowSize/2,j,  0,0,  5,-2);   // flèche
}

//-----------------------------------------------------------------------------
void Grib::drawWindArrowWithBarbs(
                        QPainter &pnt,
                        int i, int j, double vkn, double ang,
                        bool south)
{
    ang-=PI_2;
    double si=sin(ang),  co=cos(ang);

    QPen pen( QColor(255,255,255));
    pen.setWidth(2);
    pnt.setPen(pen);
    pnt.setBrush(Qt::NoBrush);

    if (vkn < 1)
    {
        int r = 5;     // vent très faible, dessine un cercle
        pnt.drawEllipse(i-r,j-r,2*r,2*r);
    }
    else {
        // Flèche centrée sur l'origine
        int dec = -windBarbuleSize/2;
        drawTransformedLine(pnt, si,co, i,j,  dec,0,  dec+windBarbuleSize, 0);   // hampe
        drawTransformedLine(pnt, si,co, i,j,  dec,0,  dec+5, 2);    // flèche
        drawTransformedLine(pnt, si,co, i,j,  dec,0,  dec+5, -2);   // flèche

                int b1 = dec+windBarbuleSize -4;  // position de la 1ère barbule
                if (vkn >= 7.5  &&  vkn < 45 ) {
                        b1 = dec+windBarbuleSize;  // position de la 1ère barbule si >= 10 noeuds
                }

        if (vkn < 7.5) {  // 5 ktn
            drawPetiteBarbule(pnt,south, si,co, i,j, b1);
        }
        else if (vkn < 12.5) { // 10 ktn
            drawGrandeBarbule(pnt,south, si,co, i,j, b1);
        }
        else if (vkn < 17.5) { // 15 ktn
            drawGrandeBarbule(pnt,south, si,co, i,j, b1);
            drawPetiteBarbule(pnt,south, si,co, i,j, b1-4);
        }
        else if (vkn < 22.5) { // 20 ktn
            drawGrandeBarbule(pnt,south, si,co, i,j, b1);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-4);
        }
        else if (vkn < 27.5) { // 25 ktn
            drawGrandeBarbule(pnt,south, si,co, i,j, b1);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-4);
            drawPetiteBarbule(pnt,south, si,co, i,j, b1-8);
        }
        else if (vkn < 32.5) { // 30 ktn
            drawGrandeBarbule(pnt,south, si,co, i,j, b1);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-4);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-8);
        }
        else if (vkn < 37.5) { // 35 ktn
            drawGrandeBarbule(pnt,south, si,co, i,j, b1);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-4);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-8);
            drawPetiteBarbule(pnt,south, si,co, i,j, b1-12);
        }
        else if (vkn < 45) { // 40 ktn
            drawGrandeBarbule(pnt,south, si,co, i,j, b1);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-4);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-8);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-12);
        }
        else if (vkn < 55) { // 50 ktn
            drawTriangle(pnt,south, si,co, i,j, b1-4);
        }
        else if (vkn < 65) { // 60 ktn
            drawTriangle(pnt,south, si,co, i,j, b1-4);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-8);
        }
        else if (vkn < 75) { // 70 ktn
            drawTriangle(pnt,south, si,co, i,j, b1-4);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-8);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-12);
        }
        else if (vkn < 85) { // 80 ktn
            drawTriangle(pnt,south, si,co, i,j, b1-4);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-8);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-12);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-16);
        }
        else { // > 90 ktn
            drawTriangle(pnt,south, si,co, i,j, b1-4);
            drawTriangle(pnt,south, si,co, i,j, b1-12);
        }
    }
}
//---------------------------------------------------------------
void Grib::drawPetiteBarbule(QPainter &pnt, bool south,
                    double si, double co, int di, int dj, int b)
{
    if (south)
        drawTransformedLine(pnt, si,co, di,dj,  b,0,  b+2, -5);
    else
        drawTransformedLine(pnt, si,co, di,dj,  b,0,  b+2, 5);
}
//---------------------------------------------------------------
void Grib::drawGrandeBarbule(QPainter &pnt, bool south,
                    double si, double co, int di, int dj, int b)
{
    if (south)
        drawTransformedLine(pnt, si,co, di,dj,  b,0,  b+4,-10);
    else
        drawTransformedLine(pnt, si,co, di,dj,  b,0,  b+4,10);
}
//---------------------------------------------------------------
void Grib::drawTriangle(QPainter &pnt, bool south,
                    double si, double co, int di, int dj, int b)
{
    if (south) {
        drawTransformedLine(pnt, si,co, di,dj,  b,0,  b+4,-10);
        drawTransformedLine(pnt, si,co, di,dj,  b+8,0,  b+4,-10);
    }
    else {
        drawTransformedLine(pnt, si,co, di,dj,  b,0,  b+4,10);
        drawTransformedLine(pnt, si,co, di,dj,  b+8,0,  b+4,10);
    }
}
