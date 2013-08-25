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

Original code: zyGrib: meteorological GRIB file viewer
Copyright (C) 2008 - Jacques Zaninetti - http://zygrib.free.fr

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

#include "interpolation.h"


#ifdef __QTVLM_WITH_TEST
int nbWarning;
#endif

//-------------------------------------------------------------------------------
Grib::Grib() {
    ok = false;
    isCurrentGrib=false;
#ifdef __QTVLM_WITH_TEST
    nbWarning=0;
#endif

    isobarsStep = Settings::getSetting("isobarsStep", 2).toDouble();
    isotherms0Step = 50;

    QString interpol_name[4] = { "UKN", "TWSA", "selecive TWSA", "Hybride" };
    interpolation_param = INTERPOLATION_DEFAULT;
    qWarning() << "Starting with interpolation: " << interpol_name[interpolation_param];

    mustInterpolateValues = MUST_INTERPOLATE_VALUE;

    dewpointDataStatus = NO_DATA_IN_FILE;

    file=NULL;
    load_forcedParam();
}

void Grib::load_forcedParam(void) {
    forceWind=Settings::getSetting("forceWind",0).toInt()==1;
    forcedTWS=Settings::getSetting("forcedTWS",0.0).toDouble();
    forcedTWD=Settings::getSetting("forcedTWD",0.0).toDouble();
    forceCurrents=Settings::getSetting("forceCurrents",0).toInt()==1;
    forcedCS=Settings::getSetting("forcedCS",0.0).toDouble();
    forcedCD=Settings::getSetting("forcedCD",0.0).toDouble();
}

//-------------------------------------------------------------------------------
Grib::~Grib() {
    if(ok)
        clean_all_vectors();
    Util::cleanListPointers(listIsobars);
    Util::cleanListPointers(listIsotherms0);
}

void Grib::setCurrentDate(time_t t)
{
    currentDate = t;
    Util::cleanListPointers(listIsobars);
    Util::cleanListPointers(listIsotherms0);
    initIsobars();
    initIsotherms0();
}

void Grib::setIsobarsStep(double step)
{
    isobarsStep = step;
    initIsobars();
}
//--------------------------------------------------------------------------
void Grib::setIsotherms0Step(double step)
{
    isotherms0Step = step;
    initIsotherms0();
}

void Grib::loadGribFile(QString fileName)
{
//    QTime td;
//    td.start();
    this->fileName = fileName;
    file=NULL;
    Util::cleanListPointers(listIsobars);
    Util::cleanListPointers(listIsotherms0);

    ok=false;
    fname = qPrintable(fileName);
    if (fname != "")
    {
        debug("Open file: %s", fname.c_str());
        clean_all_vectors();
        //--------------------------------------------------------
        // Ouverture du fichier
        //--------------------------------------------------------
        QList<int> compressModes;
        compressModes.append(ZU_COMPRESS_AUTO);
        compressModes.append(ZU_COMPRESS_BZIP);
        compressModes.append(ZU_COMPRESS_GZIP);
        compressModes.append(ZU_COMPRESS_NONE);

        bool found=false;
        int compressMode;
        for(compressMode=0;compressMode<compressModes.count();++compressMode)
        {
            if(file!=NULL)
                zu_close(file);
            file = zu_open(fname.c_str(), "rb", compressModes.at(compressMode));
            found=findCompression();
            if (found) break;
        }
        if(!found || file==NULL)
        {
            if(file!=NULL)
                zu_close(file);
            file=NULL;
            qWarning()<<"Unable to find a suitable compression mode";
            return;
        }
        zu_rewind(file);
        readGribFileContent();
        setCurrentDate ( setAllDates.empty() ? 0: *(setAllDates.begin()));
        if (file != NULL)
            zu_close(file);
        file=NULL;
    }
//    qWarning()<<"time to load grib:"<<td.elapsed();
}
bool Grib::findCompression()
{
    if(file==NULL) return false;
    char buf[1];
    memset (buf, 0, sizeof (buf));
    int fileOffset0=0;
    bool found=false;
    while(fileOffset0<100)
    {
        if(zu_read(file,buf,1)!=1) break;
        ++fileOffset0;
        if(buf[0]!='G')
            continue;
        if(zu_read(file,buf,1)!=1) break;
        ++fileOffset0;
        if(buf[0]!='R')
        {
            if(buf[0]=='G')
                zu_seek(file,--fileOffset0,SEEK_SET);
            continue;
        }
        if(zu_read(file,buf,1)!=1) break;
        ++fileOffset0;
        if(buf[0]!='I')
        {
            if(buf[0]=='G')
                zu_seek(file,--fileOffset0,SEEK_SET);
            continue;
        }
        if(zu_read(file,buf,1)!=1) break;
        ++fileOffset0;
        if(buf[0]!='B')
        {
            if(buf[0]=='G')
                zu_seek(file,--fileOffset0,SEEK_SET);
            continue;
        }
        found=true;
        break;
    }
    //qWarning()<<"fileOffset="<<fileOffset0;
    return found;
}

