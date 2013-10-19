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


#include <stdlib.h>

#include <QDebug>
#include <QDateTime>

#include "GribV1Record.h"

int GRBV1_TO_DATA[256];
int DATA_TO_GRBV1[DATA_MAX];
int GRBV1_TO_DATA_LV[256];
int DATA_TO_GRBV1_LV[DATA_LV_MAX];

//-------------------------------------------------------------------------------
// Constructeur de recopie
//-------------------------------------------------------------------------------
GribV1Record::GribV1Record(const GribV1Record &rec)
{
    *this = rec;
    // recopie les champs de bits
    if (rec.data != NULL) {
        int size = rec.Ni*rec.Nj;
        this->data = new float[size];
        for (int i=0; i<size; i++)
            this->data[i] = rec.data[i];
    }
    if (rec.BMSbits != NULL) {
        int size = rec.sectionSize3-6;
        this->BMSbits = new zuchar[size];
        for (int i=0; i<size; i++)
            this->BMSbits[i] = rec.BMSbits[i];
    }
}
//-------------------------------------------------------------------------------
GribV1Record::GribV1Record(ZUFILE* file) : GribRecord()
{
    seekStart = zu_tell(file);
    data    = NULL;
    BMSbits = NULL;
    eof     = false;
    isFull = false;
    knownData = true;

    ok = readGribSection0_IS(file);

    if (ok) {
        ok = readGribSection1_PDS(file);
        zu_seek(file, fileOffset1+sectionSize1, SEEK_SET);
    }
    if (ok) {
        ok = readGribSection2_GDS(file);
        zu_seek(file, fileOffset2+sectionSize2, SEEK_SET);
    }
    if (ok) {
        ok = readGribSection3_BMS(file);
        zu_seek(file, fileOffset3+sectionSize3, SEEK_SET);
    }
    if (ok) {
        ok = readGribSection4_BDS(file);
        zu_seek(file, fileOffset4+sectionSize4, SEEK_SET);
    }
    if (ok) {
        ok = readGribSection5_ES(file);
    }
    if (ok) {
        zu_seek(file, seekStart+totalSize, SEEK_SET);
    }

    if (ok) {
        if(dataType!=DATA_NOTDEF && levelType!=DATA_LV_NOTDEF) {
            knownData = true;
            unitConversion();
        }
        else
            knownData = false;

        computeKey();
    }
}

