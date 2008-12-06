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


/******************************************
Elément de base d'un fichier GRIB
******************************************/

#ifndef GRIBRECORD_H
#define GRIBRECORD_H

#include <iostream>
#include <cmath>

#include "zuFile.h"

#define DEBUG_INFO    false
#define DEBUG_ERROR   false
#define debug(format, ...)  {if(DEBUG_INFO)  {fprintf(stderr,format,__VA_ARGS__);fprintf(stderr,"\n");}}
#define erreur(format, ...) {if(DEBUG_ERROR) {fprintf(stderr,"ERROR: ");fprintf(stderr,format,__VA_ARGS__);fprintf(stderr,"\n");}}

#define zuint  unsigned int
#define zuchar unsigned char

#define GRIB_NOTDEF -999999999   

//-------------------------------------
// dataTypes
//-------------------------------------
#define GRB_PRESS_MSL    2    /* Pa   */
#define GRB_TEMP        11    /* K    */
#define GRB_TEMP_DIEW   17    /* K    */
#define GRB_WIND_VX     33    /* m/s  */
#define GRB_WIND_VY     34    /* m/s  */
#define GRB_PRECIP_TOT  61    /* l/m2 */
#define GRB_CLOUD_TOT   71    /* % */
#define GRB_HUMID_REL   52    /* % */

#define GRB_UNDEFINED   -1

class GribRecord
{
    public:
        GribRecord(ZUFILE* file, int id_);
        GribRecord(GribRecord &rec);
        ~GribRecord();
        
        inline bool  isOk()  const   {return ok;};
        inline bool  isEof() const   {return eof;};
        
        //-----------------------------------------
        inline int    getDataType() const  { return dataType; }
        inline int    getPeriodP1() const  { return periodP1; }
        inline int    getPeriodP2() const  { return periodP2; }

        // Nombre de points de la grille
        inline int    getNi() const     { return Ni; }
        inline int    getNj() const     { return Nj; }
        inline float  getDi() const     { return Di; }
        inline float  getDj() const     { return Dj; }
        
        // Valeur pour un point de la grille
        float getValue(int i, int j) const  { return ok ? data[j*Ni+i] : GRIB_NOTDEF;};
        
        // Valeur pour un point quelconque
        float  getInterpolatedValue(float px, float py) const;

        // coordonnées d'un point de la grille
        inline float  getX(int i) const   { return ok ? Lo1+i*Di : GRIB_NOTDEF;}
        inline float  getY(int j) const   { return ok ? La1+j*Dj : GRIB_NOTDEF;}

        // Le point est-il à l'intérieur de la grille ?
        inline bool   isPointInMap(float x, float y) const;
        inline bool   isXInMap(float x) const;
        inline bool   isYInMap(float y) const;
        
        // La valeur est-elle définie (grille à trous) ?
        inline bool   hasValue(int i, int j) const;
        
        // Date de référence (création du fichier)
        inline time_t getReferenceDate() const   { return refDate; } 
        
        // Date courante des prévisions
        inline time_t getCurrentDate() const     { return curDate; } 
        inline void  setCurrentDate(time_t t)    { curDate = t; }
    
        inline time_t getRefDate() const     { return refDate; }


    private:
        int    id;    // unique identifiant
        bool   ok;    // validité des données
        bool   eof;   // fin de fichier atteinte lors de la lecture

