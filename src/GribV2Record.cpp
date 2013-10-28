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

#include "GribV2Record.h"

QMap<grb2DataType,int> GRBV2_TO_DATA;
grb2DataType DATA_TO_GRBV2[256];
int GRBV2_TO_DATA_LV[256];
int DATA_TO_GRBV2_LV[256];

GribV2Record::GribV2Record(GribV2Record &rec)
{
    *this = rec;
    data=new float[dataSize];
    for(int i=0;i<dataSize;++i)
        data[i]=rec.get_data(i);
    bmap=new bool[bmapSize];
    for(int i=0;i<bmapSize;++i)
        bmap[i]=rec.get_bmap(i);
}

GribV2Record::GribV2Record(gribfield  *gfld, int msg, int field):GribRecord() {
    data=NULL;
    bmap=NULL;
    ok=false;
    knownData=false;
    dataSize=0;
    bmapSize=0;
    isFull = false;

    /*******************
     *Version & model *
     *******************/

    editionNumber=gfld->version;
    if(editionNumber!=2) {
        qWarning() << "Msg " << msg << " - field " << field << ": bad version number: " << editionNumber;
        return;
    }

    idCenter=gfld->idsect[0];
    idModel=gfld->ipdtmpl[4];
    idGrid=gfld->griddef;

    /*************************
     * Time/date - section 1 *
     *************************/

    if(gfld->idsect[4]!=1) {
        qWarning() << "Msg " << msg << " - field " << field << ": unsupported reference time type: " << gfld->idsect[4];
        return;
    }
    refYear=gfld->idsect[5];
    refMonth=gfld->idsect[6];
    refDay=gfld->idsect[7];
    refHour=gfld->idsect[8];
    refMinute=gfld->idsect[9];
    refSecond=gfld->idsect[10];
    refDate=makeDate(refYear,refMonth,refDay,refHour,refMinute,refSecond);
    // curdate computation

    int timeOffset=computeTimeOffset(gfld->ipdtmpl[7],gfld->ipdtmpl[8]);

    //qWarning() << "T offset " << timeOffset;
    if(timeOffset==-1) {
        qWarning() << "Msg " << msg << " - field " << field << ": bad or unsupported time unit for offset: " << timeOffset;
        return;
    }
    curDate=makeDate(refYear,refMonth,refDay,refHour,refMinute,refSecond+timeOffset);
    deltaPeriod=timeOffset/3600;

    /**************************
     * Coord data - section 3 *
     **************************/

    // REM: il faut peut etre retirer ce test pour avoir certaines valeurs 255 par ex
    if(gfld->griddef !=0) { // not defined in igdtnum
        qWarning() << "Msg " << msg << " - field " << field << ": gridDef not set to 0: " << gfld->griddef;
        return;
    }

    gridType=gfld->igdtnum;
    if(gridType !=0) { //only support for latitude/longitude grid
        qWarning() << "Msg " << msg << " - field " << field << ": unsupported grid type: " << gridType;
        return;
    }

    Ni  = gfld->igdtmpl[7];
    Nj  = gfld->igdtmpl[8];
    La1 = gfld->igdtmpl[11]/1000000.0;
    Lo1 = gfld->igdtmpl[12]/1000000.0;
    La2 = gfld->igdtmpl[14]/1000000.0;
    Lo2 = gfld->igdtmpl[15]/1000000.0;
    Di  = gfld->igdtmpl[16]/1000000.0;
    Dj  = gfld->igdtmpl[17]/1000000.0;

    //for(int i=0;i<19;++i)
    //    qWarning() << "igdtmpl " << i << ": " << gfld->igdtmpl[i];

    if (Lo1>=0 && Lo1<=180 && Lo2<0)
        Lo2 += 360.0;    // cross the 180 deg meridien,beetwen alaska and russia

    while ( Lo1> Lo2   &&  Di >0)   // horizontal size > 360 °
        Lo1 -= 360.0;

    double val=Lo2+Di;
    while(val>=360) val-=360;
    if(val==Lo1)
        isFull=true;

    // Bit representation: 12345678
    // => bit 8 set => value of 1
    resolFlags = gfld->igdtmpl[13];

    // Bit 3: i inc given, Bit 4 same for j
    // Bit 5:
    // 0: Resolved u and v components of vector quantities relative to easterly and northerly directions
    // 1: Resolved u and v components of vector quantities relative to the defined grid in the direction
    //    of increasing x and y (or i and j) coordinates, respectively.
    hasDiDj = ((resolFlags&0x20) !=0 && (resolFlags&0x10) !=0);

    //qWarning() << "hasDiDJ: " << hasDiDj;

    scanFlags = gfld->igdtmpl[18];
    // Bit 1:
    // 0: Points in the first row or column scan in the +i (+x) direction
    // 1: Points in the first row or column scan in the -i (-x) direction
    // Bit 2:
    // 0: Points in the first row or column scan in the -j (-y) direction
    // 1: Points in the first row or column scan in the +j (+y) direction
    // Bit 3:
    // 0: Adjacent points in the i (x) direction are consecutive
    // 1: Adjacent points in the j (y) direction are consecutive
    // Bit 4:
    // 0: All rows scan in the same direction
    // 1: Adjacent rows scan in the opposite direction
    isScanIpositive = (scanFlags&0x80) ==0;
    isScanJpositive = (scanFlags&0x40) !=0;
    isAdjacentI     = (scanFlags&0x20) ==0;

    //qWarning() << "isScanIPos=" << isScanIpositive << ", isScanJPos=" << isScanJpositive << ", isAdjacentI=" << isAdjacentI;

    /* compute min/Max */
    if (Lo2 > Lo1) {
        lonMin = Lo1;
        lonMax = Lo2;
    }
    else {
        lonMin = Lo2;
        lonMax = Lo1;
    }
    if (La2 > La1) {
        latMin = La1;
        latMax = La2;
    }
    else {
        latMin = La2;
        latMax = La1;
    }

    if (Ni<=1 || Nj<=1) {
        qWarning() << "Msg " << msg << " - field " << field << ": bad Ni/Nj val: " << Ni << ", " << Nj;
        return;
    }
    else {
        Di = (Lo2-Lo1) / (Ni-1);
        Dj = (La2-La1) / (Nj-1);
    }

    //qWarning() << "Nb point check:  ngrdpts=" << gfld->ngrdpts << ", ndpts=" << gfld->ndpts << ", Ni*Nj=" << Ni*Nj;

    /***********************************
     * Data type and level - section 4 *
     ***********************************/

    productDiscipline=gfld->discipline;
    productTemplate=gfld->ipdtnum;
    if(productTemplate!=0) {
        qWarning() << "Msg " << msg << " - field " << field << ": unsupported product template: " << productTemplate;
        return;
    }
    dataCat=gfld->ipdtmpl[0];
    dataNum=gfld->ipdtmpl[1];
    dataType=GRBV2_TO_DATA.value(grb2DataType(productDiscipline,dataCat,dataNum),DATA_NOTDEF);
    if(dataType==DATA_NOTDEF) {
        qWarning() << "Msg " << msg << " - field " << field << ": unsupported dataType: categ=" << dataCat << ", num=" << dataNum;
        return;
    }

    levelTypeV2=gfld->ipdtmpl[9];
    levelType=GRBV2_TO_DATA_LV[levelTypeV2];
    levelValue=gfld->ipdtmpl[11]*pow(10.0,-gfld->ipdtmpl[10]);
    //qWarning() << "levelVal: " << levelValue << " - fact=" << gfld->ipdtmpl[10] << ", val= " << gfld->ipdtmpl[11];

    /***********************************
     * Data representation - section 7 *
     ***********************************/

    if(gfld->idrtnum!=0 && gfld->idrtnum!=40) { // supporting simple packing and jpg packing (using jasper)
        qWarning() << "Msg " << msg << " - field " << field << ": unsupported data representation: " << gfld->idrtnum;
        return;
    }

    // REM nothing to do with ref value / scale factor as it is already done by lib

    /* check for unpack / expanded */
    if(!gfld->unpacked) {
        qWarning() << "Msg " << msg << " - field " << field << ": bitmap & data not unpacked";
        return;
    }

    if(!gfld->expanded) {
        qWarning() << "Msg " << msg << " - field " << field << ": data not expanded";
        return;
    }

    /**********************
     * Bitmap - section 6 *
     **********************/

    if(gfld->ibmap==0 || gfld->ibmap==254) { // bmap applies
        if(gfld->ngrdpts<=0) {
            qWarning() << "Msg " << msg << " - field " << field
                     << ": empty bmap array (size=" << gfld->ndpts << ")"
                     << ", while in bmap mode: " << gfld->ibmap;
            return;
        }
        bmap=new bool[gfld->ngrdpts];
        if(!bmap) {
            qWarning() << "Msg " << msg << " - field " << field << ": unable to allocate mem for data array (size=" << gfld->ngrdpts << ")";
            return;
        }
        bmapSize=gfld->ngrdpts;
        for(int i=0;i<gfld->ngrdpts;++i)
            bmap[i]=gfld->bmap[i]!=0;

    }
    else {
        if(gfld->ibmap==255) // no bmap applies
            bmap=NULL;
        else {
            qWarning() << "Msg " << msg << " - field " << field << ": unsupported bmap indicator: " << gfld->ibmap;
            return;
        }
    }

    /********************
     * Data - section 7 *
     ********************/
#if 0
    if(gfld->ndpts<=0) {
        qWarning() << "Msg " << msg << " - field " << field << ": empty data array (size=" << gfld->ndpts << ")";
        return;
    }
#endif

#if 0
    data=new float[gfld->ndpts];
#else
    data=new float[gfld->ngrdpts];
#endif
    if(!data) {
        qWarning() << "Msg " << msg << " - field " << field << ": unable to allocate mem for data array (size=" << gfld->ndpts << ")";
        return;
    }

    ok=true;

#if 0
    qWarning() << "ndpts=" << gfld->ndpts << ", ngrdpts=" << gfld->ngrdpts << ", Ni*Nj=" << Ni*Nj;
    dataSize=gfld->ndpts;
    for(int i=0;i<gfld->ndpts;++i) // not using std::copy as we do float => double conversion
        data[i]=gfld->fld[i];
#else
    dataSize=gfld->ngrdpts;
        unsigned int i, j, k;
        int ind;
        k=0;
        if (isAdjacentI) {
            for (j=0; j<Nj; j++) {
                for (i=0; i<Ni; i++) {
                    if (!hasDiDj && !isScanJpositive) {
                        ind = (Nj-1 -j)*Ni+i;
                    }
                    else {
                        ind = j*Ni+i;
                    }
                    if (hasValue(i,j)) {
                        data[ind] = gfld->fld[k];
                        k++;
                    }
                    else {
                        //qWarning() << ind << ": not def";
                        data[ind] = GRIB_NOTDEF;
                    }
                }
            }
        }
        else {
            for (i=0; i<Ni; i++) {
                for (j=0; j<Nj; j++) {
                    if (!hasDiDj && !isScanJpositive) {
                        ind = (Nj-1 -j)*Ni+i;
                    }
                    else {
                        ind = j*Ni+i;
                    }
                    if (hasValue(i,j)) {
                        data[ind] = gfld->fld[k];
                        k++;
                    }
                    else {
                        //qWarning() << ind << ": not def";
                        data[ind] = GRIB_NOTDEF;
                    }
                }
            }
        }
#endif

   // ok=true;

    if(dataType!=DATA_NOTDEF && levelType!=DATA_LV_NOTDEF) {
        knownData = true;
        unitConversion();
    }
    else
        knownData = false;

    computeKey();
}

