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

#include "GribReader.h"
#include <cassert>

//-------------------------------------------------------------------------------
GribReader::GribReader()
{
    ok = false;
}
//-------------------------------------------------------------------------------
GribReader::GribReader(const std::string fname)
{
    if (fname != "") {
        openFile(fname);
    }
    else {
        clean_all_vectors();
        ok = false;
    }
}
//-------------------------------------------------------------------------------
GribReader::~GribReader()
{
    clean_all_vectors();
}
//-------------------------------------------------------------------------------
void GribReader::clean_all_vectors()
{
    clean_vector(ls_GRB_PRESS_MSL);
    clean_vector(ls_GRB_TEMP);
    clean_vector(ls_GRB_TEMP_DIEW);
    clean_vector(ls_GRB_WIND_VX);
    clean_vector(ls_GRB_WIND_VY);
    clean_vector(ls_GRB_PRECIP_TOT);
    clean_vector(ls_GRB_CLOUD_TOT);
    clean_vector(ls_GRB_HUMID_REL);
    clean_vector(ls_OTHER);
}
//-------------------------------------------------------------------------------
void GribReader::clean_vector(std::vector<GribRecord *> &ls)
{
    std::vector<GribRecord *>::iterator it;
    for (it=ls.begin(); it!=ls.end(); it++) {
        delete *it;
        *it = NULL;
    }
    ls.clear();
}

//-------------------------------------------------------------------------------
// Lecture complète d'un fichier GRIB
//-------------------------------------------------------------------------------
void GribReader::openFile(const std::string fname)
{
    debug("Open file: %s", fname.c_str());
    
    fileName = fname;
    ok = false;
    clean_all_vectors();
    
    //--------------------------------------------------------
    // Ouverture du fichier
    //--------------------------------------------------------
    file = zu_open(fname.c_str(), "rb", ZU_COMPRESS_AUTO);
    if (file == NULL) {
        ok = false;
        erreur("Can't open file: %s", fname.c_str());
        return;
    }
    readGribFileContent();
    
    // Essaie d'autres compressions (extension non reconnue ?)
    if (! ok) {
        if (file != NULL)
            zu_close(file);
        file = zu_open(fname.c_str(), "rb", ZU_COMPRESS_BZIP);
        if (file != NULL)
            readGribFileContent();
    }
    if (! ok) {
        if (file != NULL)
            zu_close(file);
        file = zu_open(fname.c_str(), "rb", ZU_COMPRESS_GZIP);
        if (file != NULL)
            readGribFileContent();
    }
    if (! ok) {
        if (file != NULL)
            zu_close(file);
        file = zu_open(fname.c_str(), "rb", ZU_COMPRESS_NONE);
        if (file != NULL)
            readGribFileContent();
    }
}
    
//---------------------------------------------------------------------------------
void GribReader::readGribFileContent()
{
    fileSize = zu_filesize(file);
    //--------------------------------------------------------
    // Lecture de l'ensemble des GribRecord du fichier
    // et stockage dans les listes appropriées.
    //--------------------------------------------------------
    GribRecord *rec;
    int id = 0;
    time_t firstdate = -1;
    do {
        id ++;
        rec = new GribRecord(file, id);
        assert(rec);
        if (rec->isOk())
        {
            ok = true;   // au moins 1 record ok
            
            if (firstdate== -1)
                firstdate = rec->getCurrentDate();
            
            debug("Record %3d type=%3d %d",
                    id, rec->getDataType(), rec->isEof() );
            
            switch(rec->getDataType()) {
                case GRB_PRESS_MSL :
                    ls_GRB_PRESS_MSL.push_back(rec);
                    break;
                case GRB_TEMP :
                    ls_GRB_TEMP.push_back(rec);
                    break;
                case GRB_TEMP_DIEW :
                    ls_GRB_TEMP_DIEW.push_back(rec);
                    break;
                case GRB_WIND_VX :
                    ls_GRB_WIND_VX.push_back(rec);
                    break;
                case GRB_WIND_VY :
                    ls_GRB_WIND_VY.push_back(rec);
                    break;
                case GRB_PRECIP_TOT :
                    if (ls_GRB_PRECIP_TOT.size()==0) {
                        GribRecord *r2 = new GribRecord(*rec);
                        if(r2!=NULL)
                        {
                            r2->setCurrentDate(firstdate);    // 1er enregistrement factice
                            ls_GRB_PRECIP_TOT.push_back(r2);
                        }
                    }
                    ls_GRB_PRECIP_TOT.push_back(rec);
                    break;
                case GRB_CLOUD_TOT :
                    if (ls_GRB_CLOUD_TOT.size()==0) {
                        GribRecord *r2 = new GribRecord(*rec);
                        if(r2!=NULL)
                        {
                            r2->setCurrentDate(firstdate);    // 1er enregistrement factice
                            ls_GRB_CLOUD_TOT.push_back(r2);
                        }
                    }
                    ls_GRB_CLOUD_TOT.push_back(rec);
                    break;
                case GRB_HUMID_REL :
                    ls_GRB_HUMID_REL.push_back(rec);
                    break;
                default :
                    ls_OTHER.push_back(rec);
            }
        }
        else {
            delete rec;
            rec = NULL;
        }
    } while (rec != NULL &&  !rec->isEof());
    
    createListDates();
}