        //---------------------------------------------
        // SECTION 0: THE INDICATOR SECTION (IS)
        //---------------------------------------------
        zuint  fileOffset0;
        zuint  seekStart, totalSize;
        zuchar editionNumber;
        // SECTION 1: THE PRODUCT DEFINITION SECTION (PDS)
        zuint  fileOffset1;
        zuint  sectionSize1;
        zuchar tableVersion;
        zuchar data1[28];
        zuint  idCenter;
        zuchar idGrid;
        zuchar dataType;      // type of data
        bool   hasGDS;
        bool   hasBMS;
        zuint  refyear, refmonth, refday, refhour, refminute;
        zuchar periodP1, periodP2;
        zuchar timeRange;
        zuint  periodsec;     // period in seconds
        time_t refDate;      // C reference date
        time_t curDate;      // C current date
        float  decimalFactorD;
        // SECTION 2: THE GRID DESCRIPTION SECTION (GDS)
        zuint  fileOffset2;
        zuint  sectionSize2;
        zuchar NV, PV;
        zuchar gridType;
        zuint  Ni, Nj;
        float La1, Lo1, La2, Lo2;
        float Di, Dj;
        zuchar resolFlags, scanFlags;
        bool  hasDiDj;
        bool  isEarthSpheric;
        bool  isUeastVnorth;
        bool  isScanIpositive;
        bool  isScanJpositive;
        bool  isAdjacentI;
        // SECTION 3: BIT MAP SECTION (BMS)
        zuint  fileOffset3;
        zuint  sectionSize3;
        zuchar *BMSbits;
        // SECTION 4: BINARY DATA SECTION (BDS)
        zuint  fileOffset4;
        zuint  sectionSize4;
        zuchar unusedBitsEndBDS;
        bool  isGridData;          // not spherical harmonics
        bool  isSimplePacking;
        bool  isFloatValues;
        int   scaleFactorE;
        float scaleFactorEpow2;
        float refValue;
        zuint  nbBitsInPack;
        float  *data;
        // SECTION 5: END SECTION (ES)
        
        //---------------------------------------------
        // Lecture des données
        //---------------------------------------------
        bool readGribSection0_IS (ZUFILE* file);
        bool readGribSection1_PDS(ZUFILE* file);
        bool readGribSection2_GDS(ZUFILE* file);
        bool readGribSection3_BMS(ZUFILE* file);
        bool readGribSection4_BDS(ZUFILE* file);
        bool readGribSection5_ES (ZUFILE* file);

        //---------------------------------------------
        // Fonctions utiles
        //---------------------------------------------
        zuchar readChar(ZUFILE* file);
        int   readSignedInt3(ZUFILE* file);
        int   readSignedInt2(ZUFILE* file);
        zuint  readInt2(ZUFILE* file);
        zuint  readInt3(ZUFILE* file);
        float readFloat4(ZUFILE* file);
        
        zuint  readPackedBits(zuchar *buf, zuint first, zuint nbBits);
        zuint  makeInt3(zuchar a, zuchar b, zuchar c);
        
        time_t makeDate(zuint year,zuint month,zuint day,zuint hour,zuint min,zuint sec);
        zuint   periodSeconds(zuchar unit, zuchar P1, zuchar P2, zuchar range);

};

//==========================================================================
inline bool   GribRecord::hasValue(int i, int j) const
{
    // is data present in BMS ?
    if (!ok) {
        return false;
    }
    if (!hasBMS) {
        return true;
    }
    int bit;
    if (isAdjacentI) {
        bit = j*Ni + i;
    }
    else {
        bit = i*Nj + j;
    }
    zuchar c = BMSbits[bit/8];
    zuchar m = (zuchar)128 >> (bit % 8);
    return (m & c) != 0;
}

//-----------------------------------------------------------------
inline bool GribRecord::isPointInMap(float x, float y) const
{
    return isXInMap(x) && isYInMap(y);
/*    if (Dj < 0)
        return x>=Lo1 && y<=La1 && x<=Lo1+(Ni-1)*Di && y>=La1+(Nj-1)*Dj;
    else
        return x>=Lo1 && y>=La1 && x<=Lo1+(Ni-1)*Di && y<=La1+(Nj-1)*Dj;*/
}
//-----------------------------------------------------------------
inline bool GribRecord::isXInMap(float x) const
{
//    return x>=Lo1 && x<=Lo1+(Ni-1)*Di;
//printf ("%f %f %f\n", Lo1, Lo2, x);
    if (Di > 0)
        return x>=Lo1 && x<=Lo2;
    else
        return x>=Lo2 && x<=Lo1;
}
//-----------------------------------------------------------------
inline bool GribRecord::isYInMap(float y) const
{
    if (Dj < 0)
        return y<=La1 && y>=La2;
    else
        return y>=La1 && y<=La2;
}

#endif



