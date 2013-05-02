/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2011 - Christophe Thomas aka Oxygen77

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

#ifndef BOARDTOOLS_H
#define BOARDTOOLS_H

#include <QDoubleSpinBox>

/************************************************************/
/*   tool_edtSpinBox                                        */
/************************************************************/
class tool_edtSpinBox: public QDoubleSpinBox
{ Q_OBJECT
    public:
        tool_edtSpinBox(QWidget * parent=0);

    signals:
        void hasEvent(void);

    protected:
        void keyPressEvent ( QKeyEvent * event );
        void keyReleaseEvent ( QKeyEvent * event );

    private:
        QWidget * parent;
};

class tool_windAngle: public QWidget
{ Q_OBJECT
    public:
        tool_windAngle(QWidget * parent=0);
        void draw(QPainter * painter);
        void setValues(double heading,double windDir, double windSpeed, double WPdir,double newHeading);

    protected:
        void paintEvent(QPaintEvent * event);

    private:
        QImage *img_fond;
        QImage *img_compas;
        QImage *img_boat;
        QImage *img_boat2;
        int w,h;
        double heading,windDir,windSpeed,WPdir,newHeading;

        QColor windSpeed_toColor(void);

};

#endif // BOARDTOOLS_H
