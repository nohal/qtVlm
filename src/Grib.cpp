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

#include <complex>
#include <math.h>

#include <cassert>
#include <QDebug>
#include <QVector>

#include "Grib.h"
#include "Util.h"
#include "dataDef.h"
#include "GribRecord.h"
#include "Projection.h"
#include "IsoLine.h"
#include "settings.h"
#include "DataManager.h"

#include "GribV1.h"
#include "GribV2.h"
#include "GribV1Record.h"
#include "GribV2Record.h"
#include <QMap>

#include "interpolation.h"

Grib::Grib(DataManager * dataManager) {

    this->dataManager=dataManager;

    version=0;
    fileName="";

    dewpointDataStatus = NO_DATA_IN_FILE;
}

Grib::~Grib() {
    if(ok)
        clean_all_vectors();
    Util::cleanListPointers(listIsobars);
    Util::cleanListPointers(listIsotherms0);
}

Grib * Grib::loadGrib(QString fileName,DataManager *dataManager) {
    Grib * grib=NULL;
    /* first try to find grib version */
    if(GribV2::isGribV2(fileName)) {
        GribV2 * ptr=new GribV2(dataManager);
        ptr->loadFile(fileName);
        grib=(Grib*)ptr;
    }
    else if(GribV1::isGribV1(fileName)) {
        GribV1 * ptr=new GribV1(dataManager);
        ptr->loadFile(fileName);
        grib=(Grib*)ptr;
    }

    return grib;
}

/**************************
 * Vector data management *
 **************************/

void Grib::clean_all_vectors() {
    std::map <long int, QMap<time_t,GribRecord *>* >::iterator it;
    for (it=mapGribRecords.begin(); it!=mapGribRecords.end(); it++) {
        QMap<time_t,GribRecord *>  *ls = (*it).second;
        clean_vector( ls );
        delete ls;
    }
    mapGribRecords.clear();
}

void Grib::clean_vector(QMap<time_t, GribRecord *> *ls) {
    while(!ls->isEmpty())
        delete ls->take(ls->begin().key());
}

void Grib::addRecord(GribRecord * rec) {
    std::map <long int, QMap<time_t,GribRecord *>* >::iterator it;
    it = mapGribRecords.find(rec->get_dataKey());
    if (it == mapGribRecords.end()) { /* map doesn't contain this data type */
            mapGribRecords[rec->get_dataKey()] = new QMap<time_t,GribRecord *>;
    }
    /* adds the record to vector created for record's data type */
    mapGribRecords[rec->get_dataKey()]->insert(rec->get_curDate(),rec);
}

bool Grib::hasData(int dataType,int levelType,int levelValue) {
    return getNumberOfGribRecords(dataType,levelType,levelValue)!=0;
}

QMap<time_t,GribRecord *> * Grib::getFirstNonEmptyList() {
        QMap<time_t,GribRecord *> *ls = NULL;
        std::map <long int, QMap<time_t,GribRecord *>* >::iterator it;
        for (it=mapGribRecords.begin(); ls==NULL && it!=mapGribRecords.end(); it++)
        {
                if ((*it).second->size()>0)
                        ls = (*it).second;
        }
        return ls;
}

QMap<time_t,GribRecord *> * Grib::getListOfGribRecords(int dataType,int levelType,int levelValue) {
        long int key = GribRecord::makeKey(dataType,levelType,levelValue);
        if (mapGribRecords.find(key) != mapGribRecords.end())
                return mapGribRecords[key];
        else
                return NULL;
}

int Grib::getNumberOfGribRecords(int dataType,int levelType,int levelValue)
{
    QMap<time_t,GribRecord *> *liste = getListOfGribRecords(dataType,levelType,levelValue);
    if (liste != NULL)
        return (int)liste->size();
    else
        return 0;
}

GribRecord * Grib::getGribRecord(int dataType,int levelType,int levelValue, time_t date)
{
    QMap<time_t,GribRecord *> *ls = getListOfGribRecords(dataType,levelType,levelValue);
    if (ls != NULL) {
        // Cherche le premier enregistrement a la bonne date
        GribRecord *res = NULL;
        int nb = (int)ls->size();
        for (int i=0; i<nb && res==NULL; i++) {
            if ((*ls)[i]->get_curDate() == date)
                res = (*ls)[i];
        }
        return res;
    }
    else {
        return NULL;
    }
}

