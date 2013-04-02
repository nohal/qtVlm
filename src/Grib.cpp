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

#include "Grib.h"
#include "Util.h"
#include "dataDef.h"
#include "GribRecord.h"
#include "Projection.h"
#include "IsoLine.h"
#include "settings.h"

#include "interpolation.h"
#ifdef QT_V5
#include <QtConcurrent/QtConcurrentMap>
#else
#include <QtConcurrentMap>
#endif
#include <QVector>
#ifdef __QTVLM_WITH_TEST
int nbWarning;
#endif
GribThreadResult interpolateThreaded(const GribThreadData &g)
{
    double tws=-1, twd=-1;
    GribThreadResult r;
    if(g.grib->getInterpolatedValue_byDates(g.p.x(),g.p.y(),g.cD,g.tP,g.tN,
                                                   g.recU1,g.recV1,g.recU2,g.recV2,&tws,&twd,
                                                   g.interpolMode))
        r.rgb=g.grib->getWindColor(tws, g.smooth);
    else
        tws=-1;
    r.tws=tws;
    r.twd=twd;
    return r;
}
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
    isCurrentGrib=false;
#ifdef __QTVLM_WITH_TEST
    nbWarning=0;
#endif
    this->gribMonoCpu=Settings::getSetting("gribMonoCpu",0).toInt()==1;
    QString interpol_name[4] = { "UKN", "TWSA", "selecive TWSA", "Hybride" };

    interpolation_param = INTERPOLATION_DEFAULT;

    qWarning() << "Starting with interpolation: " << interpol_name[interpolation_param];

    mustInterpolateValues = true;

    isCloudsColorModeWhite = Settings::getSetting("cloudsColorMode", "white").toString() == "white";

    isobarsStep = Settings::getSetting("isobarsStep", 2).toDouble();

    isotherms0Step = 50;

    dewpointDataStatus = NO_DATA_IN_FILE;

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
    // Color scale for rain in mm/h
    rainColor[ 0].setRgba(qRgba(255,255,255,  mapColorTransp));
    rainColor[ 1].setRgba(qRgba(200,240,255,  mapColorTransp));
    rainColor[ 2].setRgba(qRgba(150,240,255,  mapColorTransp));
    rainColor[ 3].setRgba(qRgba(100,200,255,  mapColorTransp));
    rainColor[ 4].setRgba(qRgba( 50,200,255,  mapColorTransp));
    rainColor[ 5].setRgba(qRgba(  0,150,255,  mapColorTransp));
    rainColor[ 6].setRgba(qRgba(  0,100,255,  mapColorTransp));
    rainColor[ 7].setRgba(qRgba(  0, 50,255,  mapColorTransp));
    rainColor[ 8].setRgba(qRgba( 50,  0,255,  mapColorTransp));
    rainColor[ 9].setRgba(qRgba(100,  0,255,  mapColorTransp));
    rainColor[10].setRgba(qRgba(150,  0,255,  mapColorTransp));
    rainColor[11].setRgba(qRgba(200,  0,255,  mapColorTransp));
    rainColor[12].setRgba(qRgba(250,  0,255,  mapColorTransp));
    rainColor[13].setRgba(qRgba(200,  0,200,  mapColorTransp));
    rainColor[14].setRgba(qRgba(150,  0,150,  mapColorTransp));
    rainColor[15].setRgba(qRgba(100,  0,100,  mapColorTransp));
    rainColor[16].setRgba(qRgba( 50,  0,50,  mapColorTransp));
    file=NULL;
    forceParam();
}
void Grib::forceParam()
{
    forceWind=Settings::getSetting("forceWind",0).toInt()==1;
    forcedTWS=Settings::getSetting("forcedTWS",0.0).toDouble();
    forcedTWD=Settings::getSetting("forcedTWD",0.0).toDouble();
    forceCurrents=Settings::getSetting("forceCurrents",0).toInt()==1;
    forcedCS=Settings::getSetting("forcedCS",0.0).toDouble();
    forcedCD=Settings::getSetting("forcedCD",0.0).toDouble();
}