//-------------------------------------------------------------------------------
void Grib::clean_all_vectors()
{
        std::map <long int, std::vector<GribRecord *>* >::iterator it;
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
    for (it=ls.begin(); it!=ls.end(); ++it) {
        delete *it;
        *it = NULL;
    }
    ls.clear();
}

//---------------------------------------------------------------------------------
void Grib::storeRecordInMap(GribRecord *rec)
{
        std::map <long int, std::vector<GribRecord *>* >::iterator it;
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
    // et stockage dans les listes appropriees.
    //--------------------------------------------------------
    GribRecord *rec;
    int id = 0;
    time_t firstdate = -1;

    bool recAdded;

    while(true) {
        ++id;
        rec = new GribRecord(file, id);

        recAdded=false;

        if(!rec || rec->isEof()) {
            if(rec) {
                delete rec;
                rec=NULL;
            }
            --id;
            break;
        }

        if (rec->isOk())
        {
            if (rec->isDataKnown())
            {
                ok = true;   // au moins 1 record ok

                if (firstdate== -1)
                    firstdate = rec->getRecordCurrentDate();


                if (//-----------------------------------------
                        (rec->getDataType()==GRB_PRESSURE
                         && rec->getLevelType()==LV_MSL && rec->getLevelValue()==0)
                        //-----------------------------------------
                        || ( (rec->getDataType()==GRB_TMIN || rec->getDataType()==GRB_TMAX)
                             && rec->getLevelType()==LV_ABOV_GND && rec->getLevelValue()==2)
                        //-----------------------------------------
                        || (rec->getDataType()==GRB_TEMP
                            && rec->getLevelType()==LV_ABOV_GND && rec->getLevelValue()==2)
                        || (rec->getDataType()==GRB_TEMP
                            && rec->getLevelType()==LV_ISOBARIC
                            && (   rec->getLevelValue()==850
                                   || rec->getLevelValue()==700
                                   || rec->getLevelValue()==500
                                   || rec->getLevelValue()==300
                                   || rec->getLevelValue()==200 ) )
                        //-----------------------------------------
                        // Wind
                        //-----------------------------------------
                        || ( (rec->getDataType()==GRB_WIND_VX || rec->getDataType()==GRB_WIND_VY)
                             && rec->getLevelType()==LV_ABOV_GND
                             && (   rec->getLevelValue()==1
                                    || rec->getLevelValue()==2
                                    || rec->getLevelValue()==3
                                    || rec->getLevelValue()==10 ) )
                        || ( (rec->getDataType()==GRB_WIND_VX || rec->getDataType()==GRB_WIND_VY)
                             && rec->getLevelType()==LV_MSL
                             && rec->getLevelValue()==0 )
                        || ( (rec->getDataType()==GRB_WIND_VX || rec->getDataType()==GRB_WIND_VY)
                             && rec->getLevelType()==LV_GND_SURF
                             && (rec->getLevelValue()==0 || rec->getLevelValue()==1))
                        || ( (rec->getDataType()==GRB_WIND_VX || rec->getDataType()==GRB_WIND_VY)
                             && rec->getLevelType()==LV_ISOBARIC
                             && (   rec->getLevelValue()==850
                                    || rec->getLevelValue()==700
                                    || rec->getLevelValue()==500
                                    || rec->getLevelValue()==300
                                    || rec->getLevelValue()==200 ) )
                        || ( (rec->getDataType()==GRB_CURRENT_VX || rec->getDataType()==GRB_CURRENT_VY)
                             && rec->getLevelType()==LV_MSL
                             && rec->getLevelValue()==0 )
                        //-----------------------------------------
                        //-----------------------------------------
                        || (rec->getDataType()==GRB_HUMID_SPEC
                            && rec->getLevelType()==LV_ISOBARIC
                            && (   rec->getLevelValue()==850
                                   || rec->getLevelValue()==700
                                   || rec->getLevelValue()==500
                                   || rec->getLevelValue()==300
                                   || rec->getLevelValue()==200 ) )
                        //-----------------------------------------
                        || (rec->getDataType()==GRB_GEOPOT_HGT
                            && rec->getLevelType()==LV_ISOTHERM0 && rec->getLevelValue()==0)
                        || (rec->getDataType()==GRB_GEOPOT_HGT
                            && rec->getLevelType()==LV_ISOBARIC
                            && (   rec->getLevelValue()==850
                                   || rec->getLevelValue()==700
                                   || rec->getLevelValue()==500
                                   || rec->getLevelValue()==300
                                   || rec->getLevelValue()==200 ) )
                        //-----------------------------------------
                        || (rec->getDataType()==GRB_PRECIP_TOT
                            && rec->getLevelType()==LV_GND_SURF && rec->getLevelValue()==0)
                        //-----------------------------------------
                        || (rec->getDataType()==GRB_PRECIP_RATE
                            && rec->getLevelType()==LV_GND_SURF && rec->getLevelValue()==0)
                        //-----------------------------------------
                        || (rec->getDataType()==GRB_SNOW_DEPTH
                            && rec->getLevelType()==LV_GND_SURF && rec->getLevelValue()==0)
                        //-----------------------------------------
                        || (rec->getDataType()==GRB_SNOW_CATEG
                            && rec->getLevelType()==LV_GND_SURF && rec->getLevelValue()==0)
                        //-----------------------------------------
                        || (rec->getDataType()==GRB_FRZRAIN_CATEG
                            && rec->getLevelType()==LV_GND_SURF && rec->getLevelValue()==0)
                        //-----------------------------------------
                        || (rec->getDataType()==GRB_CLOUD_TOT
                            && rec->getLevelType()==LV_ATMOS_ALL && rec->getLevelValue()==0)
                        //-----------------------------------------
                        || (rec->getDataType()==GRB_HUMID_REL
                            && rec->getLevelType()==LV_ABOV_GND && rec->getLevelValue()==2)
                        || (rec->getDataType()==GRB_HUMID_REL
                            && rec->getLevelType()==LV_ISOBARIC
                            && (   rec->getLevelValue()==850
                                   || rec->getLevelValue()==700
                                   || rec->getLevelValue()==500
                                   || rec->getLevelValue()==300
                                   || rec->getLevelValue()==200 ) )
                        //-----------------------------------------
                        || (rec->getDataType()==GRB_TEMP_POT
                            && rec->getLevelType()==LV_SIGMA && rec->getLevelValue()==9950)
                        //-----------------------------------------
                        || (rec->getDataType()==GRB_CAPE
                            && rec->getLevelType()==LV_GND_SURF && rec->getLevelValue()==0)
                        )
                {
                    if(!isCurrentGrib || (rec->getDataType()==GRB_CURRENT_VX || rec->getDataType()==GRB_CURRENT_VY)) {
                        recAdded=true;
                        storeRecordInMap(rec);
                    }
                }
#if 0
                else
                {
                    qWarning()<<"GribReader: unknown record type: key="<<(int)rec->getKey();
                    qWarning()<<"dataType="<<rec->getDataType();
                    qWarning()<<"levelType"<<rec->getLevelType();
                    qWarning()<<"levelValue"<<rec->getLevelValue();
                    qWarning()<<"IdCenter="<<rec->getIdCenter();
                    qWarning()<<"IdModel="<<rec->getIdModel();
                    qWarning()<<"IdGrid="<<rec->getIdGrid();
                }
#endif
            }
        }

        if(!recAdded) {
            delete rec;
            rec=NULL;
            --id;
        }
    }
}