GribRecord * Grib::getFirstRecord(void) {
    if(isOk()) {
        return getFirstNonEmptyList()->begin().value();
    }
    else
        return NULL;
}

/****************************
 * Rectangular covering zone*
 ****************************/
bool Grib::getZoneExtension(double *x0,double *y0, double *x1,double *y1)
{
    if(!isOk())
        return false;

    QMap<time_t,GribRecord *> *ls = getFirstNonEmptyList();
    if (ls != NULL) {
        GribRecord *rec = ls->begin().value();
        if (rec != NULL) {
            if(rec->get_isFull())
                return false;

            *x0 = rec->getX(0);
            *x1 = rec->getX( rec->get_Ni()-1 );
            *y0 = rec->getY(0);
            *y1 = rec->getY( rec->get_Nj()-1 );
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

/**************************
 * DewPoint computation   *
 **************************/

void Grib::createDewPointData(void) {
    //-----------------------------------------------------
    // Are dewpoint data in file ?
    // If no, compute it with Magnus-Tetens formula, if possible.
    //-----------------------------------------------------
    dewpointDataStatus = DATA_IN_FILE;
    if (getNumberOfGribRecords(DATA_DEWPOINT, DATA_LV_ABOV_GND, 2) == 0) {
        dewpointDataStatus = NO_DATA_IN_FILE;
        if (  getNumberOfGribRecords(DATA_HUMID_REL, DATA_LV_ABOV_GND, 2) > 0
              && getNumberOfGribRecords(DATA_TEMP, DATA_LV_ABOV_GND, 2) > 0) {
            dewpointDataStatus = COMPUTED_DATA;
            std::set<time_t>::iterator iter;
            std::set<time_t> * dateList = dataManager->get_dateList();
            for (iter=dateList->begin(); iter!=dateList->end(); ++iter) {
                time_t date = *iter;
                GribRecord *recModel = getGribRecord(DATA_TEMP,DATA_LV_ABOV_GND,2,date);
                if (recModel != NULL) {
                    // Crée un GribRecord avec les dewpoints calcules
                    GribRecord *recDewpoint;
                    if(version == 1) {
                        GribV1Record * rec=(GribV1Record *)recModel;
                        recDewpoint = new GribV1Record(*rec);
                    }
                    else if(version == 2 ) {
                        GribV2Record * rec=(GribV2Record *)recModel;
                        recDewpoint = new GribV2Record(*rec);
                    }

                    if (recDewpoint != NULL) {

                        recDewpoint->set_dataType(DATA_DEWPOINT);
                        for (int i=0; i<(int)recModel->get_Ni(); i++)
                            for (int j=0; j<(int)recModel->get_Nj(); j++) {
                                double x = recModel->getX(i);
                                double y = recModel->getY(j);
                                double dp = computeDewPoint(x, y, date);
                                recDewpoint->setValue(i, j, dp);
                        }
                        addRecord(recDewpoint);
                    }
                }
            }
        }
    }
}

double Grib::computeDewPoint(double lon, double lat, time_t now)
{
    double diewpoint = GRIB_NOTDEF;

    GribRecord *recTempDiew =  getGribRecord(DATA_DEWPOINT,DATA_LV_ABOV_GND,2,now);
    if (recTempDiew != NULL) {
        // GRIB file contains diew point data
        diewpoint = recTempDiew->getInterpolatedValue(lon, lat);
    }
    else {
        // Compute diew point with Magnus-Tetens formula
        GribRecord *recTemp =  getGribRecord(DATA_TEMP,DATA_LV_ABOV_GND,2,now);
        GribRecord *recHumid = getGribRecord(DATA_HUMID_REL,DATA_LV_ABOV_GND,2,now);
        if (recTemp && recHumid) {
            double temp = recTemp->getInterpolatedValue(lon, lat);
            double humid = recHumid->getInterpolatedValue(lon, lat);
            if (temp != GRIB_NOTDEF && humid != GRIB_NOTDEF) {
                double a = 17.27;
                double b = 237.7;
                double t  = temp-273.15;
                double rh = humid;
                double alpha = a*t/(b+t)+log(rh/100.0);
                diewpoint = b*alpha/(a-alpha);
                diewpoint += 273.15;
            }
        }
    }
    return diewpoint;
}

/**************************
 * Date of GribRecord     *
 **************************/

void Grib::update_dateList(std::set<time_t> * dateList) {
    if(!isOk()) return;
    std::map <long int, QMap<time_t,GribRecord *>* >::iterator it;
    for (it=mapGribRecords.begin(); it!=mapGribRecords.end(); it++) {
        QMap<time_t,GribRecord *> *ls = (*it).second;
        for(int i=0;i<ls->keys().size();++i)
            dateList->insert(ls->keys().at(i));
    }
}

/********************************************************
 * Build map of all level associate to a given dataType *
 ********************************************************/

void Grib::update_levelMap(QMap<int,QMap<int,QList<int>*>*> * levelMap) {
    if(!isOk()) return;
    std::map <long int, QMap<time_t,GribRecord *>* >::iterator it;
    for (it=mapGribRecords.begin(); it!=mapGribRecords.end(); it++) {
        QMap<time_t,GribRecord *> *ls = (*it).second;

        if(ls->size()>0) {
            GribRecord * ptr=ls->begin().value();
            if(!levelMap->contains(ptr->get_dataType())) {
                levelMap->insert(ptr->get_dataType(),new QMap<int,QList<int>*>());
            }

            if(!levelMap->value(ptr->get_dataType())->contains(ptr->get_levelType()))
                levelMap->value(ptr->get_dataType())->insert(ptr->get_levelType(),new QList<int>());

            levelMap->value(ptr->get_dataType())->value(ptr->get_levelType())->append(ptr->get_levelValue());
        }
    }
}

/*****************************
 * Get records arround date  *
 *****************************/

void Grib::find_recordsAroundDate (int dataType,int levelType,int levelValue, time_t date,
                                                        GribRecord **before, GribRecord **after) {
    if(!before || !after)
        return;
    QMap<time_t,GribRecord *> *ls = getListOfGribRecords(dataType,levelType,levelValue);

    *before = NULL;
    *after  = NULL;

    if(ls==NULL)
        return;
    *before=ls->lowerBound(date).value();
    *after=ls->upperBound(date).value();

//    zuint nb = (int)ls->size();
//    for (zuint i=0; i<nb && /**before==NULL &&*/ *after==NULL; i++) {
//        GribRecord *rec = (*ls)[i];
//        if (rec->get_curDate() == date) {
//            *before = rec;
//            *after = rec;
//        }
//        else if (rec->get_curDate() < date) {
//            *before = rec;
//        }
//        else if (rec->get_curDate() > date  &&  *before != NULL) {
//            *after = rec;
//        }
//    }
}

bool Grib::get_recordsAndTime_2D(int dataType_1,int dataType_2,int levelType,int levelValue,
                                 time_t now,time_t * t1,time_t * t2,GribRecord ** recU1,GribRecord ** recV1,
                           GribRecord ** recU2,GribRecord ** recV2,bool debug) {
    if(t1 && t2 && recU1 && recV1 && recU2 && recV2) {
        find_recordsAroundDate(dataType_1,levelType,levelValue,now,recU1,recU2);
        find_recordsAroundDate(dataType_2,levelType,levelValue,now,recV1,recV2);
        if(*recU1 && *recV1) {
            if(*recU1==*recU2) {
                *t1=(*recU1)->get_curDate();
                *t2=*t1;
                *recU2=NULL;
                *recV2=NULL;
            }
            else {
                *t1=(*recU1)->get_curDate();
                if(*recU2!=NULL && *recV2!=0)
                    *t2=(*recU2)->get_curDate();
            }

            if(debug) {
                qWarning() << "Time: now=" << now << " , t1=" << *t1 << ", t2=" << *t2;
            }
            return true;
        }
    }
    return false;
}

bool Grib::get_recordsAndTime_1D(int dataType,int levelType,int levelValue,
                                time_t now,time_t * tPrev,time_t * tNxt,
                                GribRecord ** recPrev,GribRecord ** recNxt) {
    if(tPrev && tNxt && recPrev && recNxt) {
        find_recordsAroundDate(dataType,levelType,levelValue,now,recPrev,recNxt);
        if(*recPrev) {
            if(*recPrev==*recNxt) {
                *tPrev=(*recPrev)->get_curDate();
                *tNxt=*tPrev;
                *recNxt=NULL;
            }
            else {
                *tPrev=(*recPrev)->get_curDate();
                if(*recNxt!=NULL)
                    *tNxt=(*recNxt)->get_curDate();
                else
                    *tNxt=*tPrev;
            }
            return true;
        }
    }
    return false;
}

/**************************
 * Get interpolated value *
 **************************/

bool Grib::getInterpolatedValue_1D(int dataType,int levelType,int levelValue,
                                   double d_long, double d_lat, time_t now,double *res) {
    time_t tPrev,tNxt;
    GribRecord * recPrev;
    GribRecord * recNxt;
    if(!res) return false;
    *res=0;
    if(get_recordsAndTime_1D(dataType,levelType,levelValue,now,&tPrev,&tNxt,&recPrev,&recNxt)) {
        return interpolateValue_1D(d_long,d_lat,now,tPrev,tNxt,recPrev,recNxt,res);
    }
    else
        return false;

}

bool Grib::interpolateValue_1D(double d_long, double d_lat, time_t now,time_t tPrev,time_t tNxt,
                                    GribRecord * recPrev,GribRecord * recNxt,double * res) {
    if(!res) return false;
    *res=0;
    double v = recPrev->getInterpolatedValue(d_long, d_lat, MUST_INTERPOLATE_VALUE);
    double v_2;
    if(v != GRIB_NOTDEF && tPrev!=tNxt)
    {
        v_2=recNxt->getInterpolatedValue(d_long, d_lat, MUST_INTERPOLATE_VALUE);
        if(v_2 != GRIB_NOTDEF) {
            *res= v+((v_2-v)/((double)(tNxt-tPrev)))*((double)(now-tPrev));
            return true;
        }
        else
            return false;
    }
    else
        return false;
}

bool Grib::getInterpolatedValue_2D(int dataType1, int dataType2, int levelType, int levelValue,
                                   double d_long, double d_lat, time_t now, double * u, double * v,
                                   int interpolation_type, bool UV, bool debug) {
    GribRecord *recU1,*recV1,*recU2,*recV2;
    time_t t1,t2;

    if(interpolation_type==INTERPOLATION_UKN)
        interpolation_type=dataManager->get_interpolationMode();

    if(u) *u=0;
    if(v) *v=0;

    if(!isOk())
        return false;

    if(get_recordsAndTime_2D(dataType1,dataType2,levelType,levelValue,now,&t1,&t2,&recU1,&recV1,&recU2,&recV2,debug)) {
        if(debug)
            qWarning() << "Param ok => go interpolation";
        return interpolateValue_2D(d_long,d_lat,now,t1,t2,recU1,recV1,recU2,recV2,u,v,interpolation_type,UV,debug);
    }
    return false;
}

bool Grib::interpolateValue_2D(double d_long, double d_lat, time_t now, time_t t1,time_t t2,
                                              GribRecord *recU1,GribRecord *recV1,GribRecord *recU2,GribRecord *recV2,
                                              double * u, double * v,int interpolation_type,bool UV,bool debug) {
    windData wData_prev;
    windData wData_nxt;
    double gridOriginLat_1,gridOriginLon_1,gridOriginLat_2,gridOriginLon_2;

    bool hasNxt=false;
    double gribStep_t1_lon=1,gribStep_t2_lon=1;
    double gribStep_t1_lat=1,gribStep_t2_lat=1;

    /*sanity check */
    if(!u || !v || !recU1 || !recV1)
        return false;


    gridOriginLat_1=recV1->get_latMin();
    gridOriginLon_1=recV1->get_lonMin();

    //isHighRes_t1=(recU1->get_Di()==0.5 || recU1->get_Di()==-0.5)?1:0;
    gribStep_t1_lon=recU1->get_Di()==0?1:recU1->get_Di();
    gribStep_t1_lat=recU1->get_Dj()==0?1:recU1->get_Dj();

    if(!recU1->getValue_TWSA(d_long,d_lat,&(wData_prev.u0),&(wData_prev.u1),&(wData_prev.u2),&(wData_prev.u3),debug))
        return false;
    if(!recV1->getValue_TWSA(d_long,d_lat,&(wData_prev.v0),&(wData_prev.v1),&(wData_prev.v2),&(wData_prev.v3),debug))
        return false;

    if(recU2 && recV2) {
        hasNxt=true;
        gridOriginLat_2=recV2->get_latMin();
        gridOriginLon_2=recV2->get_lonMin();
        //isHighRes_t2=(recU2->get_Di()==0.5 || recU2->get_Di()==-0.5)?1:0;
        gribStep_t2_lon=recU2->get_Di()==0?1:recU2->get_Di();
        gribStep_t2_lat=recU2->get_Dj()==0?1:recU2->get_Dj();

        if(!recU2->getValue_TWSA(d_long,d_lat,&(wData_nxt.u0),(&wData_nxt.u1),&(wData_nxt.u2),&(wData_nxt.u3),debug))
            return false;
        if(!recV2->getValue_TWSA(d_long,d_lat,&(wData_nxt.v0),(&wData_nxt.v1),&(wData_nxt.v2),&(wData_nxt.v3),debug))
            return false;
    }
    gribStep_t1_lon=qAbs(gribStep_t1_lon);
    gribStep_t2_lon=qAbs(gribStep_t2_lon);
    gribStep_t1_lat=qAbs(gribStep_t1_lat);
    gribStep_t2_lat=qAbs(gribStep_t2_lat);
    switch(interpolation_type) {
        case INTERPOLATION_TWSA:
            if(debug)
                qWarning() << "Interpolation TWSA";
            interpolation::get_wind_info_latlong_TWSA(d_long,d_lat,now,t1,t2,
                                                      &wData_prev,(hasNxt?(&wData_nxt):NULL),
                                                      gribStep_t1_lat,gribStep_t1_lon,gribStep_t2_lat,gribStep_t2_lon,
                                                      u,v,UV,debug);
            break;
        case INTERPOLATION_SELECTIVE_TWSA:
            if(debug)
                qWarning() << "Interpolation selective-TWSA";
            interpolation::get_wind_info_latlong_selective_TWSA(d_long,d_lat,now,t1,t2,
                                                                &wData_prev,(hasNxt?(&wData_nxt):NULL),
                                                                gribStep_t1_lat,gribStep_t1_lon,gribStep_t2_lat,gribStep_t2_lon,
                                                                u,v,UV,debug);
            break;
        case INTERPOLATION_HYBRID:
            if(debug)
                qWarning() << "Interpolation Hybrid";
            interpolation::get_wind_info_latlong_hybrid(d_long,d_lat,now,t1,t2,
                                                        &wData_prev,(hasNxt?(&wData_nxt):NULL),
                                                        gribStep_t1_lat,gribStep_t1_lon,gribStep_t2_lat,gribStep_t2_lon,
                                                        u,v,gridOriginLat_1,gridOriginLon_1,gridOriginLat_2,gridOriginLon_2,UV,debug);
            break;
         default:
            if(debug)
                qWarning() << "NO interpolation defined";
            return false;
     }
    return true;
}



/*************************************************
 *     Isolines                                  *
 *************************************************/

void Grib::init_isos(time_t t) {
    init_isoBars(t);
    init_isoTherms0(t);
}

void Grib::init_isoBars(time_t t) {
    if (!ok)
        return;

    int step=dataManager->get_isoBarsStep();

    Util::cleanListPointers(listIsobars);

    GribRecord *rec_prev,*rec_nxt;
    time_t tPrev,tNxt;
    if(get_recordsAndTime_1D(DATA_PRESSURE,DATA_LV_MSL,0,t,
                                 &tPrev,&tNxt,&rec_prev,&rec_nxt))
    {
        Util::cleanListPointers(listIsobars);
        IsoLine *iso;
        for (double press=840; press<1120; press += step)
        {
                iso = new IsoLine(press*100, t, tPrev,tNxt, rec_prev,rec_nxt);
                listIsobars.push_back(iso);
        }
    }
}

void Grib::init_isoTherms0(time_t t) {
    if (!ok)
        return;

    int step=dataManager->get_isoTherms0Step();

    Util::cleanListPointers(listIsotherms0);

    GribRecord *rec_prev,*rec_nxt;
    time_t tPrev,tNxt;
    if(get_recordsAndTime_1D(DATA_GEOPOT_HGT,DATA_LV_ISOTHERM0,0,t,
                                 &tPrev,&tNxt,&rec_prev,&rec_nxt))
    {
        Util::cleanListPointers(listIsotherms0);
        IsoLine *iso;
        for (double alt=0; alt<12000; alt += step)
        {
                iso = new IsoLine(alt, t, tPrev,tNxt, rec_prev,rec_nxt);
                listIsotherms0.push_back(iso);
        }
    }
}