#if 0
void  GribV1Record::translateDataType()
{
    this->knownData = true;


    if(this->dataType==DATA_CURRENT_VX || this->dataType==DATA_CURRENT_VY)
    {
        this->levelType=DATA_LV_MSL;
        this->levelValue=0;
    }

    //qWarning()<<idCenter<<idModel<<idGrid<<dataType<<levelType<<levelValue;
    //------------------------
    // NOAA GFS
    //------------------------
    if (idCenter==7 && (idModel==96 || idModel==81) && (idGrid==4 || idGrid==255))
    {
        if (dataType == DATA_PRECIP_TOT)
        {	// mm/period -> mm/h
            if (periodP2 > periodP1)
                multiplyAllData( 1.0/(periodP2-periodP1) );
        }
        if (dataType == DATA_PRECIP_RATE)
        {	// mm/s -> mm/h
            if (periodP2 > periodP1)
                multiplyAllData( 3600.0 );
        }


    }
    //------------------------
    // NOAA Waves
    //------------------------
    else if (
             (idCenter==7 && idModel==122 && idGrid==239)  // akw.all.grb
             || (idCenter==7 && idModel==124 && idGrid==253)  // enp.all.grb
             || (idCenter==7 && idModel==123 && idGrid==244)  // nah.all.grb
             || (idCenter==7 && idModel==125 && idGrid==253)  // nph.all.grb
             || (idCenter==7 && idModel==88 && idGrid==233)	  // nwww3.all.grb
             || (idCenter==7 && idModel==121 && idGrid==238)  // wna.all.grb
             || (idCenter==7 && idModel==88 && idGrid==255))   // saildocs
    {
        if ( (get_dataType()==DATA_WIND_VX || get_dataType()==DATA_WIND_VY)
             && get_levelType()==DATA_LV_GND_SURF
             && get_levelValue()==1)
        {
            levelType  = DATA_LV_ABOV_GND;
            levelValue = 10;
        }

        if ( (get_dataType() == DATA_WAVES_SIG_HGT_COMB
              || get_dataType()==DATA_WAVES_WND_DIR || get_dataType()==DATA_WAVES_WND_HGT || get_dataType()==DATA_WAVES_WND_PERIOD
              || get_dataType()==DATA_WAVES_SWL_DIR || get_dataType()==DATA_WAVES_SWL_HGT || get_dataType()==DATA_WAVES_SWL_PERIOD
              || get_dataType()==DATA_WAVES_MAX_DIR || get_dataType()==DATA_WAVES_MAX_HGT || get_dataType()==DATA_WAVES_MAX_PERIOD
              || get_dataType()==DATA_WAVES_PRIM_DIR || get_dataType()==DATA_WAVES_PRIM_PERIOD
              || get_dataType()==DATA_WAVES_SEC_DIR || get_dataType()==DATA_WAVES_SEC_PERIOD
              || get_dataType()==DATA_WAVES_WHITE_CAP
              )
             && (get_levelType()==DATA_LV_ORDERED_SEQUENCE_DATA || get_levelType()==DATA_LV_GND_SURF)
             && get_levelValue()==1)
        {
            levelType  = DATA_LV_GND_SURF;
            levelValue = 0;
        }
    }
    //------------------------
    // WRF NMM grib.meteorologic.net
    //------------------------
    else if (idCenter==7 && idModel==89 && idGrid==255)
    {
        if (dataType == DATA_PRECIP_TOT) {	// mm/period -> mm/h
            if (periodP2 > periodP1)
                multiplyAllData( 1.0/(periodP2-periodP1) );
        }
        if (dataType == DATA_PRECIP_RATE) {	// mm/s -> mm/h
            if (periodP2 > periodP1)
                multiplyAllData( 3600.0 );
        }


    }
    //------------------------
    // Meteorem
    //------------------------
    else if (idCenter==59 && idModel==78 && idGrid==255)
    {
        if ( (get_dataType()==DATA_WIND_VX || get_dataType()==DATA_WIND_VY)
             && get_levelType()==DATA_LV_MSL
             && get_levelValue()==0)
        {
            levelType  = DATA_LV_ABOV_GND;
            levelValue = 10;
        }
        if ( get_dataType()==DATA_PRECIP_TOT
             && get_levelType()==DATA_LV_MSL
             && get_levelValue()==0)
        {
            levelType  = DATA_LV_GND_SURF;
            levelValue = 0;
        }
    }
    //---------------
    //Navimail
    //---------------
    else if ( (idCenter==85 && idModel==5 && idGrid==255)
              ||(idCenter==85 && idModel==83 && idGrid==255)
              ||(idCenter==85 && idModel==86 && idGrid==255) )
    {

    }
    //---------------
    //PredictWind
    //---------------
    else if (idCenter==255 && idModel==1 && idGrid==255)
    {
    }
    //---------------
    //Theyr.com
    //---------------
    else if (idCenter==131 && idModel==75 && idGrid==237)
    {

    }
    //---------------
    //NOGAPS model
    //---------------
    else if (idCenter==58 && idModel==58 && idGrid==255)    {
        /* Nothing to do */
    }
    //---------------
    //TydeTech.com
    //---------------
    else if (idCenter==0 && idModel==0 && idGrid==255)
    {
    }
    //---------------
    //Actimar: courants bretagne, contient les vagues aussi (exclues pour le moment)
    //---------------
    else if (idCenter==255 && idModel==220 && idGrid==255)
    {
    }
    //---------------
    //Navimail-Mercator
    //---------------
    else if (idCenter==85 && idModel==10 && idGrid==255)
    {
    }
    //---------------
    //Sarana
    //---------------
    else if (idCenter==161 && idModel==45 && idGrid==255)
    {
    }
    //---------------
    //RTOFS saildocs gulfstream data
    //---------------
    else if (idCenter==7 && idModel==45 && idGrid==255)
    {
    }
    //------------------------
    // COAMPS from saildocs (idModel ignored)
    //------------------------
    else if (idCenter==58 && idGrid==255)
    {
    }
    //------------------------
    // Current from ???
    //------------------------
    else if (idCenter==255 && idModel==2 && idGrid==255)
    {
    }
    // FNMOC WW3 mediterranean sea
    //----------------------------------------------
    else if (idCenter==58 && idModel==111 && idGrid==179)
    {
    }
    //----------------------------------------------
    // FNMOC WW3
    //----------------------------------------------
    else if (idCenter==58 && idModel==110 && idGrid==240)
    {
    }
    //------------------------
    // Unknown center
    //------------------------
    else
    {
        qWarning()<<"Unknown GribRecord: idcenter="<<idCenter<<"idModel="<<idModel<<"idGrid="<<idGrid;
        this->knownData = false;

    }
    //this->print();
}
#endif
//-------------------------------------------------------------------------------
void GribV1Record::print()
{
        QString debug;
        debug=debug.sprintf("idCenter=%d idModel=%d idGrid=%d dataType=%d hr=%f\n",
                             idCenter, idModel, idGrid, dataType,
                             (curDate-refDate)/3600.0
                             );
        qWarning()<<debug;
//        printf("%d: idCenter=%d idModel=%d idGrid=%d dataType=%d hr=%f\n",
//                        id, idCenter, idModel, idGrid, dataType,
//                        (curDate-refDate)/3600.0
//                        );
}


