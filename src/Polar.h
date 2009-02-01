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

class Polar : public QObject
{Q_OBJECT
    public:
        Polar(QWidget *parentWindow);
        Polar(QString fname,QWidget *parentWindow);

        void setPolarName(QString fname);
        QString getName() { if(loaded) return name; else return ""; }
        float getSpeed(float windSpeed, float angle);

    private:
        QList<float*> polar_data;
        bool loaded;
        QString name;

        int windSpeed_min,windSpeed_max,windSpeed_step;
        int windAngle_min,windAngle_max,windAngle_step;

        void clearPolar(void);
        void printPolar(void);
};

#endif
