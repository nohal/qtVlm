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

#ifndef __POLAR_H
#define __POLAR_H

#include <QString>
#include <QList>
#include <QObject>
#include <cmath>
#define PI     M_PI
#define PI_2   M_PI_2
#define PI_4   M_PI_4
#define TWO_PI M_PI * 2

#define degToRad(angle) (((angle)/180.0) * PI)
#define radToDeg(angle) (((angle)*180.0) / PI)

class Polar : public QObject
{Q_OBJECT
    public:
        Polar(void);
        Polar(QString fname);

        QString getName() { if(loaded) return name; else return ""; }
        float getSpeed(float windSpeed, float angle);
        float getBvmgUp(float windSpeed);
        float getBvmgDown(float windSpeed);

        int nbUsed;

    private:
        QList<float> polar_data;
        QList<float> tws;
        QList<float> twa;
        QList<float> best_vmg_up;
        QList<float> best_vmg_down;
        int mid_twa,mid_tws;
        bool loaded;
        QString name;

        int windSpeed_min,windSpeed_max,windSpeed_step;
        int windAngle_min,windAngle_max,windAngle_step;

        void clearPolar(void);
        void setPolarName(QString fname);
        void printPolar(void);
};

class polarList : public QObject
{ Q_OBJECT
    public:
        polarList(void);
        ~polarList(void);

        Polar * needPolar(QString fname);
        void releasePolar(QString fname);

        void stats(void);

    private:
        QList<Polar*> polars;
};

#endif