//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//-----------------------------------------
GribV1Record::~GribV1Record()
{
    if (data) {
        delete [] data;
        data = NULL;
    }
    if (BMSbits) {
        delete [] BMSbits;
        BMSbits = NULL;
    }
}

//==============================================================
// Lecture des donnees
//==============================================================
//----------------------------------------------
// SECTION 0: THE INDICATOR SECTION (IS)
//----------------------------------------------
bool GribV1Record::readGribSection0_IS(ZUFILE* file) {
    char    strgrib[4];
    memset (strgrib, 0, sizeof (strgrib));
    zuint initFoffset;
    fileOffset0 = zu_tell(file);
    initFoffset=fileOffset0;
#if 1
    char buf[1];
    memset (buf, 0, sizeof (buf));
    while(true)
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
        strgrib[0]='G';
        strgrib[1]='R';
        strgrib[2]='I';
        strgrib[3]='B';
        break;
    }
#else
    while((zu_read(file, strgrib, 4) == 4) &&
          (strgrib[0] != 'G' || strgrib[1] != 'R' ||
           strgrib[2] != 'I' || strgrib[3] != 'B')) {
          zu_seek(file,++fileOffset0,SEEK_SET);
    }
#endif
    if(strgrib[0] != 'G' || strgrib[1] != 'R' ||
            strgrib[2] != 'I' || strgrib[3] != 'B') {
        if((fileOffset0-10)>initFoffset) // displaying error msg only if we are really far from initial offset
            qWarning() << "Can't find next record / EOF - offset=" << fileOffset0 << ", initOffset=" << initFoffset ;
        ok = false;
        eof = true;
        return false;
    }


    seekStart=zu_tell(file)-4;
    totalSize = readInt3(file);


    //qWarning() << "Start = " << seekStart << ", size = " << totalSize;

    editionNumber = readChar(file);
    if (editionNumber != 1)  {
        ok = false;
        eof = true;
        return false;
    }

    return true;
}
//----------------------------------------------
// SECTION 1: THE PRODUCT DEFINITION SECTION (PDS)
//----------------------------------------------
bool GribV1Record::readGribSection1_PDS(ZUFILE* file) {
    fileOffset1 = zu_tell(file);
    if (zu_read(file, data1, 28) != 28) {
        ok=false;
        eof = true;
        return false;
    }
    sectionSize1 = makeInt3(data1[0],data1[1],data1[2]);
    tableVersion = data1[3];
    idCenter = data1[4];
    idModel  = data1[5];
    idGrid   = data1[6];
    hasGDS = (data1[7]&128)!=0;
    hasBMS = (data1[7]&64)!=0;

    dataTypeV1 = data1[8];	 // octet 9 = parameters and units
    /* convert data type */
    dataType=GRBV1_TO_DATA[dataTypeV1];
    levelTypeV1 = data1[9];
    /* convert level type */
    levelType=GRBV1_TO_DATA_LV[levelTypeV1];
    levelValue = makeInt2(data1[10],data1[11]);

    refYear   = (data1[24]-1)*100+data1[12];
    refMonth  = data1[13];
    refDay    = data1[14];
    refHour   = data1[15];
    refMinute = data1[16];

    refDate = makeDate(refYear,refMonth,refDay,refHour,refMinute,0);

    periodP1  = data1[18];
    periodP2  = data1[19];
    timeRange = data1[20];
    periodsec = periodSeconds(data1[17],data1[18],data1[19],timeRange);
    if(periodP2 > periodP1)
        deltaPeriod=periodP2-periodP1;
    //qWarning() << "Periodsec:" << periodP2-periodP1 << " - comp= " << periodsec;
    curDate = makeDate(refYear,refMonth,refDay,refHour,refMinute,periodsec);

//if (dataType == GRB_PRECIP_TOT) printf("P1=%d p2=%d\n", periodP1,periodP2);

    int decim;
    decim = (int)(((((zuint)data1[26]&0x7F)<<8)+(zuint)data1[27])&0x7FFF);
    if (data1[26]&0x80)
        decim *= -1;
    decimalFactorD = pow(10.0, decim);

    // Controls
    if (! hasGDS) {
        //erreur("Record: GDS not found");
        ok = false;
    }
    if (decimalFactorD == 0) {
        //erreur("Record: decimalFactorD null");
        ok = false;
    }
    return ok;
}
//----------------------------------------------
// SECTION 2: THE GRID DESCRIPTION SECTION (GDS)
//----------------------------------------------
bool GribV1Record::readGribSection2_GDS(ZUFILE* file) {
    if (! hasGDS)
        return 0;
    fileOffset2 = zu_tell(file);
    sectionSize2 = readInt3(file);  // byte 1-2-3
    readChar(file);			// byte 4 => NV
    readChar(file); 			// byte 5 => PV
    gridType = readChar(file); 		// byte 6

    if (gridType != 0) {
        erreur("Record: unknown grid type GDS(6) : %d",gridType);
        ok = false;
    }

    Ni  = readInt2(file);				// byte 7-8
    Nj  = readInt2(file);				// byte 9-10
    La1 = readSignedInt3(file)/1000.0;	// byte 11-12-13
    Lo1 = readSignedInt3(file)/1000.0;	// byte 14-15-16
    resolFlags = readChar(file);		// byte 17
    La2 = readSignedInt3(file)/1000.0;	// byte 18-19-20
    Lo2 = readSignedInt3(file)/1000.0;	// byte 21-22-23

    if (Lo1>=0 && Lo1<=180 && Lo2<0) {
        Lo2 += 360.0;    // cross the 180 deg meridien,beetwen alaska and russia
    }

    Di  = readSignedInt2(file)/1000.0;	// byte 24-25
    Dj  = readSignedInt2(file)/1000.0;	// byte 26-27

    while ( Lo1> Lo2   &&  Di >0) {   // horizontal size > 360 °
        Lo1 -= 360.0;
    }

    double val=Lo2+Di;
    while(val>=360) val-=360;
    if(val==Lo1)
        isFull=true;

    hasDiDj =(resolFlags&0x80) !=0;

    scanFlags = readChar(file);			// byte 28
    isScanIpositive = (scanFlags&0x80) ==0;
    isScanJpositive = (scanFlags&0x40) !=0;
    isAdjacentI     = (scanFlags&0x20) ==0;

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
                erreur("Recordd: Ni=%u Nj=%u",Ni,Nj);
                ok = false;
        }
        else {
                Di = (Lo2-Lo1) / (Ni-1);
                Dj = (La2-La1) / (Nj-1);
        }

