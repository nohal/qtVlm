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

#include <grib2.h>

#include "GribV2.h"
#include "GribV2Record.h"
#include "IsoLine.h"
#include "Util.h"

GribV2::GribV2(DataManager * dataManager): Grib(dataManager) {
    version=2;
}

bool GribV2::isGribV2(QString fileName) {
    if(fileName=="") return false;

    std::string fname = qPrintable(fileName);
    g2int lskip,lgrib;
    unsigned char *cgrib; // msg buffer
    g2int ierr,listsec0[3],listsec1[13],numfields,numlocal;

    FILE * fptr=fopen(fname.c_str(),"rb");

    if(!fptr) {
        qWarning() << "Can't open Grib2 file (in isGribV2): " << fileName;
        return false;
    }

    seekgb(fptr,0,32000,&lskip,&lgrib);

    if(lgrib==0) return false;

    cgrib=(unsigned char *)malloc(lgrib);

    fseek(fptr,lskip,SEEK_SET);
    fread(cgrib,sizeof(unsigned char),lgrib,fptr);

    ierr=g2_info(cgrib,listsec0,listsec1,&numfields,&numlocal);
    if(ierr) {
        qWarning() << "(isGrib2) g2_info error num=" << ierr;
        if(fptr) fclose(fptr);
        return false;
    }

    if(fptr) fclose(fptr);

    return true;
}

bool GribV2::loadFile(QString fileName) {

    FILE * fptr=NULL;
    int msg=0;

    this->fileName=fileName;

    QTime tLoad;
    int m_sec_readCgrib=0;
    int m_sec_ginfo=0;
    int m_sec_g2_getfld=0;
    int m_sec_grecConst=0;
    int m_sec_endLoop=0;

    qWarning() << "GV2 loading " << fileName;

    g2int lskip=0,lgrib=0,iseek=0;
    unsigned char *cgrib; // msg buffer
    g2int ierr,listsec0[3],listsec1[13],numfields,numlocal;


    std::string fname = qPrintable(fileName);
    if(fileName == "") return false;

    gribfield  *gfld=NULL;

    ok=false;

    fptr=fopen(fname.c_str(),"rb");
    if(!fptr) {
        qWarning() << "Can't open Grib2 file (in loadFile): " << fileName;
        return false;
    }

    fseek(fptr,0,SEEK_END);
    fileSize=ftell(fptr);
    rewind(fptr);

    /* clean data structure + iso lines */
    clean_all_vectors();
    Util::cleanListPointers(listIsobars);
    Util::cleanListPointers(listIsotherms0);

    for(;;) {

        msg++;

        seekgb(fptr,iseek,32000,&lskip,&lgrib);
        if (lgrib == 0) break;    // end loop at EOF or problem

        cgrib=(unsigned char *)malloc(lgrib);

        fseek(fptr,lskip,SEEK_SET);
        tLoad.start();
        fread(cgrib,sizeof(unsigned char),lgrib,fptr);
        m_sec_readCgrib+=tLoad.elapsed();
        //qWarning() << "Size of cgrib: " << lgrib << ", skip=" << lskip;
        //qWarning() << "Bytes read from file: " << bRead;
        //qWarning() << "EOF=" << feof(fptr) << ", ferror=" << ferror(fptr);
        //qWarning() << "File pos=" << ftell(fptr);
        //qWarning() << "End of grib=" << cgrib[lgrib-4] << cgrib[lgrib-3] << cgrib[lgrib-2] << cgrib[lgrib-1];

        iseek=lskip+lgrib;

        tLoad.start();
        ierr=g2_info(cgrib,listsec0,listsec1,&numfields,&numlocal);
        m_sec_ginfo+=tLoad.elapsed();
        if(ierr) {
            qWarning() << "msg " << msg << ": g2_info error num=" << ierr;
            fclose(fptr);
            return false;
        }



        // accepting only GRIB2 with discipline=0 => Meteorological product (table 0.0)
        if(listsec0[1]!=2 || (listsec0[0]!=0 && listsec0[0]!=10)) {
            qWarning() << "msg " << msg << ": wrong version " << listsec0[1] << ", or discipline: " << listsec0[0];
            continue;
        }

        if(listsec1[4]!=1) {
            qWarning() << "msg " << msg << ": wrong reference time type: " << listsec1[4];
            continue;
        }

        /* loop on th fields => 1 field = 1 GribRecord */
        //qWarning() << "nb fields=" << numfields << ", nb locals=" << numlocal;

        for(int i=0;i<numfields;++i) {
            tLoad.start();
            ierr=g2_getfld(cgrib,i+1,GRB2_UNPACK,GRB2_EXPAND,&gfld);
            m_sec_g2_getfld+=tLoad.elapsed();
            if(ierr) {
                qWarning() << "msg=" << msg << "- field=" << i << ": g2_getfld error num=" << ierr;
                continue;
            }
            tLoad.start();
            GribV2Record * record = new GribV2Record(gfld,msg,i);
            m_sec_grecConst+=tLoad.elapsed();
            tLoad.start();
            if(record && record->isOk() && record->isDataKnown())
                addRecord(record);
            else
                if(record) delete record;
            g2_free(gfld);
            m_sec_endLoop+=tLoad.elapsed();
        }
        free(cgrib);
    }

    if(fptr) fclose(fptr);

    qWarning() << "GRIBV2 load finished";
    qWarning() << "NB key: " << mapGribRecords.size();
    qWarning() << "List:";
    std::map <long int, QMap<time_t,GribRecord *>* >::iterator it;
    int i=0;
    for(it=mapGribRecords.begin();it!=mapGribRecords.end();++it) {
        qWarning() << "key " << i << ": key= " << it->first << ", nb elem" << it->second->size();
        ++i;
    }

    qWarning() << "Time stat:";
    qWarning() << "\t read Cgrib: " << m_sec_readCgrib;
    qWarning() << "\t call gInfo: " << m_sec_ginfo;
    qWarning() << "\t call getFld: " << m_sec_g2_getfld;
    qWarning() << "\t const GribRecordV2: " << m_sec_grecConst;
    qWarning() << "\t End loop: " << m_sec_endLoop;

    createDewPointData();

    ok=true;
    return true;
}

/*
GrbType GribV2::dataToGrb(int data) {
    GrbType res;
    res.undef=false;

    if(data < 0 || data > 255) {
        res.undef=true;
        return res;
    }
    if(DATA_TO_GRBV2_CAT[data]==DATA_NOTDEF) {
        res.undef=true;
        return res;
    }

    res.undef=false;
    res.cat=DATA_TO_GRBV2_CAT[data];
    res.num=DATA_TO_GRBV2_NUM[data];
    return res;
}
*/
