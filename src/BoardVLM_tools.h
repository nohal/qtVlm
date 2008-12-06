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

class tool_navCenter: public QWidget
{ Q_OBJECT
    public:
        tool_navCenter(QWidget * parent=0);
        void draw(QPainter * painter);
        void setValues(float lat, float lon, float speed, float avg, float heading,
                               float dnm, float loch, float ortho, float loxo, float vmg);

    protected:
        void paintEvent(QPaintEvent * event);

    private:
        QImage *img_fond;
        int w,h;
        QRgb bg;
        float lat,lon;
        float speed,avg,heading;
        float dnm,loch,ortho,loxo,vmg;
};

class tool_windAngle: public QWidget
{ Q_OBJECT
    public:
        tool_windAngle(QWidget * parent=0);
        void draw(QPainter * painter);
        void setValues(float heading,float windDir, float windSpeed, float WPdir);
        
    protected:
        void paintEvent(QPaintEvent * event);

    private:
        QImage *img_fond;
        QImage *img_compas;
        QImage *img_boat;
        int w,h;
        float heading,windDir,windSpeed,WPdir;

        QColor windSpeed_toColor(void);

};

class tool_windStation: public QWidget
{ Q_OBJECT
    public:
        tool_windStation(QWidget * parent=0);
        void draw(QPainter * painter);
        void setValues(float windDir, float windSpeed, float windAngle);

    protected:
        void paintEvent(QPaintEvent * event);

    private:
        QImage *img_fond;
        int w,h;

        float windSpeed,windDir,windAngle;

};


#endif