if (false) {
printf("====\n");
printf("Lo1=%f Lo2=%f    La1=%f La2=%f\n", Lo1,Lo2,La1,La2);
printf("Ni=%u Nj=%u\n", Ni,Nj);
printf("hasDiDj=%d Di,Dj=(%f %f)\n", hasDiDj, Di,Dj);
printf("hasBMS=%d\n", hasBMS);
printf("isScanIpositive=%d isScanJpositive=%d isAdjacentI=%d\n",
                        isScanIpositive,isScanJpositive,isAdjacentI );
}
    return ok;
}
//----------------------------------------------
// SECTION 3: BIT MAP SECTION (BMS)
//----------------------------------------------
bool GribV1Record::readGribSection3_BMS(ZUFILE* file) {
    fileOffset3 = zu_tell(file);
    if (! hasBMS) {
        sectionSize3 = 0;
        return ok;
    }
    sectionSize3 = readInt3(file);
    (void) readChar(file);
    int bitMapFollows = readInt2(file);

    if (bitMapFollows != 0) {
        return ok;
    }
    BMSbits = new zuchar[sectionSize3-6];
    if (!BMSbits) {
        //erreur("Record: out of memory");
        ok = false;
    }
    for (zuint i=0; i<sectionSize3-6; i++) {
        BMSbits[i] = readChar(file);
    }
    return ok;
}
//----------------------------------------------
// SECTION 4: BINARY DATA SECTION (BDS)
//----------------------------------------------
bool GribV1Record::readGribSection4_BDS(ZUFILE* file) {
    fileOffset4  = zu_tell(file);
    sectionSize4 = readInt3(file);  // byte 1-2-3

    zuchar flags  = readChar(file);			// byte 4
    scaleFactorE = readSignedInt2(file);	// byte 5-6
    refValue     = readFloat4(file);		// byte 7-8-9-10
    nbBitsInPack = readChar(file);			// byte 11
    scaleFactorEpow2 = pow(2.0,scaleFactorE);
    unusedBitsEndBDS = flags & 0x0F;
    isGridData      = (flags&0x80) ==0;
    isSimplePacking = (flags&0x80) ==0;
    isFloatValues   = (flags&0x80) ==0;

//printf("BDS type=%3d - bits=%02d - level %3d - %d\n", dataType, nbBitsInPack, levelType,levelValue);

    if (! isGridData) {
        //erreur("Record: need grid data");
        ok = false;
    }
    if (! isSimplePacking) {
        //erreur("Record: need simple packing");
        ok = false;
    }
    if (! isFloatValues) {
        //erreur("Record: need double values");
        ok = false;
    }

    if (!ok) {
        return ok;
    }

    // Allocate memory for the data
    data = new float[Ni*Nj];
    if (!data) {
        //erreur("Record: out of memory");
        ok = false;
    }

    zuint  startbit  = 0;
    int  datasize = sectionSize4-11;
    zuchar *buf = new zuchar[datasize+4];  // +4 pour simplifier les decalages ds readPackedBits
    if (!buf) {
        //erreur("Record: out of memory");
        ok = false;
    }
    if (zu_read(file, buf, datasize) != datasize) {
        //erreur("Record: data read error");
        ok = false;
        eof = true;
    }
    if (!ok) {
        return ok;
    }
    // Initialize the las 4 bytes of buf since we didn't read them
    buf[datasize+0] = buf[datasize+1] = buf[datasize+2] = buf[datasize+3] = 0;

    // Read data in the order given by isAdjacentI
    zuint i, j, x;
    int ind;
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
                    x = readPackedBits(buf, startbit, nbBitsInPack);
                    data[ind] = (refValue + x*scaleFactorEpow2)/decimalFactorD;
                    startbit += nbBitsInPack;
                }
                else {
                    data[ind] = (float) GRIB_NOTDEF;
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
                    x = readPackedBits(buf, startbit, nbBitsInPack);
                    startbit += nbBitsInPack;
                    data[ind] = (refValue + x*scaleFactorEpow2)/decimalFactorD;
                }
                else {
                    data[ind] = (float)GRIB_NOTDEF;
                }
            }
        }
    }

    if (buf) {
        delete [] buf;
        buf = NULL;
    }
    return ok;
}



