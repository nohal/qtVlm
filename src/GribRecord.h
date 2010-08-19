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

#include "class_list.h"
#include "dataDef.h"

#include "zuFile.h"

#define DEBUG_INFO    false
#define DEBUG_ERROR   false
#define debug(format, ...)  {if(DEBUG_INFO)  {fprintf(stderr,format,__VA_ARGS__);fprintf(stderr,"\n");}}
#define erreur(format, ...) {if(DEBUG_ERROR) {fprintf(stderr,"ERROR: ");fprintf(stderr,format,__VA_ARGS__);fprintf(stderr,"\n");}}

#define zuint  unsigned int
#define zuchar unsigned char


//----------------------------------------------
class GribRecord
{
    public:
        GribRecord(ZUFILE* file, int id_);
        GribRecord(const GribRecord &rec);
        ~GribRecord();
        
        bool  isOk()  const   {return ok;};
        bool  isDataKnown()  const   {return knownData;};
        bool  isEof() const   {return eof;};
        
        //-----------------------------------------
        zuchar  getDataType() const         { return dataType; }
        void    setDataType(const zuchar t);
        void    translateDataType();  // adapte les codes des différents centres météo
        
        zuchar  getLevelType() const   { return levelType; }
        zuint   getLevelValue() const  { return levelValue; }
        
        zuchar   getIdCenter() const  { return idCenter; }
        zuchar   getIdModel() const   { return idModel; }
        zuchar   getIdGrid() const    { return idGrid; }
        
        //-----------------------------------------
        std::string getKey() const  { return dataKey; }
		static std::string makeKey(int dataType,int levelType,int levelValue);
        
        //-----------------------------------------
        int    getPeriodP1() const  { return periodP1; }
        int    getPeriodP2() const  { return periodP2; }

        // Nombre de points de la grille
        int    getNi() const     { return Ni; }
        int    getNj() const     { return Nj; }
        double  getDi() const    { return Di; }
        double  getDj() const    { return Dj; }
        
        // Valeur pour un point de la grille
        double getValue(int i, int j) const  { return ok ? data[j*Ni+i] : GRIB_NOTDEF;}
        
        void setValue(zuint i, zuint j, double v)
        		{ if (i<Ni && j<Nj)
        			data[j*Ni+i] = v; }
        
        // Valeur pour un point quelconque
        bool getValue_TWSA(double px, double py,double * a00,double * a01,double * a10,double * a11,bool debug=false);
        double getInterpolatedValue(double px, double py, bool numericalInterpolation=true);

        // coordonnées d'un point de la grille
        inline double  getX(int i) const   { return ok ? Lo1+i*Di : GRIB_NOTDEF;}
        inline double  getY(int j) const   { return ok ? La1+j*Dj : GRIB_NOTDEF;}
        
        double  getLatMin() const   { return latMin;}
        double  getLonMin() const   { return lonMin;}
        double  getLatMax() const   { return latMax;}
        double  getLonMax() const   { return lonMax;}

        // Le point est-il �  l'intérieur de la grille ?
        inline bool   isPointInMap(double x, double y) const;
        inline bool   isXInMap(double x) const;
        inline bool   isYInMap(double y) const;
        // La valeur est-elle définie (grille �  trous) ?
        inline bool   hasValue(int i, int j) const;
        bool getIsFull() {return isFull; }
        
        // Date de référence (création du fichier)
        time_t getRecordRefDate () const         { return refDate; }
        const char* getStrRecordRefDate () const { return strRefDate; }
        
        // Date courante des prévisions
        time_t getRecordCurrentDate () const     { return curDate; }

    private:
        int    id;    // unique identifiant
        bool   ok;    // validité des données
        bool   eof;   // fin de fichier atteinte lors de la lecture
        bool knownData;
        std::string dataKey;
        char   strRefDate [32];
        char   strCurDate [32];

        bool isFull; /* true if grib is covering all lat */

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
        zuchar idCenter;
        zuchar idModel;
        zuchar idGrid;
        zuchar dataType;      // octet 9 = parameters and units
        zuchar levelType;
        zuint  levelValue;
        bool   hasGDS;
        bool   hasBMS;
        zuint  refyear, refmonth, refday, refhour, refminute;
        zuchar periodP1, periodP2;
        zuchar timeRange;
        zuint  periodsec;     // period in seconds
        time_t refDate;      // C reference date
        time_t curDate;      // C current date
        double  decimalFactorD;
        // SECTION 2: THE GRID DESCRIPTION SECTION (GDS)
        zuint  fileOffset2;
        zuint  sectionSize2;
        zuchar NV, PV;
        zuchar gridType;
        zuint  Ni, Nj;
        double La1, Lo1, La2, Lo2;
        double latMin, lonMin, latMax, lonMax;
        double Di, Dj;        
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
        double scaleFactorEpow2;
        double refValue;
        zuint  nbBitsInPack;
        double  *data;
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
        int    readSignedInt3(ZUFILE* file);
        int    readSignedInt2(ZUFILE* file);
        zuint  readInt2(ZUFILE* file);
        zuint  readInt3(ZUFILE* file);
        double readFloat4(ZUFILE* file);
        
        zuint  readPackedBits(zuchar *buf, zuint first, zuint nbBits);
        zuint  makeInt3(zuchar a, zuchar b, zuchar c);
        zuint  makeInt2(zuchar b, zuchar c);
        
        time_t makeDate(zuint year,zuint month,zuint day,zuint hour,zuint min,zuint sec);
        zuint  periodSeconds(zuchar unit, zuchar P1, zuchar P2, zuchar range);

        void   print();
        void   multiplyAllData(double k);


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
inline bool GribRecord::isPointInMap(double x, double y) const
{
    return isXInMap(x) && isYInMap(y);
/*    if (Dj < 0)
        return x>=Lo1 && y<=La1 && x<=Lo1+(Ni-1)*Di && y>=La1+(Nj-1)*Dj;
    else
        return x>=Lo1 && y>=La1 && x<=Lo1+(Ni-1)*Di && y<=La1+(Nj-1)*Dj;*/
}
//-----------------------------------------------------------------
inline bool GribRecord::isXInMap(double x) const
{
//    return x>=Lo1 && x<=Lo1+(Ni-1)*Di;
//printf ("%f %f %f\n", Lo1, Lo2, x);
    double a=0;

    if(isFull)
        a=Di;

    if (Di > 0)
        return x>=Lo1 && x<=(Lo2+a);
    else
        return x>=(Lo2+a) && x<=Lo1;
}
//-----------------------------------------------------------------
inline bool GribRecord::isYInMap(double y) const
{
    if (Dj < 0)
        return y<=La1 && y>=La2;
    else
        return y>=La1 && y<=La2;
}

#endif