//---------------------------------------------------
int GribReader::getTotalNumberOfGribRecords() {
    return ls_GRB_PRESS_MSL.size() 
                + ls_GRB_TEMP.size() 
                + ls_GRB_TEMP_DIEW.size() 
                + ls_GRB_WIND_VX.size() 
                + ls_GRB_WIND_VY.size() 
                + ls_GRB_PRECIP_TOT.size() 
                + ls_GRB_CLOUD_TOT.size()
                + ls_GRB_HUMID_REL.size()
                + ls_OTHER.size() ;
}
//---------------------------------------------------
int GribReader::getNumberOfGribRecords(int type) {
    switch(type) {
        case GRB_PRESS_MSL :
            return ls_GRB_PRESS_MSL.size();
        case GRB_TEMP :
            return ls_GRB_TEMP.size();
        case GRB_TEMP_DIEW :
            return ls_GRB_TEMP_DIEW.size();
        case GRB_WIND_VX :
            return ls_GRB_WIND_VX.size();
        case GRB_WIND_VY :
            return ls_GRB_WIND_VY.size();
        case GRB_PRECIP_TOT :
            return ls_GRB_PRECIP_TOT.size();
        case GRB_CLOUD_TOT :
            return ls_GRB_CLOUD_TOT.size();
        case GRB_HUMID_REL :
            return ls_GRB_HUMID_REL.size();
        case GRB_UNDEFINED :
            return ls_OTHER.size();
        default :
            return 0;
    }
}
//---------------------------------------------------
// Rectangle de la zone couverte par les données
bool GribReader::getGribExtension(float *x0,float *y0, float *x1,float *y1)
{
    std::vector<GribRecord *> *ls = NULL;
    // Cherche une liste non vide
    if (ls_GRB_WIND_VX.size()>0)
        ls = &ls_GRB_WIND_VX;
    else if(ls_GRB_WIND_VY.size()>0)
        ls = &ls_GRB_WIND_VY;
    else if(ls_GRB_PRESS_MSL.size()>0)
        ls = &ls_GRB_PRESS_MSL;
    else if(ls_GRB_TEMP.size()>0)
        ls = &ls_GRB_TEMP;
    else if(ls_GRB_TEMP_DIEW.size()>0)
        ls = &ls_GRB_TEMP_DIEW;
    else if(ls_GRB_PRECIP_TOT.size()>0)
        ls = &ls_GRB_PRECIP_TOT;
    else if(ls_GRB_CLOUD_TOT.size()>0)
        ls = &ls_GRB_CLOUD_TOT;
    else if(ls_GRB_HUMID_REL.size()>0)
        ls = &ls_GRB_HUMID_REL;
    else if(ls_OTHER.size()>0)
        ls = &ls_OTHER;
    if (ls != NULL) {
        GribRecord *rec = ls->at(0);
        if (rec != NULL) {
            *x0 = rec->getX(0);
            *y0 = rec->getY(0);
            *x1 = rec->getX( rec->getNi()-1 );
            *y1 = rec->getY( rec->getNj()-1 );
        }
        return true;
    }
    else {
        return false;
    }
}
//---------------------------------------------------
// Premier GribRecord trouvé (pour récupérer la grille)
GribRecord * GribReader::getFirstGribRecord()
{
    std::vector<GribRecord *> *ls = NULL;
    // Cherche une liste non vide
    if (ls_GRB_WIND_VX.size()>0)
        ls = &ls_GRB_WIND_VX;
    else if(ls_GRB_WIND_VY.size()>0)
        ls = &ls_GRB_WIND_VY;
    else if(ls_GRB_PRESS_MSL.size()>0)
        ls = &ls_GRB_PRESS_MSL;
    else if(ls_GRB_TEMP.size()>0)
        ls = &ls_GRB_TEMP;
    else if(ls_GRB_TEMP_DIEW.size()>0)
        ls = &ls_GRB_TEMP_DIEW;
    else if(ls_GRB_PRECIP_TOT.size()>0)
        ls = &ls_GRB_PRECIP_TOT;
    else if(ls_GRB_CLOUD_TOT.size()>0)
        ls = &ls_GRB_CLOUD_TOT;
    else if(ls_GRB_HUMID_REL.size()>0)
        ls = &ls_GRB_HUMID_REL;
    else if(ls_OTHER.size()>0)
        ls = &ls_OTHER;
    if (ls != NULL) {
        return ls->at(0);
    }
    else {
        return NULL;
    }
}
//---------------------------------------------------
// Délai en heures entre 2 records
// On suppose qu'il est fixe pour tout le fichier !!!
float GribReader::getHoursBeetweenGribRecords()
{
    std::vector<GribRecord *> *ls = NULL;
    // Cherche une liste de 2 éléments
    if (ls_GRB_WIND_VX.size()>1)
        ls = &ls_GRB_WIND_VX;
    else if(ls_GRB_WIND_VY.size()>1)
        ls = &ls_GRB_WIND_VY;
    else if(ls_GRB_PRESS_MSL.size()>1)
        ls = &ls_GRB_PRESS_MSL;
    else if(ls_GRB_TEMP.size()>1)
        ls = &ls_GRB_TEMP;
    else if(ls_GRB_TEMP_DIEW.size()>1)
        ls = &ls_GRB_TEMP_DIEW;
    else if(ls_GRB_PRECIP_TOT.size()>1)
        ls = &ls_GRB_PRECIP_TOT;
    else if(ls_GRB_CLOUD_TOT.size()>1)
        ls = &ls_GRB_CLOUD_TOT;
    else if(ls_GRB_HUMID_REL.size()>0)
        ls = &ls_GRB_HUMID_REL;
    else if(ls_OTHER.size()>1)
        ls = &ls_OTHER;
        
    if (ls == NULL) {
        return -1;
    }
    else {
        time_t t0 = (*ls)[0]->getCurrentDate();
        time_t t1 = (*ls)[1]->getCurrentDate();
        return   abs(t1-t0) / 3600.0;
    }
}
//---------------------------------------------------
GribRecord * GribReader::getGribRecord(int type, time_t date)
{
    std::vector<GribRecord *> *ls = NULL;
    bool lsok = true;
    switch(type) {
        case GRB_PRESS_MSL :
            ls = &ls_GRB_PRESS_MSL;
            break;
        case GRB_TEMP :
            ls = &ls_GRB_TEMP;
            break;
        case GRB_TEMP_DIEW :
            ls = &ls_GRB_TEMP_DIEW;
            break;
        case GRB_WIND_VX :
            ls = &ls_GRB_WIND_VX;
            break;
        case GRB_WIND_VY :
            ls = &ls_GRB_WIND_VY;
            break;
        case GRB_PRECIP_TOT :
            ls = &ls_GRB_PRECIP_TOT;
            break;
        case GRB_CLOUD_TOT :
            ls = &ls_GRB_CLOUD_TOT;
            break;
        case GRB_HUMID_REL :
            ls = &ls_GRB_HUMID_REL;
            break;
        default :
            ls = &ls_OTHER;
            lsok = false;
    }
    if (lsok) {
        // Cherche le premier enregistrement à la bonne date
        GribRecord *res = NULL;
        unsigned int nb = ls->size();
        for (unsigned int i=0; i<nb && res==NULL; i++) {
            if ((*ls)[i]->getCurrentDate() == date)
                res = (*ls)[i];
        }
        return res;
    }
    else {
        //  TODO : return GribRecord number num of type 'type' in  ls_OTHER
        return NULL;
    }
}