//----------------------------------------------
// SECTION 5: END SECTION (ES)
//----------------------------------------------
bool GribV1Record::readGribSection5_ES(ZUFILE* file) {
    char str[4];
    if (zu_read(file, str, 4) != 4) {
        ok = false;
        eof = true;
        return false;
    }
    if (strncmp(str, "7777", 4) != 0)  {
        erreur("Final 7777 not read: %c%c%c%c",str[0],str[1],str[2],str[3]);
        ok = false;
        return false;
    }
    return ok;
}

//==============================================================
// Fonctions utiles
//==============================================================
double GribV1Record::readFloat4(ZUFILE* file) {
    unsigned char t[4];
    if (zu_read(file, t, 4) != 4) {
        ok = false;
        eof = true;
        return 0;
    }

    double val;
    int A = (zuint)t[0]&0x7F;
    int B = ((zuint)t[1]<<16)+((zuint)t[2]<<8)+(zuint)t[3];

    val = pow(2.0,-24)*B*pow(16.0,A-64);
    if (t[0]&0x80)
        return -val;
    else
        return val;
}
//----------------------------------------------
zuchar GribV1Record::readChar(ZUFILE* file) {
    zuchar t;
    if (zu_read(file, &t, 1) != 1) {
        ok = false;
        eof = true;
        return 0;
    }
    return t;
}
//----------------------------------------------
int GribV1Record::readSignedInt3(ZUFILE* file) {
    unsigned char t[3];
    if (zu_read(file, t, 3) != 3) {
        ok = false;
        eof = true;
        return 0;
    }
    int val = (((zuint)t[0]&0x7F)<<16)+((zuint)t[1]<<8)+(zuint)t[2];
    if (t[0]&0x80)
        return -val;
    else
        return val;
}
//----------------------------------------------
int GribV1Record::readSignedInt2(ZUFILE* file) {
    unsigned char t[2];
    if (zu_read(file, t, 2) != 2) {
        ok = false;
        eof = true;
        return 0;
    }
    int val = (((zuint)t[0]&0x7F)<<8)+(zuint)t[1];
    if (t[0]&0x80)
        return -val;
    else
        return val;
}
//----------------------------------------------
zuint GribV1Record::readInt3(ZUFILE* file) {
    unsigned char t[3];
    if (zu_read(file, t, 3) != 3) {
        ok = false;
        eof = true;
        return 0;
    }
    return ((zuint)t[0]<<16)+((zuint)t[1]<<8)+(zuint)t[2];
}
//----------------------------------------------
zuint GribV1Record::readInt2(ZUFILE* file) {
    unsigned char t[2];
    if (zu_read(file, t, 2) != 2) {
        ok = false;
        eof = true;
        return 0;
    }
    return ((zuint)t[0]<<8)+(zuint)t[1];
}
//----------------------------------------------
zuint GribV1Record::makeInt3(zuchar a, zuchar b, zuchar c) {
    return ((zuint)a<<16)+((zuint)b<<8)+(zuint)c;
}
//----------------------------------------------
zuint GribV1Record::makeInt2(zuchar b, zuchar c) {
    return ((zuint)b<<8)+(zuint)c;
}
//----------------------------------------------
zuint GribV1Record::readPackedBits(zuchar *buf, zuint first, zuint nbBits)
{
    zuint oct = first / 8;
    zuint bit = first % 8;

    zuint val = (buf[oct]<<24) + (buf[oct+1]<<16) + (buf[oct+2]<<8) + (buf[oct+3]);
    val = val << bit;
    val = val >> (32-nbBits);
    return val;
}