//---------------------------------------------------------------------------------
void Grib::readGribFileContent()
{
    fileSize = zu_filesize(file);
    readAllGribRecords();

    createListDates();
    hoursBetweenRecords = computeHoursBeetweenGribRecords();

    //-----------------------------------------------------
    // Are dewpoint data in file ?
    // If no, compute it with Magnus-Tetens formula, if possible.
    //-----------------------------------------------------
    dewpointDataStatus = DATA_IN_FILE;
    if (getNumberOfGribRecords(GRB_DEWPOINT, LV_ABOV_GND, 2) == 0)
    {
        dewpointDataStatus = NO_DATA_IN_FILE;
        if (  getNumberOfGribRecords(GRB_HUMID_REL, LV_ABOV_GND, 2) > 0
              && getNumberOfGribRecords(GRB_TEMP, LV_ABOV_GND, 2) > 0)
        {
            dewpointDataStatus = COMPUTED_DATA;
            std::set<time_t>::iterator iter;
            for (iter=setAllDates.begin(); iter!=setAllDates.end(); ++iter)
            {
                time_t date = *iter;
                GribRecord *recModel = getGribRecord(GRB_TEMP,LV_ABOV_GND,2,date);
                if (recModel != NULL)
                {
                    // Crée un GribRecord avec les dewpoints calcules
                    GribRecord *recDewpoint = new GribRecord(*recModel);
                    if (recDewpoint != NULL)
                    {
                        recDewpoint->setDataType(GRB_DEWPOINT);
                        for (zuint i=0; i<(zuint)recModel->getNi(); i++)
                            for (zuint j=0; j<(zuint)recModel->getNj(); j++)
                            {
                            double x = recModel->getX(i);
                            double y = recModel->getY(j);
                            double dp = computeDewPoint(x, y, date);
                            recDewpoint->setValue(i, j, dp);
                        }
                        storeRecordInMap(recDewpoint);
                    }
                }
            }
        }
    }
        //-----------------------------------------------------
}

