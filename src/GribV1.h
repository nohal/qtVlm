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

#ifndef GRIBV1_H
#define GRIBV1_H

#include "class_list.h"
#include "dataDef.h"

#include "Grib.h"

#include "zuFile.h"

#define COMPRESS_NO_GRIB -2

class GribV1 : public Grib
{
    public:
        GribV1(DataManager *dataManager);

        static bool isGribV1(QString fileName);

        virtual bool loadFile(QString fileName);

        static void initCompressModes(void);

    private:
        bool readAllGribRecords(const char *fname, int compressMode);

        static int findCompressMode(const char * fname);
        static bool chkIsGrib(ZUFILE *fptr);
};

#endif // GRIBV1_H