//----------------------------------------------


//----------------------------------------------
zuint GribV1Record::periodSeconds(zuchar unit,zuchar P1,zuchar P2,zuchar range) {
    zuint res, dur;
    switch (unit) {
        case 0: //	Minute
            res = 60; break;
        case 1: //	Hour
            res = 3600; break;
        case 2: //	Day
            res = 86400; break;
        case 10: //	3 hours
            res = 10800; break;
        case 11: //	6 hours
            res = 21600; break;
        case 12: //	12 hours
            res = 43200; break;
        case 254: // Second
            res = 1; break;
        case 3: //	Month
        case 4: //	Year
        case 5: //	Decade (10 years)
        case 6: //	Normal (30 years)
        case 7: //	Century (100 years)
        default:
            erreur("id: unknown time unit in PDS b18=%d",unit);
            res = 0;
            ok = false;
    }
    debug("id: PDS (time range) b21=%d P1=%d P2=%d",range,P1,P2);
    dur = 0;
    switch (range) {
        case 0:
            dur = (zuint)P1; break;
        case 1:
            dur = 0; break;
        case 2:
        case 3:
            // dur = ((zuint)P1+(zuint)P2)/2; break;     // TODO
            dur = (zuint)P2; break;
         case 4:
            dur = (zuint)P2; break;
        case 10:
            dur = ((zuint)P1<<8) + (zuint)P2; break;
        default:
            erreur("id: unknown time range in PDS b21=%d",range);
            dur = 0;
            ok = false;
    }
    return res*dur;
}

/***************************************************
 * Init matrix used to convert GRB2 <-> Data code
 **************************************************/

