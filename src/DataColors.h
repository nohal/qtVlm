/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2013 - Christophe Thomas aka Oxygen77

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

#ifndef DATACOLORS_H
#define DATACOLORS_H

#include <QColor>
#include <QMap>

#include "class_list.h"
#include "dataDef.h"

class ColorElement {
    public:
        ColorElement(int transparence);

        QRgb get_color(double v, bool smooth);

        void add_color(double value,QRgb color);
        void set_param(double coef,double offset) {this->coef=coef; this->offset=offset;}

        FCT_GET(double,coef)
        FCT_GET(double,offset)


        void print_data(void);

    private:
        QMap<double,QRgb> colorMap;

        double minVal;
        double maxVal;

        double coef;
        double offset;

        int transparence;
};

extern QMap<QString,ColorElement*> colorMap;

class DataColors
{
    public:
        static QRgb get_color(QString type,double v, bool smooth);
        static QRgb get_color_windColorScale(double v, double min, double max, bool smooth);

        static ColorElement * get_colorElement(QString type);

        static void load_colors(int transparence);

        static void print_data(void);

};

#endif // DATACOLORS_H