//-------------------------------------------------------------------------------
Grib::~Grib()
{
    if(ok)
    {
        clean_all_vectors();
        Util::cleanListPointers(listIsobars);
        Util::cleanListPointers(listIsotherms0);
    }

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

int Grib::getDewpointDataStatus(int /*levelType*/,int /*levelValue*/)
{

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
    if (ls != NULL) {
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

/* Plot */
//--------------------------------------------------------------------------
QRgb  Grib::getRainColor(double v, bool smooth)
{
    QRgb rgb = 0;
    int  ind;

    double indf = pow(67.5*v,1.0/3.0);        // TODO better color map!!!!
        if (v > 0)
                indf += 0.2;

    ind = (int) floor(Util::inRange(indf, 0.0, 15.0));

    if (smooth && ind<16) {
        // Interpolation de couleur
        QColor c1 = rainColor[ind];
        QColor c2 = rainColor[ind+1];
        double dcol = indf-ind;
        rgb = qRgba(
            (int)( (double) c1.red()  *(1.0-dcol) + dcol*c2.red()   +0.5),
            (int)( (double) c1.green()*(1.0-dcol) + dcol*c2.green() +0.5),
            (int)( (double) c1.blue() *(1.0-dcol) + dcol*c2.blue()  +0.5),
            mapColorTransp
            );
    }
    else {
                ind = (int) (indf + 0.5);
                ind = Util::inRange(ind, 0, 15);
        rgb = rainColor[ind].rgba();
    }
    return rgb;
}
//--------------------------------------------------------------------------
QRgb  Grib::getSnowDepthColor(double mm, bool smooth)
{
    QRgb rgb = 0;
    int  ind;
    double v = mm / 3.0;
    double indf = pow(900*v,1.0/3.0);        // TODO better color map!!!!
        if (v > 0)
                indf += 0.2;
    ind = (int) floor(Util::inRange(indf, 0.0, 15.0));

    if (smooth && ind<16) {
        // Interpolation de couleur
        QColor c1 = rainColor[ind];
        QColor c2 = rainColor[ind+1];
        double dcol = indf-ind;
        rgb = qRgba(
            (int)( (double) c1.red()  *(1.0-dcol) + dcol*c2.red()   +0.5),
            (int)( (double) c1.green()*(1.0-dcol) + dcol*c2.green() +0.5),
            (int)( (double) c1.blue() *(1.0-dcol) + dcol*c2.blue()  +0.5),
            mapColorTransp
            );
    }
    else {
                ind = (int) (indf + 0.5);
                ind = Util::inRange(ind, 0, 15);
        rgb = rainColor[ind].rgba();
    }
    return rgb;
}
//--------------------------------------------------------------------------
QRgb  Grib::getCloudColor(double v, bool smooth)
{
        return getCloudColor(v, smooth, -1);
}
//--------------------------------------------------------------------------
QRgb  Grib::getCloudColor(double v, bool smooth, int colorModeUser) {
    QRgb rgb = 0;
    if (!smooth) {
        v = 10.0*floor(v/10.0);
    }
        v = v*v*v/10000;
    int r,g,b;
    int tr;
    if (  colorModeUser==1 ||
                 (colorModeUser<0 && isCloudsColorModeWhite) )
    {
                double k = 2.55;
                r = (int)(k*v);
                g = (int)(k*v);
                b = (int)(k*v);
                tr = (int)(2.5*v);
        }
        else {
                r = 255 - (int)(1.6*v);
                g = 255 - (int)(1.6*v);
                b = 255 - (int)(2.0*v);
                tr = mapColorTransp;
        }
    rgb = qRgba(r,g,b,  tr);
    return rgb;
}
//--------------------------------------------------------------------------
QRgb  Grib::getDeltaTemperaturesColor(double v, bool smooth) {
        v = 100.0 - Util::inRange(20.0*v, 0.0, 100.0);
    QRgb rgb = 0;
    if (!smooth) {
        v = 10.0*floor(v/10.0);
    }
    int r,g,b;
    int tr;
                r = 255 - (int)(1.5*v);
                g = 255 - (int)(1.5*v);
                b = 255 - (int)(2.2*v);
                tr = mapColorTransp;
    rgb = qRgba(r,g,b,  tr);
    return rgb;
}
//--------------------------------------------------------------------------
QRgb  Grib::getAltitudeColor(double v, bool smooth) {
    QRgb rgb = 0;
    if (!smooth) {
        v = 100*floor(v/100);
    }
    if (v<0)
        v = 0;
    if (v>6000)
        v = 6000;
        v = pow(v/6000, 1.0/2.0);
    int r = 255 - (int)(255*v);
    int g = 255;
    int b = 255 - (int)(255*v);
    rgb = qRgba(r,g,b,  mapColorTransp);
    return rgb;
}
//--------------------------------------------------------------------------
QRgb  Grib::getHumidColor(double v, bool smooth) {
    QRgb rgb = 0;
    if (!smooth) {
        v = 10.0*floor(v/10.0);
    }

    //v = v*v*v/10000;
    v = v*v/100;

    int r = 255 - (int)(2.4*v);
    int g = 255 - (int)(2.4*v);
    int b = 255 - (int)(1.2*v);
    rgb = qRgba(r,g,b,  mapColorTransp);
    return rgb;
}
//--------------------------------------------------------------------------
QRgb  Grib::getCAPEColor(double v, bool smooth)
{
        double x = sqrt(v)*70.71;
        double t0 = 0;  // valeur mini de l'echelle
        double t1 = 3000;  // valeur maxi de l'echelle
        double b0 = 0;    // min beauforts
        double b1 = 12;   // max beauforts
        double eqbeauf = b0 + (x-t0)*(b1-b0)/(t1-t0);
        if (eqbeauf < 0)
                eqbeauf = 0;
        else if (eqbeauf > 12)
                eqbeauf = 12;
        return getWindColor(Util::BeaufortToKmh_F(eqbeauf), smooth);
}
//--------------------------------------------------------------------------
QRgb  Grib::getTemperatureColor(double v, bool smooth)
{
        // Meme echelle coloree que pour le vent
        double x = v-273.15;
        double t0 = -30;  // valeur mini de l'echelle
        double t1 =  40;  // valeur maxi de l'echelle
        double b0 = 0;    // min beauforts
        double b1 = 12;   // max beauforts
        double eqbeauf = b0 + (x-t0)*(b1-b0)/(t1-t0);
        if (eqbeauf < 0)
                eqbeauf = 0;
        else if (eqbeauf > 12)
                eqbeauf = 12;
        return getWindColor(Util::BeaufortToKmh_F(eqbeauf), smooth);
}
//--------------------------------------------------------------------------
QRgb  Grib::getPressureColor(double v, bool smooth)
{
        // Meme echelle coloree que pour le vent
        double x = v;
        double t0 = 960;  // valeur mini de l'echelle
        double t1 = 1050;  // valeur maxi de l'echelle
        double b0 = 0;    // min beauforts
        double b1 = 12;   // max beauforts
        double eqbeauf = b0 + (x-t0)*(b1-b0)/(t1-t0);
        if (eqbeauf < 0)
                eqbeauf = 0;
        else if (eqbeauf > 12)
                eqbeauf = 12;
        return getWindColor(Util::BeaufortToKmh_F(eqbeauf), smooth);
}
//--------------------------------------------------------------------------
QRgb Grib::getWindColor(double v, bool smooth)
{
    QRgb rgb = 0;
    if (! smooth) {
        const int beauf = Util::kmhToBeaufort(v);
        rgb = windColor[beauf].rgba();
    }
    else {
        // Interpolation de couleur
        double fbeauf = Util::kmhToBeaufort_F(v);
        int f1=qBound(0,(int)(fbeauf),12);
        int f2=qBound(0,(int)(fbeauf+1),12);
        QColor c1 = windColor[f1];
        QColor c2 = windColor[f2];
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
// Carte de couleurs generique en dimension 1
//--------------------------------------------------------------------------
void Grib::drawColorMapGeneric_1D (
                QPainter &pnt, const Projection *proj, bool smooth,
                time_t now,time_t tPrev,time_t tNxt,
                GribRecord * recPrev,GribRecord * recNxt,
                QRgb (Grib::*function_getColor) (double v, bool smooth)
        )
{
    if (recPrev == NULL)
        return;
    int i, j;
    double x, y, v, v_2;
    int W = proj->getW();
    int H = proj->getH();
    QRgb   rgb;
    QImage *image = new QImage(W,H,QImage::Format_ARGB32);
    image->fill( qRgba(0,0,0,0));

    //qWarning() << "1D";

    for (i=0; i<W-1; i+=2) {
        for (j=0; j<H-1; j+=2) {
            proj->screen2map(i,j, &x, &y);
            if (! recPrev->isXInMap(x))
                x += 360.0;    // tour complet ?
            if (recPrev->isPointInMap(x, y)) {
                v = recPrev->getInterpolatedValue(x, y, mustInterpolateValues);
                if(v != GRIB_NOTDEF && tPrev!=tNxt)
                {
                    v_2=recNxt->getInterpolatedValue(x, y, mustInterpolateValues);
                    if(v_2 != GRIB_NOTDEF)
                        v=v+((v_2-v)/((double)(tNxt-tPrev)))*((double)(now-tPrev));
                }
                if (v != GRIB_NOTDEF) {
                    rgb = (this->*function_getColor) (v, smooth);
                    image->setPixel(i,  j, rgb);
                    image->setPixel(i+1,j, rgb);
                    image->setPixel(i,  j+1, rgb);
                    image->setPixel(i+1,j+1, rgb);
                }
            }
        }
    }

    pnt.drawImage(0,0,*image);
    delete image;
}

void Grib::show_CoverZone(QPainter &pnt, Projection * proj)
{
    if (!ok) {
        return;
    }

    double x0,y0, x1,y1;
    int i, j, k,l;
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
}

//--------------------------------------------------------------------------
// Carte de couleurs du vent
//--------------------------------------------------------------------------
void Grib::draw_WIND_Color_old(QPainter &pnt, const Projection *proj, bool smooth,
                               bool showWindArrows,bool barbules)
{

    int i, j;
    double u,v,x,y;
    int W = proj->getW();
    int H = proj->getH();
    int space=0;
    int W_s=0,H_s=0;
    QRgb   rgb;
    QImage image(W,H,QImage::Format_ARGB32);

    GribRecord *recU1,*recV1,*recU2,*recV2;
    time_t t1,t2;

    //qWarning() << "Drawing on " << proj->getW() << " " << proj->getH();

    if (!ok) {
        return;
    }

    if(!getInterpolationParam(currentDate,&t1,&t2,&recU1,&recV1,&recU2,&recV2))
        return;
    int sz=1;
    if(showWindArrows)
    {
        if (barbules)
            space =  windBarbuleSpace;
        else
            space =  windArrowSpace;

        W_s=W/space+1;
        H_s=H/space+1;
        sz=(W_s+2)*(H_s+2);
    }
    QVector<double> u_tab(sz,-1.0);
    QVector<double> v_tab(sz);
    QVector<bool> y_tab(sz);

    image.fill(Qt::transparent);
    int indice=0;
    for (i=0; i<W-2; i+=2)
    {
        for (j=0; j<H-2; j+=2)
        {
            proj->screen2map(i,j, &x, &y);
            if(getInterpolatedValue_byDates(x,y,currentDate,t1,t2,recU1,recV1,recU2,recV2,&u,&v))
            {
                if(showWindArrows && i%space==0 && j%space==0)
                {
                    int i_s=i/space;
                    int j_s=j/space;
                    indice=i_s*H_s+j_s;
                    u_tab[indice]=u;
                    v_tab[indice]=v;
                    y_tab[indice]=(y<0);
                }

                rgb=getWindColor(u, smooth);
                image.setPixel(i,  j,rgb);
                image.setPixel(i+1,j,rgb);
                image.setPixel(i,  j+1,rgb);
                image.setPixel(i+1,j+1,rgb);
            }
        }
    }

    pnt.drawImage(0,0,image);



    if(showWindArrows)
    {
        for (i=0; i<W_s; ++i)
        {
            for (j=0; j<H_s; ++j)
            {
                indice=i*H_s+j;
                u=u_tab.at(indice);
                v=v_tab.at(indice);

                if(u<0)
                    continue;
                if (barbules)
                    drawWindArrowWithBarbs(pnt, i*space,j*space, u,v, y_tab.at(indice));
                else
                    drawWindArrow(pnt, i*space,j*space, v);

            }
        }
    }
}
void Grib::draw_WIND_Color(QPainter &pnt, const Projection *proj, bool smooth,
                               bool showWindArrows,bool barbules)
{
//    QTime timeG;
//    int msecsG=0;
    if(gribMonoCpu || QThread::idealThreadCount()<=1)
    {
        draw_WIND_Color_old(pnt,proj,smooth,showWindArrows,barbules);
        return;
    }
//    else
//    {
//        timeG.start();
//        draw_WIND_Color_old(pnt,proj,smooth,showWindArrows,barbules);
//        msecsG=timeG.elapsed();
//    }
//    timeG.start();
    int i, j;
    double u,v,x,y;
    int W = proj->getW();
    int H = proj->getH();
    int space=0;
    int W_s=0,H_s=0;
    QImage image(W,H,QImage::Format_ARGB32_Premultiplied);
    GribRecord *recU1,*recV1,*recU2,*recV2;
    time_t t1,t2;
    if (!ok) {
        return;
    }

    if(!getInterpolationParam(currentDate,&t1,&t2,&recU1,&recV1,&recU2,&recV2))
        return;
    int sz=1;
    if(showWindArrows)
    {
        if (barbules)
            space =  windBarbuleSpace;
        else
            space =  windArrowSpace;

        W_s=W/space+1;
        H_s=H/space+1;
        sz=(W_s+2)*(H_s+2);
    }
    QVector<double> u_tab(sz,-1.0);
    QVector<double> v_tab(sz);
    QVector<bool> y_tab(sz);

    image.fill(Qt::transparent);

    int pass=-1;
    GribThreadData g;
    g.cD=currentDate;
    g.recU1=recU1;
    g.recU2=recU2;
    g.recV1=recV1;
    g.recV2=recV2;
    g.tP=t1;
    g.tN=t2;
    g.interpolMode=this->getInterpolationMode();
    g.smooth=smooth;
    g.grib=this;
    QList<GribThreadData> windData;
    windData.reserve(W*H);
    for (i=0; i<W-2; i+=2)
    {
        for (j=0; j<H-2; j+=2)
        {
            proj->screen2map(i,j, &x, &y);
            g.p=QPointF(x,y);
            windData.append(g);
        }
    }

    QList<GribThreadResult> windResults  = QtConcurrent::blockingMapped(windData, interpolateThreaded);

    int indice;
    for (i=0; i<W-2; i+=2)
    {
        for (j=0; j<H-2; j+=2)
        {
            ++pass;
            if(windResults.at(pass).tws!=-1)
            {
                if(showWindArrows && i%space==0 && j%space==0)
                {
                    int i_s=i/space;
                    int j_s=j/space;
                    indice=i_s*H_s+j_s;
                    u_tab[indice]=windResults.at(pass).tws;
                    v_tab[indice]=windResults.at(pass).twd;
                    y_tab[indice]=windData.at(pass).p.y()<0;
                }
                const QRgb rg=windResults.at(pass).rgb;
                image.setPixel(i,  j,rg);
                image.setPixel(i+1,j,rg);
                image.setPixel(i,  j+1,rg);
                image.setPixel(i+1,j+1,rg);
            }
        }
    }

    pnt.drawImage(0,0,image);

    if(showWindArrows)
    {
        for (i=0; i<W_s; ++i)
        {
            for (j=0; j<H_s; ++j)
            {
                indice=i*H_s+j;
                u=u_tab.at(indice);

                if(u<0)
                    continue;
                v=v_tab.at(indice);
                if (barbules)
                    drawWindArrowWithBarbs(pnt, i*space,j*space, u,v, y_tab.at(indice));
                else
                    drawWindArrow(pnt, i*space,j*space, v);

            }
        }
    }
#if 0
    qWarning()<<"old way"<<msecsG<<"new way"<<timeG.elapsed();
#endif
}
//--------------------------------------------------------------------------
// Carte de couleurs du courant
//--------------------------------------------------------------------------
void Grib::draw_CURRENT_Color(QPainter &pnt, const Projection *proj, bool smooth,
                               bool showWindArrows,bool barbules)
{
    int i, j;
    double u,v,x,y;
    double * u_tab=NULL, * v_tab=NULL;
    bool * y_tab=NULL;
    int W = proj->getW();
    int H = proj->getH();
    int space=0;
    int W_s=0,H_s=0;
    QRgb   rgb;
    QImage *image= new QImage(W,H,QImage::Format_ARGB32_Premultiplied);

    GribRecord *recU1,*recV1,*recU2,*recV2;
    time_t t1,t2;

    //qWarning() << "Drawing on " << proj->getW() << " " << proj->getH();

    if (!ok) {
        return;
    }

    if(!getInterpolationParamCurrent(currentDate,&t1,&t2,&recU1,&recV1,&recU2,&recV2))
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

        /* clearing u_tab array */
        for(i=0;i<W_s*H_s;i++)
        {
            u_tab[i]=-1;
        }
    }

    image->fill( qRgba(0,0,0,0));

    for (i=0; i<W-2; i+=2)
    {
        for (j=0; j<H-2; j+=2)
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

                rgb=getWindColor(u*20.0, smooth); //for current
                image->setPixel(i,  j,rgb);
                image->setPixel(i+1,j,rgb);
                image->setPixel(i,  j+1,rgb);
                image->setPixel(i+1,j+1,rgb);
            }
            /*else
            {
                if(showWindArrows && i%space==0 && j%space==0)
                {
                    int i_s=i/space;
                    int j_s=j/space;
                    u_tab[i_s*H_s+j_s]=-1;
                }
            }*/
        }
    }

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
        delete[] u_tab;
        delete[] v_tab;
        delete[] y_tab;
    }
}

//-------------------------------------------------------------
// Grille GRIB
void Grib::draw_GribGrid(QPainter &pnt, const Projection *proj)
{

    pnt.setPen(QColor(100,100,100));

    if (!ok) {
        return;
    }
    GribRecord *rec = getFirstGribRecord();
    if (rec == NULL)
        return;
    int px,py, i,j, dl=2;
    for (i=0; i<rec->getNi(); i++)
        for (j=0; j<rec->getNj(); j++)
        {
            if (rec->hasValue(i,j))
            {
                proj->map2screen(rec->getX(i), rec->getY(j), &px,&py);
                pnt.drawLine(px-dl,py, px+dl,py);
                pnt.drawLine(px,py-dl, px,py+dl);
                proj->map2screen(rec->getX(i)-360.0, rec->getY(j), &px,&py);
                pnt.drawLine(px-dl,py, px+dl,py);
                pnt.drawLine(px,py-dl, px,py+dl);
            }
        }
}

//--------------------------------------------------------------------------
// Carte de couleurs generique de la difference entre 2 champs
//--------------------------------------------------------------------------
void  Grib::drawColorMapGeneric_Abs_Delta_Data (
                QPainter &pnt, const Projection *proj, bool smooth,time_t now,
                time_t tPrevTemp,time_t tNxtTemp,GribRecord * recPrevTemp,GribRecord * recNxtTemp,
                time_t tPrevDewpoint,time_t tNxtDewpoint,GribRecord * recPrevDewpoint,GribRecord * recNxtDewpoint,
                QRgb (Grib::*function_getColor) (double v, bool smooth)
        )
{
    if (recPrevTemp == NULL || recPrevDewpoint == NULL)
        return;
    int i, j;
    double x, y, vx, vy, v, vx2, vy2;
    int W = proj->getW();
    int H = proj->getH();
    QRgb   rgb;
    QImage *image = new QImage(W,H,QImage::Format_ARGB32);
    image->fill( qRgba(0,0,0,0));
    for (i=0; i<W-1; i+=2) {
        for (j=0; j<H-1; j+=2)
        {
            proj->screen2map(i,j, &x, &y);

            if (! recPrevTemp->isXInMap(x))
                x += 360.0;    // tour complet ?

            if (recPrevTemp->isPointInMap(x, y))
            {
                vx = recPrevTemp->getInterpolatedValue(x, y, mustInterpolateValues);
                if(vx != GRIB_NOTDEF && tPrevTemp!=tNxtTemp)
                {
                    vx2=recNxtTemp->getInterpolatedValue(x, y, mustInterpolateValues);
                    if(vx2 != GRIB_NOTDEF)
                        vx=vx+((vx2-vx)/((double)(tNxtTemp-tPrevTemp)))*((double)(now-tPrevTemp));
                }

                vy = recPrevDewpoint->getInterpolatedValue(x, y, mustInterpolateValues);
                if(vy != GRIB_NOTDEF && tPrevDewpoint!=tNxtDewpoint)
                {
                    vy2=recNxtDewpoint->getInterpolatedValue(x, y, mustInterpolateValues);
                    if(vy2 != GRIB_NOTDEF)
                        vy=vy+((vy2-vy)/((double)(tNxtDewpoint-tPrevDewpoint)))*((double)(now-tPrevDewpoint));
                }

                if (vx != GRIB_NOTDEF && vy != GRIB_NOTDEF)
                {
                    v = fabs(vx-vy);
                    rgb = (this->*function_getColor) (v, smooth);
                    image->setPixel(i,  j, rgb);
                    image->setPixel(i+1,j, rgb);
                    image->setPixel(i,  j+1, rgb);
                    image->setPixel(i+1,j+1, rgb);
                }
            }
        }
    }
        pnt.drawImage(0,0,*image);
    delete image;
}


//--------------------------------------------------------------------------
// Carte de couleurs des precipitations
//--------------------------------------------------------------------------
void Grib::draw_RAIN_Color(QPainter &pnt, const Projection *proj, bool smooth)
{
    if (!ok)
        return;
    GribRecord *rec_prev,*rec_nxt;
    time_t tPrev,tNxt;
    if(getGribRecordArroundDates(GRB_PRECIP_TOT,LV_GND_SURF,0,currentDate,
                                 &tPrev,&tNxt,&rec_prev,&rec_nxt))
    {
        drawColorMapGeneric_1D(pnt,proj,smooth, currentDate,tPrev,tNxt,rec_prev,rec_nxt, &Grib::getRainColor);
    }
}

//--------------------------------------------------------------------------
// Carte de couleurs de la hauteur de neige
//--------------------------------------------------------------------------
void Grib::draw_SNOW_DEPTH_Color(QPainter &pnt, const Projection *proj, bool smooth)
{
    if (!ok)
        return;
    GribRecord *rec_prev,*rec_nxt;
    time_t tPrev,tNxt;
    if(getGribRecordArroundDates(GRB_SNOW_DEPTH,LV_GND_SURF,0,currentDate,
                                 &tPrev,&tNxt,&rec_prev,&rec_nxt))
    {
        drawColorMapGeneric_1D(pnt,proj,smooth, currentDate,tPrev,tNxt,rec_prev,rec_nxt, &Grib::getSnowDepthColor);
    }
}
void Grib::draw_SNOW_CATEG_Color(QPainter &pnt, const Projection *proj, bool smooth)
{
    if (!ok)
        return;
    GribRecord *rec_prev,*rec_nxt;
    time_t tPrev,tNxt;
    if(getGribRecordArroundDates(GRB_SNOW_CATEG,LV_GND_SURF,0,currentDate,
                                 &tPrev,&tNxt,&rec_prev,&rec_nxt))
    {
        drawColorMapGeneric_1D(pnt,proj,smooth, currentDate,tPrev,tNxt,rec_prev,rec_nxt, &Grib::getSnowDepthColor);
    }
}
void Grib::draw_FRZRAIN_CATEG_Color(QPainter &pnt, const Projection *proj, bool smooth)
{
    if (!ok)
        return;
    GribRecord *rec_prev,*rec_nxt;
    time_t tPrev,tNxt;
    if(getGribRecordArroundDates(GRB_FRZRAIN_CATEG,LV_GND_SURF,0,currentDate,
                                 &tPrev,&tNxt,&rec_prev,&rec_nxt))
    {
        drawColorMapGeneric_1D(pnt,proj,smooth, currentDate,tPrev,tNxt,rec_prev,rec_nxt, &Grib::getSnowDepthColor);
    }
}

//--------------------------------------------------------------------------
// Carte de couleurs de la nebulosite
//--------------------------------------------------------------------------
void Grib::draw_CLOUD_Color(QPainter &pnt, const Projection *proj, bool smooth)
{
    if (!ok)
        return;
    isCloudsColorModeWhite = Settings::getSetting("cloudsColorMode", "white").toString() == "white";

    GribRecord *rec_prev,*rec_nxt;
    time_t tPrev,tNxt;
    if(getGribRecordArroundDates(GRB_CLOUD_TOT,LV_ATMOS_ALL,0,currentDate,
                                 &tPrev,&tNxt,&rec_prev,&rec_nxt))
    {
        drawColorMapGeneric_1D(pnt,proj,smooth, currentDate,tPrev,tNxt,rec_prev,rec_nxt, &Grib::getCloudColor);
    }
}
//--------------------------------------------------------------------------
// Carte de couleurs de l'humidite relative
//--------------------------------------------------------------------------
void Grib::draw_HUMID_Color(QPainter &pnt, const Projection *proj, bool smooth)
{
    if (!ok)
        return;
    GribRecord *rec_prev,*rec_nxt;
    time_t tPrev,tNxt;
    if(getGribRecordArroundDates(GRB_HUMID_REL,LV_ABOV_GND,2,currentDate,
                                 &tPrev,&tNxt,&rec_prev,&rec_nxt))
    {
        drawColorMapGeneric_1D(pnt,proj,smooth, currentDate,tPrev,tNxt,rec_prev,rec_nxt, &Grib::getHumidColor);
    }
}

//--------------------------------------------------------------------------
void Grib::draw_Temp_Color(QPainter &pnt, const Projection *proj, bool smooth)
{
    if (!ok)
        return;
    GribRecord *rec_prev,*rec_nxt;
    time_t tPrev,tNxt;
    if(getGribRecordArroundDates(GRB_TEMP,LV_ABOV_GND,2,currentDate,
                                 &tPrev,&tNxt,&rec_prev,&rec_nxt))
    {
        drawColorMapGeneric_1D(pnt,proj,smooth, currentDate,tPrev,tNxt,rec_prev,rec_nxt, &Grib::getTemperatureColor);
    }
}
//--------------------------------------------------------------------------
void Grib::draw_TempPot_Color(QPainter &pnt, const Projection *proj, bool smooth)
{
    if (!ok)
        return;
    GribRecord *rec_prev,*rec_nxt;
    time_t tPrev,tNxt;
    if(getGribRecordArroundDates(GRB_TEMP_POT,LV_SIGMA,9950,currentDate,
                                 &tPrev,&tNxt,&rec_prev,&rec_nxt))
    {
        drawColorMapGeneric_1D(pnt,proj,smooth, currentDate,tPrev,tNxt,rec_prev,rec_nxt, &Grib::getTemperatureColor);
    }
}
//--------------------------------------------------------------------------
void Grib::draw_Dewpoint_Color(QPainter &pnt, const Projection *proj, bool smooth)
{
    if (!ok)
        return;
    GribRecord *rec_prev,*rec_nxt;
    time_t tPrev,tNxt;
    if(getGribRecordArroundDates(GRB_DEWPOINT,LV_ABOV_GND,2,currentDate,
                                 &tPrev,&tNxt,&rec_prev,&rec_nxt))
    {
        drawColorMapGeneric_1D(pnt,proj,smooth, currentDate,tPrev,tNxt,rec_prev,rec_nxt, &Grib::getTemperatureColor);
    }
}
//--------------------------------------------------------------------------
void Grib::draw_CAPEsfc(QPainter &pnt, const Projection *proj, bool smooth)
{
    if (!ok)
        return;
    GribRecord *rec_prev,*rec_nxt;
    time_t tPrev,tNxt;
    if(getGribRecordArroundDates(GRB_CAPE,LV_GND_SURF,0,currentDate,
                                 &tPrev,&tNxt,&rec_prev,&rec_nxt))
    {
        drawColorMapGeneric_1D(pnt,proj,smooth, currentDate,tPrev,tNxt,rec_prev,rec_nxt, &Grib::getCAPEColor);
    }
}
//--------------------------------------------------------------------------
// Carte de l'ecart temperature-point de rosee
//--------------------------------------------------------------------------
void Grib::draw_DeltaDewpoint_Color(QPainter &pnt, const Projection *proj, bool smooth)
{
    if (!ok) {
        return;
    }

    GribRecord *rec_prevTemp,*rec_nxtTemp;
    time_t tPrevTemp,tNxtTemp;
    GribRecord *rec_prevDewpoint,*rec_nxtDewpoint;
    time_t tPrevDewpoint,tNxtDewpoint;

    if(getGribRecordArroundDates(GRB_TEMP,LV_ABOV_GND,2,currentDate,
                                 &tPrevTemp,&tNxtTemp,&rec_prevTemp,&rec_nxtTemp)
        && getGribRecordArroundDates(GRB_DEWPOINT,LV_ABOV_GND,2,currentDate,
                                     &tPrevDewpoint,&tNxtDewpoint,&rec_prevDewpoint,&rec_nxtDewpoint))
    {
            drawColorMapGeneric_Abs_Delta_Data (pnt,proj,smooth,currentDate,
                                                tPrevTemp,tNxtTemp,rec_prevTemp,rec_nxtTemp,
                                                tPrevDewpoint,tNxtDewpoint,rec_prevDewpoint,rec_nxtDewpoint,
                                                        &Grib::getDeltaTemperaturesColor );
    }
}

QString Grib::drawCartouche(QPainter &)
{
    if (!ok) return QString();
    return Util::formatDateTimeLong(currentDate);
//    int fSize=12;
//    QFont fontbig("TypeWriter", fSize, QFont::Bold, false);
//    fontbig.setStyleHint(QFont::TypeWriter);
//    fontbig.setStretch(QFont::Condensed);
//    QColor   transpcolor(255,255,255,120);
//    QColor   textcolor(20,20,20,255);
//    pnt.setBrush(transpcolor);
//    pnt.setFont(fontbig);
//    pnt.setPen(transpcolor);
//    pnt.drawRect(3,3,190,fSize+3+4);
//    pnt.setPen(textcolor);

//    pnt.drawText(10, fSize+6, Util::formatDateTimeLong(currentDate));// forecast validity date
}

void Grib::draw_Isobars(QPainter &pnt, const Projection *proj)
{
    std::list<IsoLine *>::iterator it;
    for(it=listIsobars.begin(); it!=listIsobars.end(); ++it)
    {
        (*it)->drawIsoLine(pnt, proj);
    }
}

//--------------------------------------------------------------------------
void Grib::draw_Isotherms0(QPainter &pnt, const Projection *proj)
{
    std::list<IsoLine *>::iterator it;
    for(it=listIsotherms0.begin(); it!=listIsotherms0.end(); ++it)
    {
        (*it)->drawIsoLine(pnt, proj);
    }
}
//--------------------------------------------------------------------------
void Grib::draw_IsoLinesLabels(QPainter &pnt, QColor &couleur, const Projection *proj,
                                                std::list<IsoLine *>liste, double coef)
{
    std::list<IsoLine *>::iterator it;
    int nbseg = 0;
    for(it=liste.begin(); it!=liste.end(); ++it)
    {
        nbseg += (*it)->getNbSegments();
    }
    int nbpix, density, first;
    nbpix = proj->getW()*proj->getH();
    if (nbpix == 0)
        return;
    double r = (double)nbseg/nbpix *1000;
    double dens = 10;
    density =  (int) (r*dens +0.5);
    if (density < 20)
        density = 20;
    first = 0;
    for(it=liste.begin(); it!=liste.end(); ++it)
    {
        first += 20;
        (*it)->drawIsoLineLabels(pnt, couleur, proj, density, first, coef);
    }
}
//--------------------------------------------------------------------------
void Grib::draw_Isotherms0Labels(QPainter &pnt, const Projection *proj)
{
        QColor couleur(200,80,80);
        draw_IsoLinesLabels(pnt, couleur, proj, listIsotherms0, 1.0);
}
//--------------------------------------------------------------------------
void Grib::draw_IsobarsLabels(QPainter &pnt, const Projection *proj)
{
    QColor couleur(40,40,40);
        draw_IsoLinesLabels(pnt, couleur, proj, listIsobars, 0.01);
}

//--------------------------------------------------------------------------
void Grib::draw_PRESSURE_MinMax(QPainter &pnt, const Projection *proj)
{
    if (!ok) {
        return;
    }
    GribRecord *rec_prev,*rec_nxt;
    time_t tPrev,tNxt;
    if(!getGribRecordArroundDates(GRB_PRESSURE,LV_MSL,0,currentDate,
                                 &tPrev,&tNxt,&rec_prev,&rec_nxt))
        return;

    int i, j, W, H, pi,pj;
    double x, y, v,v1;
    double a,b,c,d,e,f,g,h,a1,b1,c1,d1,e1,f1,g1,h1;

    QFont fontPressureMinMax("Times", 18, QFont::Bold, true);
    QFontMetrics fmet(fontPressureMinMax);
    pnt.setPen(QColor(0,0,0));
    pnt.setFont(fontPressureMinMax);
    W = rec_prev->getNi();
    H = rec_prev->getNj();

    for (j=1; j<H-1; j++) {     // !!!! 1 to end-1
        for (i=1; i<W-1; i++) {
            v = rec_prev->getValue( i, j );

            a=rec_prev->getValue( i-1, j-1 );
            b=rec_prev->getValue( i-1, j   );
            c=rec_prev->getValue( i-1, j+1 );
            d=rec_prev->getValue( i  , j-1 );
            e=rec_prev->getValue( i  , j+1 );
            f=rec_prev->getValue( i+1, j-1 );
            g=rec_prev->getValue( i+1, j   );
            h=rec_prev->getValue( i+1, j+1 );
            if(tNxt!=tPrev)
            {
                v1 = rec_nxt->getValue( i, j );
                a1=rec_nxt->getValue( i-1, j-1 );
                b1=rec_nxt->getValue( i-1, j   );
                c1=rec_nxt->getValue( i-1, j+1 );
                d1=rec_nxt->getValue( i  , j-1 );
                e1=rec_nxt->getValue( i  , j+1 );
                f1=rec_nxt->getValue( i+1, j-1 );
                g1=rec_nxt->getValue( i+1, j   );
                h1=rec_nxt->getValue( i+1, j+1 );

                v = v + ((v1-v)/((double)(tNxt-tPrev)))*((double)(currentDate-tPrev));
                a = a + ((a1-a)/((double)(tNxt-tPrev)))*((double)(currentDate-tPrev));
                b = b + ((b1-b)/((double)(tNxt-tPrev)))*((double)(currentDate-tPrev));
                c = c + ((c1-c)/((double)(tNxt-tPrev)))*((double)(currentDate-tPrev));
                d = d + ((d1-d)/((double)(tNxt-tPrev)))*((double)(currentDate-tPrev));
                e = e + ((e1-e)/((double)(tNxt-tPrev)))*((double)(currentDate-tPrev));
                f = f + ((f1-f)/((double)(tNxt-tPrev)))*((double)(currentDate-tPrev));
                g = g + ((g1-g)/((double)(tNxt-tPrev)))*((double)(currentDate-tPrev));
                h = h + ((h1-h)/((double)(tNxt-tPrev)))*((double)(currentDate-tPrev));
            }

            if ( v < 101200
                   && v < a  // Minima local ?
                   && v < b
                   && v < c
                   && v < d
                   && v < e
                   && v < f
                   && v < g
                   && v < h
            ) {
                x = rec_prev->getX(i);
                y = rec_prev->getY(j);
                proj->map2screen(x,y, &pi, &pj);
                pnt.drawText(pi-fmet.width('L')/2, pj+fmet.ascent()/2, "L");
                proj->map2screen(x-360.0,y, &pi, &pj);
                pnt.drawText(pi-fmet.width('L')/2, pj+fmet.ascent()/2, "L");
            }
            if ( v > 101200
                   && v >= a  // Maxima local ?
                   && v >= b
                   && v >= c
                   && v >= d
                   && v >= e
                   && v >= f
                   && v >= g
                   && v >= h
            ) {
                x = rec_prev->getX(i);
                y = rec_prev->getY(j);
                proj->map2screen(x,y, &pi, &pj);
                pnt.drawText(pi-fmet.width('H')/2, pj+fmet.ascent()/2, "H");
                proj->map2screen(x-360.0,y, &pi, &pj);
                pnt.drawText(pi-fmet.width('H')/2, pj+fmet.ascent()/2, "H");
            }
        }
    }
}


//--------------------------------------------------------------------------
void Grib::draw_TEMPERATURE_Labels(QPainter &pnt, const Projection *proj)
{
    if (!ok) {
        return;
    }

    GribRecord *rec_prev,*rec_nxt;
    time_t tPrev,tNxt;
    if(!getGribRecordArroundDates(GRB_TEMP,LV_ABOV_GND,2,currentDate,
                                 &tPrev,&tNxt,&rec_prev,&rec_nxt))
        return;


    QFont fontTemperatureLabels("Times", 9, QFont::Bold, true);
    QFontMetrics fmet(fontTemperatureLabels);
    pnt.setFont(fontTemperatureLabels);
    pnt.setPen(QColor(0,0,0));

    double x, y, v,v1;
    int i, j, dimin, djmin;
    dimin = 50;
    djmin = 30;

    for (j=0; j<proj->getH(); j+= djmin) {
        for (i=0; i<proj->getW(); i+= dimin) {
            proj->screen2map(i,j, &x,&y);

            v = rec_prev->getInterpolatedValue(x, y, mustInterpolateValues);
            if (v!= GRIB_NOTDEF) {
                if(tNxt!=tPrev)
                {
                    v1 = rec_nxt->getInterpolatedValue(x, y, mustInterpolateValues);
                    if (v1!= GRIB_NOTDEF)
                        v = v + ((v1-v)/((double)(tNxt-tPrev)))*((double)(currentDate-tPrev));
                }
                QString strtemp = Util::formatTemperature_short(v);
                pnt.drawText(i-fmet.width("XXX")/2, j+fmet.ascent()/2, strtemp);
            }

        }
    }
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
    // Clip force a cause d'un bug qpixmap sous windows
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
        int r = 5;     // vent tres faible, dessine un cercle
        pnt.drawEllipse(i-r,j-r,2*r,2*r);
    }
    else {
        // Fleche centree sur l'origine
        int dec = -windBarbuleSize/2;
        drawTransformedLine(pnt, si,co, i,j,  dec,0,  dec+windBarbuleSize, 0);   // hampe
        drawTransformedLine(pnt, si,co, i,j,  dec,0,  dec+5, 2);    // flèche
        drawTransformedLine(pnt, si,co, i,j,  dec,0,  dec+5, -2);   // flèche

                int b1 = dec+windBarbuleSize -4;  // position de la 1ere barbule
                if (vkn >= 7.5  &&  vkn < 45 ) {
                        b1 = dec+windBarbuleSize;  // position de la 1ere barbule si >= 10 noeuds
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