void GribV1Record::init_conversionMatrix(void) {
    /* blank all data in matrix */
    for(int i=0;i<DATA_MAX;++i) {
        DATA_TO_GRBV1[i]=DATA_NOTDEF;
    }

    for(int i=0;i<DATA_LV_MAX;++i) {
        DATA_TO_GRBV1_LV[i]=DATA_NOTDEF;
    }

    for(int i=0;i<256;++i) {
        GRBV1_TO_DATA[i]=DATA_NOTDEF;
        GRBV1_TO_DATA_LV[i]=DATA_NOTDEF;
    }

    /* init DATA => GRB */
    DATA_TO_GRBV1[DATA_PRESSURE] = 2;
    DATA_TO_GRBV1[DATA_GEOPOT_HGT] = 7;
    DATA_TO_GRBV1[DATA_TEMP] = 11;
    DATA_TO_GRBV1[DATA_TEMP_POT] = 13;
    DATA_TO_GRBV1[DATA_TMAX] = 15;
    DATA_TO_GRBV1[DATA_TMIN] = 16;
    DATA_TO_GRBV1[DATA_DEWPOINT] = 17;
    DATA_TO_GRBV1[DATA_WIND_VX] = 33;
    DATA_TO_GRBV1[DATA_WIND_VY] = 34;
    DATA_TO_GRBV1[DATA_CURRENT_VX] = 49;
    DATA_TO_GRBV1[DATA_CURRENT_VY] = 50;
    DATA_TO_GRBV1[DATA_HUMID_SPEC] = 51;
    DATA_TO_GRBV1[DATA_HUMID_REL] = 52;
    DATA_TO_GRBV1[DATA_PRECIP_RATE] = 59;
    DATA_TO_GRBV1[DATA_PRECIP_TOT] = 61;
    DATA_TO_GRBV1[DATA_SNOW_DEPTH] = 66;
    DATA_TO_GRBV1[DATA_CLOUD_TOT] = 71;
    DATA_TO_GRBV1[DATA_FRZRAIN_CATEG] = 141;
    DATA_TO_GRBV1[DATA_SNOW_CATEG] = 143;
    DATA_TO_GRBV1[DATA_CIN] = 156;
    DATA_TO_GRBV1[DATA_CAPE] = 157;
    DATA_TO_GRBV1[DATA_WAVES_SIG_HGT_COMB] = 100;
    DATA_TO_GRBV1[DATA_WAVES_WND_DIR] = 101;
    DATA_TO_GRBV1[DATA_WAVES_WND_HGT] = 102;
    DATA_TO_GRBV1[DATA_WAVES_WND_PERIOD] = 103;
    DATA_TO_GRBV1[DATA_WAVES_SWL_DIR] = 104;
    DATA_TO_GRBV1[DATA_WAVES_SWL_HGT] = 105;
    DATA_TO_GRBV1[DATA_WAVES_SWL_PERIOD] = 106;
    DATA_TO_GRBV1[DATA_WAVES_PRIM_DIR] = 107;
    DATA_TO_GRBV1[DATA_WAVES_PRIM_PERIOD] = 108;
    DATA_TO_GRBV1[DATA_WAVES_SEC_DIR] = 109;
    DATA_TO_GRBV1[DATA_WAVES_SEC_PERIOD] = 110;
    DATA_TO_GRBV1[DATA_WAVES_WHITE_CAP] = 155;
    DATA_TO_GRBV1[DATA_WAVES_MAX_DIR] = 207;
    DATA_TO_GRBV1[DATA_WAVES_MAX_HGT] = 220;
    DATA_TO_GRBV1[DATA_WAVES_MAX_PERIOD] = 208;

    /* init LV DATA=>GRB */
    DATA_TO_GRBV1_LV[DATA_LV_GND_SURF]=1;
    DATA_TO_GRBV1_LV[DATA_LV_ISOTHERM0]=4;
    DATA_TO_GRBV1_LV[DATA_LV_ISOBARIC]=100;
    DATA_TO_GRBV1_LV[DATA_LV_MSL]=102;
    DATA_TO_GRBV1_LV[DATA_LV_ABOV_GND]=105;
    DATA_TO_GRBV1_LV[DATA_LV_SIGMA]=107;
    DATA_TO_GRBV1_LV[DATA_LV_ATMOS_ALL]=200;
    DATA_TO_GRBV1_LV[DATA_LV_ORDERED_SEQUENCE_DATA]=241;

    for(int i=0;i<DATA_MAX;++i)
        if(DATA_TO_GRBV1[i]!=DATA_NOTDEF)
            GRBV1_TO_DATA[DATA_TO_GRBV1[i]]=i;

    for(int i=0;i<DATA_LV_MAX;++i)
        if(DATA_TO_GRBV1_LV[i]!=DATA_NOTDEF)
            GRBV1_TO_DATA_LV[DATA_TO_GRBV1_LV[i]]=i;


}