GribV2Record::~GribV2Record(void) {
    if (bmap) {
        delete [] bmap;
        bmap = NULL;
    }
}

int GribV2Record::computeTimeOffset(int type,int val) {
    int unit;
    switch (type) {
        case 0: //	Minute
            unit = 60; break;
        case 1: //	Hour
            unit = 3600; break;
        case 2: //	Day
            unit = 86400; break;
        case 10: //	3 hours
            unit = 10800; break;
        case 11: //	6 hours
            unit = 21600; break;
        case 12: //	12 hours
            unit = 43200; break;
        case 13: // Second
            unit = 1; break;
        case 3: //	Month
        case 4: //	Year
        case 5: //	Decade (10 years)
        case 6: //	Normal (30 years)
        case 7: //	Century (100 years)
        default:
            return -1;
    }
    return unit*val;
}

/***************************************************
 * Init matrix used to convert GRB2 <-> Data code
 **************************************************/

void GribV2Record::init_conversionMatrix(void) {
    /* blank all data in atrix */
    for(int i=0;i<DATA_MAX;++i) {
        DATA_TO_GRBV2[i]=grb2DataType(DATA_NOTDEF,DATA_NOTDEF,DATA_NOTDEF);
    }

    for(int i=0;i<DATA_LV_MAX;++i)
        DATA_TO_GRBV2_LV[i]=DATA_LV_NOTDEF;

    for(int i=0;i<256;++i)
        GRBV2_TO_DATA_LV[i]=DATA_LV_NOTDEF;


    /* init the DATA => GRB matrix */
    DATA_TO_GRBV2[DATA_PRESSURE] = grb2DataType(0,3,0);
    DATA_TO_GRBV2[DATA_GEOPOT_HGT]= grb2DataType(0,3,5);
    DATA_TO_GRBV2[DATA_TEMP] = grb2DataType(0,0,0);
    DATA_TO_GRBV2[DATA_TEMP_POT] = grb2DataType(0,0,2);
    DATA_TO_GRBV2[DATA_TMAX] = grb2DataType(0,0,4);
    DATA_TO_GRBV2[DATA_TMIN] = grb2DataType(0,0,5);
    DATA_TO_GRBV2[DATA_DEWPOINT] = grb2DataType(0,0,6);
    DATA_TO_GRBV2[DATA_WIND_VX] = grb2DataType(0,2,2);
    DATA_TO_GRBV2[DATA_WIND_VY] = grb2DataType(0,2,3);
    DATA_TO_GRBV2[DATA_CURRENT_VX] = grb2DataType(10,1,2);
    DATA_TO_GRBV2[DATA_CURRENT_VY] = grb2DataType(10,1,3);
    DATA_TO_GRBV2[DATA_HUMID_SPEC] = grb2DataType(0,1,0);
    DATA_TO_GRBV2[DATA_HUMID_REL] = grb2DataType(0,1,1);
    DATA_TO_GRBV2[DATA_PRECIP_RATE] = grb2DataType(0,1,7);
    DATA_TO_GRBV2[DATA_PRECIP_TOT] = grb2DataType(0,1,8);
    DATA_TO_GRBV2[DATA_SNOW_DEPTH] = grb2DataType(0,1,11);
    DATA_TO_GRBV2[DATA_CLOUD_TOT] = grb2DataType(0,6,1);
    DATA_TO_GRBV2[DATA_FRZRAIN_CATEG] = grb2DataType(0,1,193);
    DATA_TO_GRBV2[DATA_SNOW_CATEG] = grb2DataType(0,1,195);
    DATA_TO_GRBV2[DATA_CIN] = grb2DataType(0,7,7);
    DATA_TO_GRBV2[DATA_CAPE] = grb2DataType(0,7,6);
    // waves
    DATA_TO_GRBV2[DATA_WAVES_SIG_HGT_COMB] = grb2DataType(10,0,3);
    DATA_TO_GRBV2[DATA_WAVES_WND_DIR] = grb2DataType(10,0,4);
    DATA_TO_GRBV2[DATA_WAVES_WND_HGT] = grb2DataType(10,0,5);
    DATA_TO_GRBV2[DATA_WAVES_WND_PERIOD] = grb2DataType(10,0,6);
    DATA_TO_GRBV2[DATA_WAVES_SWL_DIR] = grb2DataType(10,0,7);
    DATA_TO_GRBV2[DATA_WAVES_SWL_HGT] = grb2DataType(10,0,8);
    DATA_TO_GRBV2[DATA_WAVES_SWL_PERIOD] = grb2DataType(10,0,9);
    DATA_TO_GRBV2[DATA_WAVES_PRIM_DIR] = grb2DataType(10,0,10);
    DATA_TO_GRBV2[DATA_WAVES_PRIM_PERIOD] = grb2DataType(10,0,11);
    DATA_TO_GRBV2[DATA_WAVES_SEC_DIR] = grb2DataType(10,0,12);
    DATA_TO_GRBV2[DATA_WAVES_SEC_PERIOD] = grb2DataType(10,0,13);


    /* init LV DATA=>GRB */
    DATA_TO_GRBV2_LV[DATA_LV_GND_SURF]=1;
    DATA_TO_GRBV2_LV[DATA_LV_ISOTHERM0]=4;
    DATA_TO_GRBV2_LV[DATA_LV_ISOBARIC]=100;
    DATA_TO_GRBV2_LV[DATA_LV_MSL]=101;
    DATA_TO_GRBV2_LV[DATA_LV_ABOV_GND]=103;
    DATA_TO_GRBV2_LV[DATA_LV_SIGMA]=104;
    DATA_TO_GRBV2_LV[DATA_LV_ATMOS_ALL]=200;
    DATA_TO_GRBV2_LV[DATA_LV_ORDERED_SEQUENCE_DATA]=241;

    /* init GRB => Data matrix */
    for(int i=0;i<DATA_MAX;++i) {
        if(DATA_TO_GRBV2[i].discipline!=DATA_NOTDEF)
            GRBV2_TO_DATA.insert(DATA_TO_GRBV2[i],i);
    }

    for(int i=0;i<DATA_LV_MAX;++i) {
        if(DATA_TO_GRBV2_LV[i]!=DATA_LV_NOTDEF)
            GRBV2_TO_DATA_LV[DATA_TO_GRBV2_LV[i]]=i;
    }

}

