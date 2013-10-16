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

#ifndef GRIBV1RECORD_H
#define GRIBV1RECORD_H

#include <iostream>
#include <cmath>

#include "class_list.h"
#include "dataDef.h"
#include "GribRecord.h"

#include "zuFile.h"

#define DEBUG_INFO    false
#define DEBUG_ERROR   false
#define debug(format, ...)  {if(DEBUG_INFO)  {fprintf(stderr,format,__VA_ARGS__);fprintf(stderr,"\n");}}
#define erreur(format, ...) {if(DEBUG_ERROR) {fprintf(stderr,"ERROR: ");fprintf(stderr,format,__VA_ARGS__);fprintf(stderr,"\n");}}

#define zuint  unsigned int
#define zuchar unsigned char


//----------------------------------------------
class GribV1Record: public GribRecord
{
    public:
        GribV1Record(ZUFILE* file);
        GribV1Record(const GribV1Record &rec);

        ~GribV1Record();

        static void init_conversionMatrix(void);
        bool  isEof() const   {return eof;}

        //-----------------------------------------

        //void    translateDataType();  // adapte les codes des differents centres meteo

        zuchar   getIdGrid() const    { return idGrid; }


        // La valeur est-elle definie (grille a trous) ?
        inline bool hasValue(int i, int j) const;

        FCT_GET(int,dataTypeV1)
        FCT_GET(int,levelTypeV1)


    private:
        bool   eof;   // fin de fichier atteinte lors de la lecture
        //---------------------------------------------
        // SECTION 0: THE INDICATOR SECTION (IS)
        //---------------------------------------------
        zuint  fileOffset0;
        zuint  seekStart, totalSize;

        // SECTION 1: THE PRODUCT DEFINITION SECTION (PDS)
        zuint  fileOffset1;
        zuint  sectionSize1;
        zuchar tableVersion;
        zuchar data1[28];

        int dataTypeV1;
        int levelTypeV1;

        bool   hasGDS;
        bool   hasBMS;
        zuchar periodP1, periodP2;
        zuchar timeRange;
        zuint  periodsec;     // period in seconds
        double  decimalFactorD;
        // SECTION 2: THE GRID DESCRIPTION SECTION (GDS)
        zuint  fileOffset2;
        zuint  sectionSize2;
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

        zuint  periodSeconds(zuchar unit, zuchar P1, zuchar P2, zuchar range);

        void   print();



};
Q_DECLARE_TYPEINFO(GribV1Record,Q_MOVABLE_TYPE);

inline bool GribV1Record::hasValue(int i, int j) const
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




#endif // GRIBV1RECORD_H
