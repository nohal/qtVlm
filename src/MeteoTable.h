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

#ifndef METEOTABLE_H
#define METEOTABLE_H

#include <QDialog>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QScrollArea>
#include <QPushButton>

#include "GribPlot.h"



//===================================================================
class TableCell : public QWidget
{ Q_OBJECT
    public:
        TableCell(QWidget *parent, QString txt, bool bold,
        			QColor bgcolor, QColor fgcolor=Qt::black);
		
		void setBorders(int num) { borders=num; }
		static const uint none  = 0;
		static const uint north = 1;
		static const uint west  = 2;
		static const uint south = 4;
		static const uint east  = 8;
		static const uint all   = 1+2+4+8;

    protected:
	    void  	paintEvent(QPaintEvent *event);
	    QColor  bgcolor, bordercolor;
	    QLabel  *label;
	    int     borders;
};

//===================================================================
class TableCell_Wind : public TableCell
{ Q_OBJECT
    public:
        TableCell_Wind(
        			float vx, float vy, bool south,
        			GribPlot *gribplot,
        			QWidget *parent, QString txt, bool bold,
        			QColor bgcolor, QColor fgcolor=Qt::black
				);
    
    protected:
    	QColor    windArrowsColor;
    	float 	  vx, vy;
    	bool	  south;
    	GribPlot *gribplot;
	    void  	paintEvent(QPaintEvent *event);
};


//===================================================================
class MeteoTableWidget : public QWidget
{ Q_OBJECT
    public:
        MeteoTableWidget(GribPlot *gribplot, float lon, float lat, QWidget *parent);
    
    private:
        float        lon, lat;
        GribPlot    *gribplot;
		QGridLayout *layout;
		
		void  createTable();
		void addLine_Wind(int lig);
		void addLine_CloudCover(int lig);
		void addLine_Rain(int lig);
		void addLine_Temperature(int lig);
		void addLine_Pressure(int lig);

		void addCell_content( QString txt, 
				QGridLayout *layout,int lig,int col,
				int    rowspan=1,
				int    colspan=1,
				QColor bgcolor=Qt::white,
				bool   isWindCell=false,
				float  vx=0,
				float  vy=0
				);
				
		void addCell_title( QString txt, bool bold,
				QGridLayout *layout,int lig,int col, int rowspan=1,int colspan=1);

};


//===================================================================
// MeteoTable : dialog + MeteoTableWidget
//===================================================================
class MeteoTableDialog : public QDialog
{ Q_OBJECT
    public:
        MeteoTableDialog(GribPlot *gribplot, float lon, float lat, QString posName="");
        ~MeteoTableDialog();
    
    private:
		MeteoTableWidget *meteoTableWidget;
     	QPushButton *btClose;
     	
		void closeEvent(QCloseEvent *) {delete this;};
	private slots :
		void reject()	{delete this;};
};


#endif
