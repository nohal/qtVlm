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


#ifndef GRIBV2_H
#define GRIBV2_H

#include "dataDef.h"
#include "class_list.h"
#include "Grib.h"

/* switch 1/0 for g2_getfld */
#define GRB2_UNPACK 1
#define GRB2_EXPAND 1

/*struct GrbType {
    int cat;
    int num;
    bool undef;
};*/

class GribV2: public Grib
{
    public:
        GribV2(DataManager *dataManager);

        virtual bool loadFile(QString fileName);

        static bool isGribV2(QString fileName);

};

#endif // GRIBV2_H
