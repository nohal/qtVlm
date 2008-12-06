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

#include "GribRecord.h"

//-------------------------------------------------------------------------------
// Constructeur de recopie
//-------------------------------------------------------------------------------
GribRecord::GribRecord(GribRecord &rec)
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
GribRecord::GribRecord(ZUFILE* file, int id_)
{
    id = id_;
    seekStart = zu_tell(file);
    data    = NULL;
    BMSbits = NULL;
    eof     = false;
    
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
}
//-----------------------------------------
GribRecord::~GribRecord()
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
// Lecture des données
//==============================================================
//----------------------------------------------
// SECTION 0: THE INDICATOR SECTION (IS)
//----------------------------------------------
bool GribRecord::readGribSection0_IS(ZUFILE* file) {
    char strgrib[4];
    fileOffset0 = zu_tell(file);
    if (zu_read(file, strgrib, 4) != 4) {
        ok = false;
        eof = true;
        return false;
    }
    if (strncmp(strgrib, "GRIB", 4) != 0)  {
        erreur("readGribSection0_IS(): Unknown file header : %c%c%c%c",
                    strgrib[0],strgrib[1],strgrib[2],strgrib[3]);
        ok = false;
        eof = true;
        return false;
    }
    totalSize = readInt3(file);

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
bool GribRecord::readGribSection1_PDS(ZUFILE* file) {
    fileOffset1 = zu_tell(file);
    if (zu_read(file, data1, 28) != 28) {
        ok=false;
        eof = true;
        return false;
    }
    sectionSize1 = makeInt3(data1[0],data1[1],data1[2]);
    tableVersion = data1[3];
    idCenter = data1[4];
    idGrid = data1[6];
    dataType = data1[8];

    hasGDS = (data1[7]&128)!=0;
    hasBMS = (data1[7]&64)!=0;

    refyear   = (data1[24]-1)*100+data1[12];
    refmonth  = data1[13];
    refday    = data1[14];
    refhour   = data1[15];
    refminute = data1[16];

    refDate = makeDate(refyear,refmonth,refday,refhour,refminute,0);
    periodP1  = data1[18];
    periodP2  = data1[19];
    timeRange = data1[20];
    periodsec = periodSeconds(data1[17],data1[18],data1[19],timeRange);
    curDate = makeDate(refyear,refmonth,refday,refhour,refminute,periodsec);
    
    int decim;
    decim = (int)(((((zuint)data1[26]&0x7F)<<8)+(zuint)data1[27])&0x7FFF);
    if (data1[26]&0x80)
        decim *= -1;
    decimalFactorD = pow(10.0, decim);

    // Controls
    if (! hasGDS) {
        erreur("Record %d: GDS not found",id);
        ok = false;
    }
    if (decimalFactorD == 0) {
        erreur("Record %d: decimalFactorD null",id);
        ok = false;
    }
    return ok;
}
//----------------------------------------------
// SECTION 2: THE GRID DESCRIPTION SECTION (GDS)
//----------------------------------------------
bool GribRecord::readGribSection2_GDS(ZUFILE* file) {
    if (! hasGDS)
        return 0;
    fileOffset2 = zu_tell(file);
    sectionSize2 = readInt3(file);  // byte 1-2-3
    NV = readChar(file);			// byte 4
    PV = readChar(file); 			// byte 5
    gridType = readChar(file); 		// byte 6

    if (gridType != 0) {
        erreur("Record %d: unknown grid type GDS(6) : %d",id,gridType);
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

// float Dii=Di;
// float Djj=Dj;

    while ( Lo1> Lo2   &&  Di >0) {   // horizontal size > 360 °
        Lo1 -= 360.0;
    }
    
    // Signed Di and Dj is better
    Di = (Lo2>Lo1) ?  fabs(Di) :  -fabs(Di);
    Dj = (La2>La1) ?  fabs(Dj) :  -fabs(Dj);

    scanFlags = readChar(file);			// byte 28

    hasDiDj = (resolFlags&0x80) !=0;
    isEarthSpheric = (resolFlags&0x40) ==0;
    isUeastVnorth =  (resolFlags&0x08) ==0;

    isScanIpositive = (scanFlags&0x80) ==0;
    isScanJpositive = (scanFlags&0x40) !=0;
    isAdjacentI     = (scanFlags&0x20) ==0;
    
//    if (! hasDiDj) {
        if (Ni<=1 || Nj<=1) {
            erreur("Record %d: not found: Di and Dj : Ni=%d Nj=%d",id,Ni,Nj);
            ok = false;
        }
        else {
            Di = (Lo2-Lo1) / (Ni-1);
            Dj = (La2-La1) / (Nj-1);
        }
//    }
/*if (id==3) {
printf("====\n");
printf("Lo1=%f Lo2=%f    La1=%f La2=%f\n", Lo1,Lo2,La1,La2);
printf("Ni=%d Nj=%d\n", Ni,Nj);
printf("hasDiDj=%d  Dii,Djj=(%f,%f)  Di,Dj=(%f %f)\n", hasDiDj, Dii,Djj, Di,Dj);
printf("hasBMS=%d\n", hasBMS);
printf("isScanIpositive=%d isScanJpositive=%d isAdjacentI=%d\n",
                        isScanIpositive,isScanJpositive,isAdjacentI );
}*/
    return ok;
}
//----------------------------------------------
// SECTION 3: BIT MAP SECTION (BMS)
//----------------------------------------------
bool GribRecord::readGribSection3_BMS(ZUFILE* file) {
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
        erreur("Record %d: out of memory",id);
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
bool GribRecord::readGribSection4_BDS(ZUFILE* file) {
    fileOffset4  = zu_tell(file);
    sectionSize4 = readInt3(file);  // byte 1-2-3
    
    zuchar flags  = readChar(file);			// byte 4
    scaleFactorE = readSignedInt2(file);	// byte 5-6
    refValue     = readFloat4(file);		// byte 7-8-9-10
    nbBitsInPack = readChar(file);			// byte 11
    scaleFactorEpow2 = pow(2,scaleFactorE);
    unusedBitsEndBDS = flags & 0x0F;
    isGridData      = (flags&0x80) ==0;
    isSimplePacking = (flags&0x80) ==0;
    isFloatValues   = (flags&0x80) ==0;

    if (! isGridData) {
        erreur("Record %d: need grid data",id);
        ok = false;
    }
    if (! isSimplePacking) {
        erreur("Record %d: need simple packing",id);
        ok = false;
    }
    if (! isFloatValues) {
        erreur("Record %d: need float values",id);
        ok = false;
    }
    
    if (!ok) {
        return ok;
    }

    // Allocate memory for the data
    data = new float[Ni*Nj];   
    if (!data) {
        erreur("Record %d: out of memory",id);
        ok = false;
    }
    
    zuint  startbit  = 0;
    int  datasize = sectionSize4-11;
    zuchar *buf = new zuchar[datasize+4];  // +4 pour simplifier les décalages ds readPackedBits
    if (!buf) {
        erreur("Record %d: out of memory",id);
        ok = false;
    }
    if (zu_read(file, buf, datasize) != datasize) {
        erreur("Record %d: data read error",id);
        ok = false;
        eof = true;
    }
    if (!ok) {
        return ok;
    }

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
                    x = readPackedBits(buf, startbit, nbBitsInPack);
                    startbit += nbBitsInPack;
                    data[ind] = (refValue + x*scaleFactorEpow2)/decimalFactorD;
                }
                else {
                    data[ind] = GRIB_NOTDEF;
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
bool GribRecord::readGribSection5_ES(ZUFILE* file) {
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
float GribRecord::readFloat4(ZUFILE* file) {
    unsigned char t[4];
    if (zu_read(file, t, 4) != 4) {
        ok = false;
        eof = true;
        return 0;
    }

    float val;
    int A = (zuint)t[0]&0x7F;
    int B = ((zuint)t[1]<<16)+((zuint)t[2]<<8)+(zuint)t[3];

    val = pow(2,-24)*B*pow(16,A-64);
    if (t[0]&0x80)
        return -val;
    else
        return val;
}
//----------------------------------------------
zuchar GribRecord::readChar(ZUFILE* file) {
    zuchar t;
    if (zu_read(file, &t, 1) != 1) {
        ok = false;
        eof = true;
        return 0;
    }
    return t;
}
//----------------------------------------------
int GribRecord::readSignedInt3(ZUFILE* file) {
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
int GribRecord::readSignedInt2(ZUFILE* file) {
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
zuint GribRecord::readInt3(ZUFILE* file) {
    unsigned char t[3];
    if (zu_read(file, t, 3) != 3) {
        ok = false;
        eof = true;
        return 0;
    }
    return ((zuint)t[0]<<16)+((zuint)t[1]<<8)+(zuint)t[2];
}
//----------------------------------------------
zuint GribRecord::readInt2(ZUFILE* file) {
    unsigned char t[2];
    if (zu_read(file, t, 2) != 2) {
        ok = false;
        eof = true;
        return 0;
    }
    return ((zuint)t[0]<<8)+(zuint)t[1];
}
//----------------------------------------------
zuint GribRecord::makeInt3(zuchar a, zuchar b, zuchar c) {
    return ((zuint)a<<16)+((zuint)b<<8)+(zuint)c;
}
//----------------------------------------------
zuint GribRecord::readPackedBits(zuchar *buf, zuint first, zuint nbBits)
{
    zuint oct = first / 8;
    zuint bit = first % 8;
    
    zuint val = (buf[oct]<<24) + (buf[oct+1]<<16) + (buf[oct+2]<<8) + (buf[oct+3]);
    val = val << bit;
    val = val >> (32-nbBits);
    return val;
}
//----------------------------------------------
time_t GribRecord::makeDate(
            zuint year,zuint month,zuint day,zuint hour,zuint min,zuint sec) {
    struct tm date;
    date.tm_sec  = sec;         /* seconds */
    date.tm_min  = min;         /* minutes */
    date.tm_hour = hour;        /* hours */
    date.tm_mday = day;         /* day of the month */
    date.tm_mon  = month-1;     /* month */
    date.tm_year = year-1900;   /* year */
    date.tm_wday   = 0;         /* day of the week */
    date. tm_yday  = 0;         /* day in the year */
    date.tm_isdst  = 0;         /* daylight saving time */
    return mktime(&date);
}
//----------------------------------------------
zuint GribRecord::periodSeconds(zuchar unit,zuchar P1,zuchar P2,zuchar range) {
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
            erreur("id=%d: unknown time unit in PDS b18=%d",id,unit);
            res = 0;
            ok = false;
    }
    debug("id=%d: PDS (time range) b21=%d P1=%d P2=%d",id,range,P1,P2);
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
            erreur("id=%d: unknown time range in PDS b21=%d",id,range);
            dur = 0;
            ok = false;
    }
    return res*dur;
}


//======================================================================
float GribRecord::getInterpolatedValue(float px, float py) const
{
    float val;
    if (!ok || Di==0 || Dj==0) {
        return GRIB_NOTDEF;
    }
    if (!isPointInMap(px,py)) {
        px += 360.0;               // tour du monde à droite ?
        if (!isPointInMap(px,py)) {
            px -= 2*360.0;              // tour du monde à gauche ?
            if (!isPointInMap(px,py)) {
                return GRIB_NOTDEF;
            }
        }
    }
    float pi, pj;     // coord. in grid unit
    pi = (px-Lo1)/Di;
    pj = (py-La1)/Dj;

    // 00 10      point is in a square
    // 01 11
    int i0 = (int) pi;  // point 00
    int j0 = (int) pj;

    // distances to 00
    float dx = pi-i0;
    float dy = pj-j0;
    dx = (3.0 - 2.0*dx)*dx*dx;   // pseudo hermite interpolation
    dy = (3.0 - 2.0*dy)*dy*dy;
    
    bool h00,h01,h10,h11;
    int nbval = 0;     // how many values in grid ?
    if ((h00=hasValue(i0,   j0)))
        nbval ++;
    if ((h10=hasValue(i0+1, j0)))
        nbval ++;
    if ((h01=hasValue(i0,   j0+1)))
        nbval ++;
    if ((h11=hasValue(i0+1, j0+1)))
        nbval ++;

    if (nbval <3) {
        return GRIB_NOTDEF;
    }
    
    float xa, xb, xc, kx, ky;
    // Triangle :
    //   xa  xb
    //   xc
    // kx = distance(xa,x)
    // ky = distance(xa,y)
    if (nbval == 4)
    {
        float x00 = getValue(i0,   j0);
        float x01 = getValue(i0,   j0+1);
        float x10 = getValue(i0+1, j0);
        float x11 = getValue(i0+1, j0+1);
        val = ((1-dy)*x00 + dy*x01)*(1-dx)
                    + ((1-dy)*x10+dy*(x11))*dx;

        float x1 = (1.0-dx)*x00 + dx*x10;
        float x2 = (1.0-dx)*x01 + dx*x11;
        val =  (1.0-dy)*x1 + dy*x2;
        
        return val;
    }
    else {    
        // here nbval==3, check the corner without data
        if (!h00) {
            //printf("! h00  %f %f\n", dx,dy);
            xa = getValue(i0+1, j0+1);   // A = point 11
            xb = getValue(i0,   j0+1);   // B = point 01
            xc = getValue(i0+1, j0);     // C = point 10
            kx = 1-dx;
            ky = 1-dy;
        }
        else if (!h01) {
            //printf("! h01  %f %f\n", dx,dy);
            xa = getValue(i0+1, j0);     // A = point 10
            xb = getValue(i0+1, j0+1);   // B = point 11
            xc = getValue(i0,   j0);     // C = point 00
            kx = dy;
            ky = 1-dx;
        }
        else if (!h10) {
            //printf("! h10  %f %f\n", dx,dy);
            xa = getValue(i0,   j0+1);     // A = point 01
            xb = getValue(i0,   j0);       // B = point 00
            xc = getValue(i0+1, j0+1);     // C = point 11
            kx = 1-dy;
            ky = dx;
        }
        else {
            //printf("! h11  %f %f\n", dx,dy);
            xa = getValue(i0,   j0);    // A = point 00
            xb = getValue(i0+1, j0);    // B = point 10
            xc = getValue(i0,   j0+1);  // C = point 01
            kx = dx;
            ky = dy;
        }
    }
    float k = kx + ky;
    if (k<0 || k>1) {
        val = GRIB_NOTDEF;
    }
    else if (k == 0) {
        val = xa;
    }
    else {
        // axes interpolation
        float vx = k*xb + (1-k)*xa;
        float vy = k*xc + (1-k)*xa;
        // diagonal interpolation
        float k2 = kx / k;
        val =  k2*vx + (1-k2)*vy;
    }
    return val;
}







