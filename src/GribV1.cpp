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

#include <QDebug>
#include <map>


#include "GribV1Record.h"
#include "GribV1.h"
#include "IsoLine.h"

GribV1::GribV1(DataManager *dataManager): Grib(dataManager) {
    version=1;
}

bool GribV1::isGribV1(QString fileName) {
    if(fileName=="") return false;

    std::string fname = qPrintable(fileName);

    int compressMode=GribV1::findCompressMode(fname.c_str());
    if(compressMode!=COMPRESS_NO_GRIB)
        return true;
    else
        return false;
}

int GribV1::findCompressMode(const char * fname) {
    ZUFILE * fptr=NULL;

    QList<int> compressModes;

    bool found=false;
    int compressMode;

    compressModes.append(ZU_COMPRESS_AUTO);
    compressModes.append(ZU_COMPRESS_BZIP);
    compressModes.append(ZU_COMPRESS_GZIP);
    compressModes.append(ZU_COMPRESS_NONE);

    for(compressMode=0;compressMode<compressModes.count();++compressMode)
    {
        if(fptr!=NULL)
            zu_close(fptr);
        fptr = zu_open(fname, "rb", compressModes.at(compressMode));
        if(!fptr) continue;
        found=chkIsGrib(fptr);
        if (found) break;
    }

    if(found && fptr) {
        zu_seek(fptr,3,SEEK_CUR);
        zuchar t;
        if (zu_read(fptr, &t, 1) == 1) {
            if(t!=1)
                found=false;
        }
        else
            found=false;
    }

    if(fptr!=NULL)
        zu_close(fptr);

    if(!found) {
        qWarning() << "Not a grib file (V1): " << fname;
        return COMPRESS_NO_GRIB;
    }
    else
        return compressModes.at(compressMode);
}

bool GribV1::chkIsGrib(ZUFILE * fptr) {
    if(fptr==NULL) return false;
    char buf[1];
    memset (buf, 0, sizeof (buf));
    int fileOffset0=0;
    bool found=false;
    while(fileOffset0<100)
    {
        if(zu_read(fptr,buf,1)!=1) break;
        ++fileOffset0;
        if(buf[0]!='G')
            continue;
        if(zu_read(fptr,buf,1)!=1) break;
        ++fileOffset0;
        if(buf[0]!='R')
        {
            if(buf[0]=='G')
                zu_seek(fptr,--fileOffset0,SEEK_SET);
            continue;
        }
        if(zu_read(fptr,buf,1)!=1) break;
        ++fileOffset0;
        if(buf[0]!='I')
        {
            if(buf[0]=='G')
                zu_seek(fptr,--fileOffset0,SEEK_SET);
            continue;
        }
        if(zu_read(fptr,buf,1)!=1) break;
        ++fileOffset0;
        if(buf[0]!='B')
        {
            if(buf[0]=='G')
                zu_seek(fptr,--fileOffset0,SEEK_SET);
            continue;
        }
        found=true;
        break;
    }
    return found;
}

bool GribV1::loadFile(QString fileName) {
    std::string fname = qPrintable(fileName);
    ok=false;

    //qWarning() << "V1-loadfile: " << fname.c_str();

    this->fileName=fileName;

    int compressMode=findCompressMode(fname.c_str());

    qWarning() << "GV1 loading: file " << fileName;
    qWarning() << "GV1 loading: compress mode found: " << compressMode;

    if(compressMode==COMPRESS_NO_GRIB)
        return false;

    /* clean data structure + iso lines */
    clean_all_vectors();
    Util::cleanListPointers(listIsobars);
    Util::cleanListPointers(listIsotherms0);

    if(!readAllGribRecords(fname.c_str(),compressMode))
        return false;

    createDewPointData();

    ok=true;

    return true;
}

