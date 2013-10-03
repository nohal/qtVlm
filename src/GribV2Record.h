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

#ifndef GRIBV2RECORD_H
#define GRIBV2RECORD_H

#include "class_list.h"
#include "dataDef.h"
#include <grib2.h>

#include "GribRecord.h"

class GribV2Record: public GribRecord
{
    public:
        GribV2Record(gribfield  *gfld, int msg, int field);
        GribV2Record(GribV2Record &rec);
        ~GribV2Record(void);

        static void init_conversionMatrix(void);

        // La valeur est-elle definie (grille a trous) ?
        inline bool   hasValue(int i, int j) const;

        double get_data(int i) { if(ok) return data[i]; else return 0; }
        bool get_bmap(int i) { if(ok) return bmap[i]; else return 0; }



    private:
        int productTemplate;
        int productDiscipline;
        int gridTemplateNum;
        int dataCat;
        int dataNum;
        int levelTypeV2;

        bool * bmap;

        int computeTimeOffset(int type,int val);

};

inline bool GribV2Record::hasValue(int i, int j) const {
    if(!ok) return false;

    if(!bmap) return true;

    int bit;
    if (isAdjacentI)  bit = j*Ni + i;
    else              bit = i*Nj + j;

    return bmap[bit];
}

class grb2DataType {
    public:
        grb2DataType() { ; }
        grb2DataType(int discipline,int category,int number) {
            this->discipline=discipline;
            this->category=category;
            this->number=number;
        }

        int discipline;
        int category;
        int number;
};

inline bool operator<(const grb2DataType &e1, const grb2DataType &e2) {
    if(e1.discipline == e2.discipline) {
        if(e1.category == e2.category) {
            return e1.number < e2.number;
        }
        else
            return e1.category < e2.category;
    }
    else
        return e1.discipline<e2.discipline;
}

#endif // GRIBV2RECORD_H
