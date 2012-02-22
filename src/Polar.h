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
#include <QFile>
#include <QMutex>

#include "inetClient.h"

#define PI     M_PI
#define PI_2   M_PI_2
#define PI_4   M_PI_4
#define TWO_PI M_PI * 2

#define degToRad(angle) (((angle)/180.0) * PI)
#define radToDeg(angle) (((angle)*180.0) / PI)

class Polar : public QObject
{Q_OBJECT
    public:
        Polar(MainWindow * mainWindow);
        Polar(QString fname,MainWindow * mainWindow);

        QString getName() { if(loaded) return name; else return ""; }
        double   getSpeed(double windSpeed, double angle);
        double   getBvmgUp(double windSpeed);
        double   getBvmgDown(double windSpeed);
        bool    isLoaded() { return loaded; }
        double   getMaxSpeed() {return maxSpeed;}
        bool    getIsCsv(){return this->isCsv;}
        int     nbUsed;

        void    bvmg(double bt_longitude,double bt_latitude, double wp_longitude, double wp_latitude,
                          double w_angle, double w_speed,
                          double *heading, double *wangle);
        void    bvmgWind(double w_angle, double w_speed,double *wangle);
        void    getBvmg(double twaOrtho,double tws,double *twa);

    private:
        MainWindow * mainWindow;
        QList<double> polar_data;
        QList<double> tws;
        QList<double> twa;
        QList<double> best_vmg_up;
        QList<double> best_vmg_down;
        int     mid_twa,mid_tws;
        bool    loaded;
        QString name;

        int     windSpeed_min,windSpeed_max,windSpeed_step;
        int     windAngle_min,windAngle_max,windAngle_step;

        double   myGetSpeed(double windSpeed, double angle, bool force);

        void    clearPolar(void);
        void    setPolarName(QString fname);
        void    printPolar(void);
        double   maxSpeed;
        bool    isCsv;
        double   A180(double angle);
        void    myBvmgWind(double w_angle, double w_speed,double *wangle);
        double  A360(double hdg);
        QFile   fileVMG;
};

class polarList : public QObject, public inetClient
{ Q_OBJECT
    public:
        polarList(inetConnexion * inet, MainWindow * mainWindow);
        ~polarList(void);

        Polar * needPolar(QString fname);
        void releasePolar(QString fname);

        void stats(void);

        /* inetClient */
        void requestFinished(QByteArray res);
        void get_polarList(void);

    public slots:
        void getPolar(QString);

    signals:
        void polarLoaded(QString,Polar *);

    private:
        QList<Polar*> polars;
        QList<QString> loadList;
        bool isLoading;
        MainWindow * mainWindow;
        void loadPolars(void);
};

#endif