bool GribV1::readAllGribRecords(const char * fname,int compressMode) {
    ZUFILE * fptr=NULL;
    GribV1Record *rec;
    time_t firstdate = -1;
    bool recAdded;

    fptr = zu_open(fname, "rb", compressMode);
    if(!fptr)
        return false;

    //--------------------------------------------------------
    // Lecture de l'ensemble des GribRecord du fichier
    // et stockage dans les listes appropriees.
    //--------------------------------------------------------

    while(true) {
        rec = new GribV1Record(fptr);

        recAdded=false;

        if(!rec || rec->isEof()) {
            if(rec) {
                delete rec;
                rec=NULL;
            }
            break;
        }

        if (rec->isOk())
        {
            if (rec->isDataKnown())
            {
                ok = true;   // au moins 1 record ok

                if (firstdate== -1)
                    firstdate = rec->get_curDate();


                if (//-----------------------------------------
                        (rec->get_dataType()==DATA_PRESSURE
                         && rec->get_levelType()==DATA_LV_MSL && rec->get_levelValue()==0)
                        //-----------------------------------------
                        || ( (rec->get_dataType()==DATA_TMIN || rec->get_dataType()==DATA_TMAX)
                             && rec->get_levelType()==DATA_LV_ABOV_GND && rec->get_levelValue()==2)
                        //-----------------------------------------
                        || (rec->get_dataType()==DATA_TEMP
                            && rec->get_levelType()==DATA_LV_ABOV_GND && rec->get_levelValue()==2)
                        || (rec->get_dataType()==DATA_TEMP
                            && rec->get_levelType()==DATA_LV_ISOBARIC
                            && (   rec->get_levelValue()==850
                                   || rec->get_levelValue()==700
                                   || rec->get_levelValue()==500
                                   || rec->get_levelValue()==300
                                   || rec->get_levelValue()==200 ) )
                        //-----------------------------------------
                        // Wind
                        //-----------------------------------------
                        || ( (rec->get_dataType()==DATA_WIND_VX || rec->get_dataType()==DATA_WIND_VY)
                             && rec->get_levelType()==DATA_LV_ABOV_GND
                             && (   rec->get_levelValue()==1
                                    || rec->get_levelValue()==2
                                    || rec->get_levelValue()==3
                                    || rec->get_levelValue()==10 ) )
                        || ( (rec->get_dataType()==DATA_WIND_VX || rec->get_dataType()==DATA_WIND_VY)
                             && rec->get_levelType()==DATA_LV_MSL
                             && rec->get_levelValue()==0 )
                        || ( (rec->get_dataType()==DATA_WIND_VX || rec->get_dataType()==DATA_WIND_VY)
                             && rec->get_levelType()==DATA_LV_GND_SURF
                             && (rec->get_levelValue()==0 || rec->get_levelValue()==1))
                        || ( (rec->get_dataType()==DATA_WIND_VX || rec->get_dataType()==DATA_WIND_VY)
                             && rec->get_levelType()==DATA_LV_ISOBARIC
                             && (   rec->get_levelValue()==850
                                    || rec->get_levelValue()==700
                                    || rec->get_levelValue()==500
                                    || rec->get_levelValue()==300
                                    || rec->get_levelValue()==200 ) )
                        || ( (rec->get_dataType()==DATA_CURRENT_VX || rec->get_dataType()==DATA_CURRENT_VY)
                             && rec->get_levelType()==DATA_LV_MSL
                             && rec->get_levelValue()==0 )
                        //-----------------------------------------
                        //-----------------------------------------
                        || (rec->get_dataType()==DATA_HUMID_SPEC
                            && rec->get_levelType()==DATA_LV_ISOBARIC
                            && (   rec->get_levelValue()==850
                                   || rec->get_levelValue()==700
                                   || rec->get_levelValue()==500
                                   || rec->get_levelValue()==300
                                   || rec->get_levelValue()==200 ) )
                        //-----------------------------------------
                        || (rec->get_dataType()==DATA_GEOPOT_HGT
                            && rec->get_levelType()==DATA_LV_ISOTHERM0 && rec->get_levelValue()==0)
                        || (rec->get_dataType()==DATA_GEOPOT_HGT
                            && rec->get_levelType()==DATA_LV_ISOBARIC
                            && (   rec->get_levelValue()==850
                                   || rec->get_levelValue()==700
                                   || rec->get_levelValue()==500
                                   || rec->get_levelValue()==300
                                   || rec->get_levelValue()==200 ) )
                        //-----------------------------------------
                        || (rec->get_dataType()==DATA_PRECIP_TOT
                            && rec->get_levelType()==DATA_LV_GND_SURF && rec->get_levelValue()==0)
                        //-----------------------------------------
                        || (rec->get_dataType()==DATA_PRECIP_RATE
                            && rec->get_levelType()==DATA_LV_GND_SURF && rec->get_levelValue()==0)
                        //-----------------------------------------
                        || (rec->get_dataType()==DATA_SNOW_DEPTH
                            && rec->get_levelType()==DATA_LV_GND_SURF && rec->get_levelValue()==0)
                        //-----------------------------------------
                        || (rec->get_dataType()==DATA_SNOW_CATEG
                            && rec->get_levelType()==DATA_LV_GND_SURF && rec->get_levelValue()==0)
                        //-----------------------------------------
                        || (rec->get_dataType()==DATA_FRZRAIN_CATEG
                            && rec->get_levelType()==DATA_LV_GND_SURF && rec->get_levelValue()==0)
                        //-----------------------------------------
                        || (rec->get_dataType()==DATA_CLOUD_TOT
                            && rec->get_levelType()==DATA_LV_ATMOS_ALL && rec->get_levelValue()==0)
                        //-----------------------------------------
                        || (rec->get_dataType()==DATA_HUMID_REL
                            && rec->get_levelType()==DATA_LV_ABOV_GND && rec->get_levelValue()==2)
                        || (rec->get_dataType()==DATA_HUMID_REL
                            && rec->get_levelType()==DATA_LV_ISOBARIC
                            && (   rec->get_levelValue()==850
                                   || rec->get_levelValue()==700
                                   || rec->get_levelValue()==500
                                   || rec->get_levelValue()==300
                                   || rec->get_levelValue()==200 ) )
                        //-----------------------------------------
                        || (rec->get_dataType()==DATA_TEMP_POT
                            && rec->get_levelType()==DATA_LV_SIGMA && rec->get_levelValue()==9950)
                        //-----------------------------------------
                        || (rec->get_dataType()==DATA_CAPE
                            && rec->get_levelType()==DATA_LV_GND_SURF && rec->get_levelValue()==0)
                        || (rec->get_dataType()==DATA_CIN
                                && rec->get_levelType()==DATA_LV_GND_SURF && rec->get_levelValue()==0)
                        //-----------------------------------------
                        || (rec->get_dataType()==DATA_WAVES
                            && rec->get_levelType()==DATA_LV_GND_SURF && rec->get_levelValue()==0)
                        )
                {
                    recAdded=true;
                    addRecord(rec);
                }
#if 1
                else
                {
                    qWarning()<<"GribReader: unknown record type: key="<<(int)rec->get_dataKey();
                    qWarning()<<"dataType="<<rec->get_dataType()<< " - V1: " << rec->get_dataTypeV1();
                    qWarning()<<"levelType"<<rec->get_levelType()<< " - V1: " << rec->get_levelTypeV1();;
                    qWarning()<<"levelValue"<<rec->get_levelValue();
                }
#endif
            }
        }

        if(!recAdded) {
            delete rec;
            rec=NULL;
        }
    }
    if(fptr)
        zu_close(fptr);

    qWarning() << "GRIBV1 load finished";
    qWarning() << "NB key: " << mapGribRecords.size();
    qWarning() << "List:";
    std::map <long int, std::vector<GribRecord *>* >::iterator it;
    int i=0;
    for(it=mapGribRecords.begin();it!=mapGribRecords.end();++it) {
        qWarning() << "key " << i << ": key= " << it->first << ", nb elem" << it->second->size();
        ++i;
    }

    return true;
}
