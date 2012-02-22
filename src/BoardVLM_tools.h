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
***********************************************************************/

#ifndef VLM_BOARD_TOOLS_H
#define VLM_BOARD_TOOLS_H

#include <QLabel>
#include <QHBoxLayout>
#include <QImage>
#include <QPainter>
#include <QDoubleSpinBox>

class tool_navCenter: public QWidget
{ Q_OBJECT
    public:
        tool_navCenter(QWidget * parent=0);
        void draw(QPainter * painter);
        void setValues(double lat, double lon, double speed, double avg, double heading,
                               double dnm, double loch, double ortho, double loxo, double vmg);

    protected:
        void paintEvent(QPaintEvent * event);

    private:
        QImage *img_fond;
        int w,h;
        QRgb bg;
        double lat,lon;
        double speed,avg,heading;
        double dnm,loch,ortho,loxo,vmg;
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

class tool_windStation: public QWidget
{ Q_OBJECT
    public:
        tool_windStation(QWidget * parent=0);
        void draw(QPainter * painter);
        void setValues(double windDir, double windSpeed, double windAngle);

    protected:
        void paintEvent(QPaintEvent * event);

    private:
        QImage *img_fond;
        int w,h;

        double windSpeed,windDir,windAngle;

};

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

#endif
