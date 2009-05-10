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

/*************************************
Dessin des données GRIB (avec QT)
*************************************/

#ifndef GRIBPLOT_H
#define GRIBPLOT_H

#include <iostream>
#include <cmath>
#include <vector>
#include <list>
#include <set>

#include <QApplication>
#include <QPainter>

#include "GribReader.h"
#include "GribPointInfo.h"
#include "Projection.h"
#include "Util.h"

//===============================================================
class GribPlot
{
    public:
        GribPlot(bool interpolateValues_, bool windArrowsOnGribGrid_);
        GribPlot(const GribPlot &);
        ~GribPlot();
        
        void 		loadGribFile(QString fileName);
        GribReader *getGribReader()    {return gribReader;}
        bool        isGribReaderOk()   {return gribReader!=NULL && gribReader->isOk();}
        
        void     	setCurrentDate(time_t t);
        time_t   	getCurrentDate()         {return currentDate;}
        std::set<time_t> * getListDates()    {return &listDates;}
        
        GribPointInfo  getGribPointInfo(double x, double y);

        // Carte de couleurs du vent
        void draw_WIND_Color(QPainter &pnt, const Projection *proj, bool smooth);

        // Flèches de direction du vent espacées régulièrement
        void draw_WIND_Arrows(
                    QPainter &pnt, const Projection *proj, bool barbules, QColor arrowsColor);

        // Rectangle translucide sur la zone couverte par les données
        void show_CoverZone(QPainter &pnt, const Projection *proj);
        // Grille GRIB
        void draw_GribGrid(QPainter &pnt, const Projection *proj);
                
        QRgb   getWindColor(double v, bool smooth);


		void  drawColorMapGeneric_1D (
				QPainter &pnt, const Projection *proj, bool smooth,
				GribRecord *rec,
				QRgb (GribPlot::*function_getColor) (double v, bool smooth)
			);
		void  drawColorMapGeneric_2D (
				QPainter &pnt, const Projection *proj, bool smooth,
				GribRecord *recx, GribRecord *recy,
				QRgb (GribPlot::*function_getColor) (double v, bool smooth)
			);
		void  drawColorMapGeneric_Abs_Delta_Data (
				QPainter &pnt, const Projection *proj, bool smooth,
				GribRecord *recx, GribRecord *recy,
				QRgb (GribPlot::*function_getColor) (double v, bool smooth)
			);
        
        void drawWindArrowWithBarbs(
        			QPainter &pnt, int i, int j,
        			double vx, double vy,
        			bool south,
        			QColor arrowColor=Qt::white);
        			
		void duplicateFirstCumulativeRecord( bool mustDuplicate );
		void interpolateValues( bool b );
		void windArrowsOnGribGrid( bool b );

    
    private:
        void       	initNewGribPlot(bool interpolateValues_, bool windArrowsOnGribGrid_);
        GribReader 	*gribReader;
        
        QString 	fileName;
        
        time_t  	currentDate;
        std::set<time_t>    listDates;     // liste des dates des GribRecord

        QColor windColor[14];        // couleur selon la force du vent en beauforts
        QColor rainColor[17];
        int    mapColorTransp;

        QColor windArrowColor;        // couleur des flèches du vent

        int    windArrowSpace;        // distance mini entre flèches (pixels)
        int    windArrowSpaceOnGrid;  // distance mini entre flèches si affichage sur grille
        int    windBarbuleSpace;      // distance mini entre flèches (pixels)
        int    windBarbuleSpaceOnGrid;  // distance mini entre flèches
        
        int    windArrowSize;         // longueur des flèches
        int    windBarbuleSize;       // longueur des flèches

        bool    mustInterpolateValues;
        bool    drawWindArrowsOnGribGrid;
        bool	mustDuplicateFirstCumulativeRecord;
        bool	isCloudsColorModeWhite;
        

        void drawWindArrow(QPainter &pnt, int i, int j, double vx, double vy);
        
        void drawTransformedLine( QPainter &pnt,
                double si, double co,int di, int dj, int i,int j, int k,int l);
        
        void drawPetiteBarbule(QPainter &pnt, bool south,
                    double si, double co, int di, int dj, int b);
        void drawGrandeBarbule(QPainter &pnt,  bool south,
                    double si, double co, int di, int dj, int b);
        void drawTriangle(QPainter &pnt, bool south,
                    double si, double co, int di, int dj, int b);


};

#endif
