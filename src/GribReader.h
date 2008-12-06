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

/*************************
Lecture mise en mémoire d'un fichier GRIB

*************************/

#ifndef GRIBREADER_H
#define GRIBREADER_H

#include <iostream>
#include <cmath>
#include <vector>
#include <set>

#include "GribRecord.h"
#include "zuFile.h"

//===============================================================
class GribReader
{
    public:
        GribReader();
        GribReader(const std::string fname);
        ~GribReader();
        
        void  openFile(const std::string fname);
        bool  isOk()                 {return ok;}
        long  getFileSize()          {return fileSize;}
        std::string getFileName()    {return fileName;}

        int          getNumberOfGribRecords(int type);
        int          getTotalNumberOfGribRecords();
        GribRecord * getGribRecord(int type, time_t date);
        GribRecord * getFirstGribRecord();
        
        std::set<time_t>  getListDates()   {return setAllDates;}
        int        getNumberOfDates()   {return setAllDates.size();}
        time_t     getRefDate()   {return setAllDates.size()>0 ?
                                                *setAllDates.begin() : 0;}
        
        // Rectangle de la zone couverte par les données
        bool getGribExtension(float *x0,float *y0, float *x1,float *y1);
    

    private:
        bool   ok;
        std::string fileName;
        ZUFILE *file;
        long    fileSize;
        
        std::vector<GribRecord *> ls_GRB_PRESS_MSL;
        std::vector<GribRecord *> ls_GRB_TEMP;
        std::vector<GribRecord *> ls_GRB_TEMP_DIEW;
        std::vector<GribRecord *> ls_GRB_WIND_VX;
        std::vector<GribRecord *> ls_GRB_WIND_VY;
        std::vector<GribRecord *> ls_GRB_PRECIP_TOT;
        std::vector<GribRecord *> ls_GRB_CLOUD_TOT;
        std::vector<GribRecord *> ls_GRB_HUMID_REL;
        std::vector<GribRecord *> ls_OTHER;

        void   createListDates();
        std::set<time_t> setAllDates;
        
        void clean_vector(std::vector<GribRecord *> &ls);
        void clean_all_vectors();
        float  getHoursBeetweenGribRecords();   // delay in hours
};


#endif
