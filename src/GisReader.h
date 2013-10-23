/**********************************************************************
zyGrib: meteorological GRIB file viewer
Copyright (C) 2008 - Jacques Zaninetti - http://www.zygrib.org

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

#ifndef GisREADER_H
#define GisREADER_H

#include <iostream>
#include <list>

#include <QImage>
#include <QPainter>

#include "class_list.h"
#include <QFile>

//==========================================================
class GisPoint {
    public:
        float   x,y;    // longitude, latitude
        
        GisPoint(const float &x_, const float &y_) {
            x = x_;
            y = y_;
        }
		virtual ~GisPoint() {}
        
        virtual void draw (QPainter *pnt, Projection *proj);
};
Q_DECLARE_TYPEINFO(GisPoint,Q_MOVABLE_TYPE);
//----------------------------------------------------------
class GisCountry : public GisPoint
{
    public:
        QString name;

        GisCountry (QString name_, double lon, double lat)
            : GisPoint(lon, lat)
            {
                name = name_;
            }
		virtual ~GisCountry() {}
		
        virtual void draw (QPainter *pnt, Projection *proj);
};
Q_DECLARE_TYPEINFO(GisCountry,Q_MOVABLE_TYPE);
//----------------------------------------------------------
class GisCity : public GisPoint
{
    public:
        QString name;
        int     population;
        quint8 level;

        GisCity (const QString &name_, const int &pop, const float &lon, const float &lat)
            : GisPoint(lon, lat)
            {
                name = name_;
                population = pop;
				if (population >= 1000000) {
					level = 1;
				}
				else if (population >= 200000) {
					level = 2;
				}
				else if (population >= 50000) {
					level = 3;
				}
				else {
					level = 4;
				}
            }
		~GisCity() {}
        
        void  draw (QPainter *pnt, Projection *proj, int level);
        void  getRectName  (QPainter *pnt, Projection *proj, QRect *rectName);
        void  drawCityName (QPainter *pnt, QRect *rectName);

	private:
	    int     x0, y0;   // for drawing
};
Q_DECLARE_TYPEINFO(GisCity,Q_MOVABLE_TYPE);

//==========================================================
class GisReader
{
    public:
        GisReader();
        ~GisReader();
        
        void drawCountriesNames (QPainter &pnt, Projection *proj);
        void drawCitiesNames (QPainter &pnt, Projection *proj, int level);
        void clearLists();

    private:
        std::list<GisPoint*> lsCountries;
        std::list<GisCity*> lsCities;
        
        //QFile q_cities;
        void loadCities(const int &level);
        void loadCountries();
};
Q_DECLARE_TYPEINFO(GisReader,Q_MOVABLE_TYPE);



#endif
