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

#ifndef GisREADER_H
#define GisREADER_H

#include <iostream>
#include <list>

#include <QImage>
#include <QPainter>

#include "zuFile.h"
#include "Projection.h"
#include "Util.h"

//==========================================================
class GisPoint {
    public:
        float   x,y;    // longitude, latitude
        
        GisPoint(float x_, float y_) {
            x = x_;
            y = y_;
        }
		virtual ~GisPoint() {}
        
        virtual void draw(QPainter *pnt, Projection *proj);
};
//----------------------------------------------------------
class GisCountry : public GisPoint
{
    public:
        QString code;
        QString name;

        GisCountry(QString code_, QString name_, float lon, float lat)
            : GisPoint(lon, lat)
            {
                code = code_;
                name = name_;
            }
		virtual ~GisCountry() {}
		
        virtual void draw(QPainter *pnt, Projection *proj);
};
//----------------------------------------------------------
class GisCity : public GisPoint
{
    public:
        QString country;
        QString name;
        int     population;

        GisCity(QString country_, QString name_, int pop, float lon, float lat)
            : GisPoint(lon, lat)
            {
                country = country_;
                name = name_;
                population = pop;
            }
		~GisCity() {}
        
        void draw(QPainter *pnt, Projection *proj, int level);
};

//==========================================================
class GisReader
{
    public:
        GisReader();
        ~GisReader();
        
        void drawCountriesNames(QPainter &pnt, Projection *proj);
        void drawCitiesNames(QPainter &pnt, Projection *proj, int level);
    
    private:
        
        std::list<GisPoint*> lsCountries;
        std::list<GisCity*> lsCities;
        
        void clearLists();
};



#endif