double Grib::computeDewPoint(double lon, double lat, time_t now)
{
    double diewpoint = GRIB_NOTDEF;

    GribRecord *recTempDiew =  getGribRecord(GRB_DEWPOINT,LV_ABOV_GND,2,now);
    if (recTempDiew != NULL)
    {
        // GRIB file contains diew point data
        diewpoint = recTempDiew->getInterpolatedValue(lon, lat);
    }
    else
    {
        // Compute diew point with Magnus-Tetens formula
        GribRecord *recTemp =  getGribRecord(GRB_TEMP,LV_ABOV_GND,2,now);
        GribRecord *recHumid = getGribRecord(GRB_HUMID_REL,LV_ABOV_GND,2,now);
        if (recTemp && recHumid)
        {
            double temp = recTemp->getInterpolatedValue(lon, lat);
            double humid = recHumid->getInterpolatedValue(lon, lat);
            if (temp != GRIB_NOTDEF && humid != GRIB_NOTDEF)
            {
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

//---------------------------------------------------
int Grib::getTotalNumberOfGribRecords() {
        int nb=0;
        std::map <long int, std::vector<GribRecord *>* >::iterator it;
        for (it=mapGribRecords.begin(); it!=mapGribRecords.end(); it++)
        {
                nb += (int)(*it).second->size();
        }
        return nb;
}

//---------------------------------------------------
std::vector<GribRecord *> * Grib::getFirstNonEmptyList()
{
    std::vector<GribRecord *> *ls = NULL;
        std::map <long int, std::vector<GribRecord *>* >::iterator it;
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
                return (int)liste->size();
        else
                return 0;
}

//---------------------------------------------------------------------
std::vector<GribRecord *> * Grib::getListOfGribRecords(int dataType,int levelType,int levelValue)
{
        long int key = GribRecord::makeKey(dataType,levelType,levelValue);
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
        if(!before || !after)
            return;
        std::vector<GribRecord *> *ls = getListOfGribRecords(dataType,levelType,levelValue);

        *before = NULL;
        *after  = NULL;

        if(ls==NULL)
            return;

        zuint nb = (int)ls->size();
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

GribRecord * Grib::getGribRecord(int dataType,int levelType,int levelValue, time_t date)
{
    std::vector<GribRecord *> *ls = getListOfGribRecords(dataType,levelType,levelValue);
    if (ls != NULL) {
        // Cherche le premier enregistrement a la bonne date
        GribRecord *res = NULL;
        zuint nb = (int)ls->size();
        for (zuint i=0; i<nb && res==NULL; i++) {
            if ((*ls)[i]->getRecordCurrentDate() == date)
                res = (*ls)[i];
        }
        return res;
    }
    else {
        return NULL;
    }
}

bool Grib::getGribRecordArroundDates(int dataType,int levelType,int levelValue,
                                time_t now,time_t * tPrev,time_t * tNxt,
                                GribRecord ** recPrev,GribRecord ** recNxt)
{
    if(tPrev && tNxt && recPrev && recNxt)
    {
        findGribsAroundDate(dataType,levelType,levelValue,now,recPrev,recNxt);
        if(*recPrev)
        {
            if(*recPrev==*recNxt)
            {
                *tPrev=(*recPrev)->getRecordCurrentDate();
                *tNxt=*tPrev;
                *recNxt=NULL;
            }
            else
            {
                *tPrev=(*recPrev)->getRecordCurrentDate();
                if(*recNxt!=NULL)
                    *tNxt=(*recNxt)->getRecordCurrentDate();
                else
                    *tNxt=*tPrev;
            }
            return true;
        }
    }
    return false;
}

double Grib::getInterpolatedValue_byDates(int dataType,int levelType,int levelValue,double d_long, double d_lat, time_t now) {
    time_t tPrev,tNxt;
    GribRecord * recPrev;
    GribRecord * recNxt;
    if(getGribRecordArroundDates(dataType,levelType,levelValue,now,&tPrev,&tNxt,&recPrev,&recNxt)) {
        return getInterpolatedValue_byDates(d_long,d_lat,now,tPrev,tNxt,recPrev,recNxt);
    }
    else
        return 0;

}

double Grib::getInterpolatedValue_byDates(double d_long, double d_lat, time_t now,time_t tPrev,time_t tNxt,
                                    GribRecord * recPrev,GribRecord * recNxt) {
    double v = recPrev->getInterpolatedValue(d_long, d_lat, MUST_INTERPOLATE_VALUE);
    double v_2;
    if(v != GRIB_NOTDEF && tPrev!=tNxt)
    {
        v_2=recNxt->getInterpolatedValue(d_long, d_lat, MUST_INTERPOLATE_VALUE);
        if(v_2 != GRIB_NOTDEF)
            return v+((v_2-v)/((double)(tNxt-tPrev)))*((double)(now-tPrev));
        else
            return 0;
    }
    else
        return 0;
}

int Grib::getDewpointDataStatus(int /*levelType*/,int /*levelValue*/) {
        return dewpointDataStatus;
}

bool Grib::getInterpolatedValue_byDates(double d_long, double d_lat, time_t now,double * u, double * v,
                                        int interpolation_type,bool debug)
{
    GribRecord *recU1,*recV1,*recU2,*recV2;
    time_t t1,t2;

    if(interpolation_type==INTERPOLATION_UKN)
        interpolation_type=interpolation_param;

    if(u) *u=0;
    if(v) *v=0;

    if(!isOk())
        return false;

    if(getInterpolationParam(now,&t1,&t2,&recU1,&recV1,&recU2,&recV2,debug))
    {
        if(debug)
            qWarning() << "Param ok => go interpolation";
        return getInterpolatedValue_byDates(d_long,d_lat,now,t1,t2,recU1,recV1,recU2,recV2,u,v,interpolation_type,debug);
    }
    return false;
}

bool Grib::getInterpolatedValueCurrent_byDates(double d_long, double d_lat, time_t now,double * u, double * v,
                                        int interpolation_type,bool debug)
{

    if(forceCurrents)
    {
        *u=forcedCS;
        *v=degToRad(forcedCD);
        return true;
    }
    GribRecord *recU1,*recV1,*recU2,*recV2;
    time_t t1,t2;

    if(interpolation_type==INTERPOLATION_UKN)
        interpolation_type=interpolation_param;

    if(u) *u=0;
    if(v) *v=0;

    if(isOk())
    {
        if(this->getNumberOfGribRecords(GRB_CURRENT_VX,LV_MSL,0)>0)
        {
            if(getInterpolationParamCurrent(now,&t1,&t2,&recU1,&recV1,&recU2,&recV2,debug))
            {
                if(debug)
                    qWarning() << "Param ok => go interpolation";
                return getInterpolatedValue_byDates(d_long,d_lat,now,t1,t2,recU1,recV1,recU2,recV2,u,v,interpolation_type,debug);
            }
        }
    }
    if(!isCurrentGrib)
        return gribCurrent->getInterpolatedValueCurrent_byDates(d_long, d_lat, now, u, v, interpolation_type, debug);
    else
        return false;
}

bool Grib::getInterpolationParam(time_t now,time_t * t1,time_t * t2,GribRecord ** recU1,GribRecord ** recV1,
                           GribRecord ** recU2,GribRecord ** recV2,bool debug)
{
    if(t1 && t2 && recU1 && recV1 && recU2 && recV2)
    {
        findGribsAroundDate(GRB_WIND_VX,LV_ABOV_GND,10,now,recU1,recU2);
        findGribsAroundDate(GRB_WIND_VY,LV_ABOV_GND,10,now,recV1,recV2);
        if(*recU1 && *recV1)
        {
            if(*recU1==*recU2)
            {
                *t1=(*recU1)->getRecordCurrentDate();
                *t2=*t1;
                *recU2=NULL;
                *recV2=NULL;
            }
            else
            {
                *t1=(*recU1)->getRecordCurrentDate();
                if(*recU2!=NULL && *recV2!=0)
                    *t2=(*recU2)->getRecordCurrentDate();
            }

            if(debug)
            {
                qWarning() << "Time: now=" << now << " , t1=" << *t1 << ", t2=" << *t2;
            }

            return true;
        }
    }
    return false;
}
bool Grib::getInterpolationParamCurrent(time_t now,time_t * t1,time_t * t2,GribRecord ** recU1,GribRecord ** recV1,
                           GribRecord ** recU2,GribRecord ** recV2,bool debug)
{
    if(t1 && t2 && recU1 && recV1 && recU2 && recV2)
    {
        findGribsAroundDate(GRB_CURRENT_VX,LV_MSL,0,now,recU1,recU2);
        findGribsAroundDate(GRB_CURRENT_VY,LV_MSL,0,now,recV1,recV2);
        if(*recU1 && *recV1)
        {
            if(*recU1==*recU2)
            {
                *t1=(*recU1)->getRecordCurrentDate();
                *t2=*t1;
                *recU2=NULL;
                *recV2=NULL;
            }
            else
            {
                *t1=(*recU1)->getRecordCurrentDate();
                if(*recU2!=NULL && *recV2!=0)
                    *t2=(*recU2)->getRecordCurrentDate();
            }

            if(debug)
            {
                qWarning() << "Time: now=" << now << " , t1=" << *t1 << ", t2=" << *t2;
            }

            return true;
        }
    }
    return false;
}

bool Grib::getInterpolatedValue_byDates(double d_long, double d_lat, time_t now, time_t t1,time_t t2,
                                              GribRecord *recU1,GribRecord *recV1,GribRecord *recU2,GribRecord *recV2,
                                              double * u, double * v,int interpolation_type,bool debug)
{
    if(forceWind)
    {
        *u=forcedTWS;
        *v=degToRad(forcedTWD);
        return true;
    }
    windData wData_prev;
    windData wData_nxt;
    double gridOriginLat,gridOriginLon;
    gridOriginLat=recV1->getLatMin();
    gridOriginLon=recV1->getLonMin();

    if(interpolation_type==INTERPOLATION_UKN)
        interpolation_type=interpolation_param;

    bool hasNxt=false;
    //int isHighRes_t1=false,isHighRes_t2=false;
    double gribStep_t1_lon=1,gribStep_t2_lon=1;
    double gribStep_t1_lat=1,gribStep_t2_lat=1;

    /*sanity check */
    if(!u || !v || !recU1 || !recV1)
        return false;

    //isHighRes_t1=(recU1->getDi()==0.5 || recU1->getDi()==-0.5)?1:0;
    gribStep_t1_lon=recU1->getDi()==0?1:recU1->getDi();
    gribStep_t1_lat=recU1->getDj()==0?1:recU1->getDj();

    if(!recU1->getValue_TWSA(d_long,d_lat,&(wData_prev.u0),&(wData_prev.u1),&(wData_prev.u2),&(wData_prev.u3),debug))
        return false;
    if(!recV1->getValue_TWSA(d_long,d_lat,&(wData_prev.v0),&(wData_prev.v1),&(wData_prev.v2),&(wData_prev.v3),debug))
        return false;

    if(recU2 && recV2)
    {
        hasNxt=true;
        //isHighRes_t2=(recU2->getDi()==0.5 || recU2->getDi()==-0.5)?1:0;
        gribStep_t2_lon=recU2->getDi()==0?1:recU2->getDi();
        gribStep_t2_lat=recU2->getDj()==0?1:recU2->getDj();

        if(!recU2->getValue_TWSA(d_long,d_lat,&(wData_nxt.u0),(&wData_nxt.u1),&(wData_nxt.u2),&(wData_nxt.u3),debug))
            return false;
        if(!recV2->getValue_TWSA(d_long,d_lat,&(wData_nxt.v0),(&wData_nxt.v1),&(wData_nxt.v2),&(wData_nxt.v3),debug))
            return false;
    }
    gribStep_t1_lon=qAbs(gribStep_t1_lon);
    gribStep_t2_lon=qAbs(gribStep_t2_lon);
    gribStep_t1_lat=qAbs(gribStep_t1_lat);
    gribStep_t2_lat=qAbs(gribStep_t2_lat);
    switch(interpolation_type)
    {
        case INTERPOLATION_TWSA:
            if(debug)
                qWarning() << "Interpolation TWSA";
            interpolation::get_wind_info_latlong_TWSA(d_long,d_lat,now,t1,t2,&wData_prev,(hasNxt?(&wData_nxt):NULL),gribStep_t1_lat,gribStep_t1_lon,gribStep_t2_lat,gribStep_t2_lon,u,v,debug);
            break;
        case INTERPOLATION_SELECTIVE_TWSA:
            if(debug)
                qWarning() << "Interpolation selective-TWSA";
            interpolation::get_wind_info_latlong_selective_TWSA(d_long,d_lat,now,t1,t2,&wData_prev,(hasNxt?(&wData_nxt):NULL),gribStep_t1_lat,gribStep_t1_lon,gribStep_t2_lat,gribStep_t2_lon,u,v,debug);
            break;
        case INTERPOLATION_HYBRID:
            if(debug)
                qWarning() << "Interpolation Hybrid";
            interpolation::get_wind_info_latlong_hybrid(d_long,d_lat,now,t1,t2,&wData_prev,(hasNxt?(&wData_nxt):NULL),gribStep_t1_lat,gribStep_t1_lon,gribStep_t2_lat,gribStep_t2_lon,u,v,gridOriginLat,gridOriginLon,debug);
            break;
         default:
            if(debug)
                qWarning() << "NO interpolation defined";
            return false;
     }
    return true;
}

//---------------------------------------------------
// Rectangle de la zone couverte par les donnees
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
// Premier GribRecord trouve (pour recuperer la grille)
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
// Delai en heures entre 2 records
// On suppose qu'il est fixe pour tout le fichier !!!
double Grib::computeHoursBeetweenGribRecords()
{
        double res = 1;
    std::vector<GribRecord *> *ls = getFirstNonEmptyList();
    if (ls != NULL && ls->size()>1) {
        time_t t0 = (*ls)[0]->getRecordCurrentDate();
        time_t t1 = (*ls)[1]->getRecordCurrentDate();
        res = qAbs(t1-t0) / 3600.0;
        if (res < 1)
                res = 1;
    }
    return res;
}

//-------------------------------------------------------
// Genere la liste des dates pour lesquelles des previsions existent
void Grib::createListDates()
{   // Le set assure l'ordre et l'unicite des dates
    minDate=-1;
    maxDate=-1;
    setAllDates.clear();
    std::map <long int, std::vector<GribRecord *>* >::iterator it;
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

//----------------------------------------------------
void Grib::initIsobars()
{
    if (!ok)
        return;

    GribRecord *rec_prev,*rec_nxt;
    time_t tPrev,tNxt;
    if(getGribRecordArroundDates(GRB_PRESSURE,LV_MSL,0,currentDate,
                                 &tPrev,&tNxt,&rec_prev,&rec_nxt))
    {
        Util::cleanListPointers(listIsobars);
        IsoLine *iso;
        for (double press=840; press<1120; press += isobarsStep)
        {
                iso = new IsoLine(press*100, currentDate, tPrev,tNxt, rec_prev,rec_nxt);
                listIsobars.push_back(iso);
        }
    }
}

//----------------------------------------------------
void Grib::initIsotherms0()
{
    if (!ok)
        return;

    GribRecord *rec_prev,*rec_nxt;
    time_t tPrev,tNxt;
    if(getGribRecordArroundDates(GRB_GEOPOT_HGT,LV_ISOTHERM0,0,currentDate,
                                 &tPrev,&tNxt,&rec_prev,&rec_nxt))
    {
        Util::cleanListPointers(listIsotherms0);
        IsoLine *iso;
        for (double alt=0; alt<12000; alt += isotherms0Step)
        {
                iso = new IsoLine(alt, currentDate, tPrev,tNxt, rec_prev,rec_nxt);
                listIsotherms0.push_back(iso);
        }
    }
}

QString Grib::get_cartoucheData(void)
{
    if (!ok) return QString();
    return Util::formatDateTimeLong(currentDate);
}
