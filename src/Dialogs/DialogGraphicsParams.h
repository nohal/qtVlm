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

#ifndef DIALOGGRAPHICSPARAMS_H
#define DIALOGGRAPHICSPARAMS_H
#ifdef QT_V5
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QScrollArea>
#else
#include <QDialog>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QButtonGroup>
#include <QRadioButton>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QScrollArea>
#endif
#include "class_list.h"

//-------------------------------------------------------------------------------
class InputLineParams_testZone : public QWidget
{ Q_OBJECT
    public:
        InputLineParams_testZone(double width, QColor color)
        		{ lineWidth = width; lineColor=color;}
        		
        void setWidth (double width)
        		{ lineWidth = width; update(); }
        void setColor (QColor color)
        		{ lineColor=color; update(); }

                double  getLineWidth()   {return lineWidth;}
		QColor getLineColor()   {return lineColor;}
    
    private:
                double  lineWidth;
		QColor lineColor;
    	void   paintEvent(QPaintEvent *);
    	void   mouseReleaseEvent(QMouseEvent * );
};

//--------------------------------------------------------------
class InputLineParams : public QWidget
{ Q_OBJECT
    public:
        InputLineParams(double width, QColor color,
                                                double defaultWidth, QColor defaultColor,
						QWidget *parent,
                                                double minWidth=0.2, double maxWidth=6,int decimal=1,
                                                bool useTestZone=true
        			);
        			
        double  getLineWidth() {return sbWidth->value();}
        QColor getLineColor() {return testZone->getLineColor();}
    
    private:
                double defaultWidth;
                bool useTestZone;
		QColor defaultColor, color;
		
		QDoubleSpinBox *sbWidth;
		QPushButton    *bdDefault;
		InputLineParams_testZone *testZone;

	private slots:
		void lineWidthChanged();
		void resetDefault();
};


//==============================================================
class InputColor_testZone : public QWidget
{ Q_OBJECT
    public:
        InputColor_testZone(QColor color, QWidget *parent);
        
        void setColor (QColor color)
        		{ this->color=color; update(); }
    	
        QColor getColor() {return color;}
    
    private:
		QColor color;
	
	private slots:
    	void   paintEvent(QPaintEvent *);
    	void   mouseReleaseEvent(QMouseEvent * );
};
//--------------------------------------------------------------
class InputColor : public QWidget
{ Q_OBJECT
    public:
        InputColor(QColor color, QColor defaultColor, QWidget *parent);
    	
        QColor getColor() {return testZone->getColor();}
    
    private:
		QColor defaultColor, color;
		InputColor_testZone *testZone;
		QPushButton    *bdDefault;
	
	private slots:
		void resetDefault();
};



//==============================================================
class DialogGraphicsParams : public QDialog
{ Q_OBJECT
    public:
        DialogGraphicsParams(myCentralWidget * parent);
        ~DialogGraphicsParams();
    
    public slots:
        void slotBtOK();
        void slotBtCancel();
    

        void slot_screenResize();
protected:
        void resizeEvent ( QResizeEvent * event );
    private:
        QFrame *frameGui;
        QGridLayout *layout;
        
        QPushButton *btOK;
        QPushButton *btCancel;

        InputColor *inputBackgroundColor;
        InputColor *inputSeaColor;
        InputColor *inputLandColor;
        
        InputLineParams *inputSeaBordersLine;
        InputLineParams *inputBoundariesLine;
        InputLineParams *inputRiversLine;
        InputLineParams *inputIsobarsLine;
        InputLineParams *inputIsotherms0Line;
        InputLineParams *inputOpacity;
        InputLineParams *inputOpacityNuit;
        InputLineParams *inputEstimeLine;
        InputLineParams *inputNextGateLine;
        InputLineParams *inputGateLine;
        InputLineParams *inputRouteLine;
        InputLineParams *inputTraceLine;
        InputLineParams *inputOrthoLine;

        QFrame * createFrameGui(QWidget *parent);
        QScrollArea *scroll;
};


#endif