//=========================================================================
// Génère la liste des dates pour lesquelles des prévisions existent
void GribReader::createListDates()
{
    // Le set assure l'ordre et l'unicité des dates
    setAllDates.clear();
    
    std::vector<GribRecord *> *tab;
    unsigned int i;
    tab = & ls_GRB_PRESS_MSL;
    for (i=0; i<tab->size(); i++) {
        setAllDates.insert( tab->at(i)->getCurrentDate());
    }
    tab = & ls_GRB_TEMP;
    for (i=0; i<tab->size(); i++) {
        setAllDates.insert( tab->at(i)->getCurrentDate());
    }
    tab = & ls_GRB_TEMP_DIEW;
    for (i=0; i<tab->size(); i++) {
        setAllDates.insert( tab->at(i)->getCurrentDate());
    }
    tab = & ls_GRB_WIND_VX;
    for (i=0; i<tab->size(); i++) {
        setAllDates.insert( tab->at(i)->getCurrentDate());
    }
    tab = & ls_GRB_WIND_VY;
    for (i=0; i<tab->size(); i++) {
        setAllDates.insert( tab->at(i)->getCurrentDate());
    }
    tab = & ls_GRB_PRECIP_TOT;
    for (i=0; i<tab->size(); i++) {
        setAllDates.insert( tab->at(i)->getCurrentDate());
    }
    tab = & ls_GRB_CLOUD_TOT;
    for (i=0; i<tab->size(); i++) {
        setAllDates.insert( tab->at(i)->getCurrentDate());
    }
    tab = & ls_GRB_HUMID_REL;
    for (i=0; i<tab->size(); i++) {
        setAllDates.insert( tab->at(i)->getCurrentDate());
    }
//     tab = & ls_OTHER;    // TODO
//     for (i=0; i<tab->size(); i++) {
//         setAllDates.insert( tab->at(i)->getCurrentDate());
//     }

}




