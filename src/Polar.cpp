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

#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDebug>

#include "Polar.h"

Polar::Polar(QWidget *)
{
    loaded=false;
}

Polar::Polar(QString fname,QWidget *)
{
    loaded=false;
    setPolarName(fname);
}

#define getInt(INC,RES) { \
    bool ok;                 \
    RES=list[INC].toInt(&ok);  \
    if(!ok)                    \
    {                          \
         QMessageBox::warning(0,QObject::tr("Lecture de polaire"), \
             QString(QObject::tr("Fichier %1 invalide")).arg(fname)); \
        file.close();          \
        clearPolar();          \
        return;                \
    }                          \
}

#define getFloat(INC,RES) { \
    bool ok;                 \
    RES=list[INC].toFloat(&ok);  \
    if(!ok)                    \
    {                          \
         QMessageBox::warning(0,QObject::tr("Lecture de polaire"), \
             QString(QObject::tr("Fichier %1 invalide")).arg(fname)); \
         file.close();          \
         clearPolar();          \
         return;                \
    }                          \
}

void Polar::setPolarName(QString fname)
{
    int val,prev;
    int nbWindSpeedVal;
    int nbWindAngleVal=0;
    float * dataLine;
    float valFloat;
    if(loaded)
        clearPolar();
    
    loaded=false;
    name=fname;
    fname = "polar/"+fname+".pol";
    
    QFile file(fname);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text ))
    {
         QMessageBox::warning(0,QObject::tr("Lecture de polaire"),
             QString(QObject::tr("Ne peux ouvrir le fichier %1")).arg(fname));
        return;
    }
    QTextStream stream(&file);
    QString line;
    QStringList list;
    /* read first line to see line length */
    line=stream.readLine();
    if(line.isNull())
    {
        QMessageBox::warning(0,QObject::tr("Lecture de polaire"),
             QString(QObject::tr("Fichier %1 vide")).arg(fname));
        file.close();
        return;
    }
    list = line.split(";");
    nbWindSpeedVal=list.count()-1;
    getInt(1,windSpeed_min);
    
    getInt(2,val); /*we use val var as this can be the max*/
    windSpeed_step=val-windSpeed_min;
    prev = val;
    if(windSpeed_step<0)
    {
        QMessageBox::warning(0,QObject::tr("Lecture de polaire"),
             QString(QObject::tr("Fichier %1 invalide - step vitesse non positif")).arg(fname));
        file.close();
        return;
    }
    /*control that step is constant*/
    for(int i=3;i<list.count();i++)
    {
        getInt(i,val);
        if((val-prev)!= windSpeed_step)
        {
            QMessageBox::warning(0,QObject::tr("Lecture de polaire"),
             QString(QObject::tr("Fichier %1 invalide - step vitesse non constant (speed=%2,step=%3)")).arg(fname)
                    .arg(val).arg(val-prev));
            file.close();
            return;
        }
        prev=val;
    }
    windSpeed_max=val; /* val could have bee set in the loop or before windSpeed_step computation*/
    /* we can now read the polar data */
    nbWindAngleVal=0;
    while(true)
    {
        line=stream.readLine();
        if(line.isNull()) break;
        list = line.split(";");
        if(list.count()!=(nbWindSpeedVal+1))
        {
            qWarning() << "line " << nbWindAngleVal << " wrong number of data: " << list.count() << "  => nxtLine";
            continue;
        }
        nbWindAngleVal++;
        getInt(0,val);
        if(nbWindAngleVal==1)  /* this is the first line */
        {
            windAngle_min=val;
        }
        else
        {
            if(nbWindAngleVal==2) /* this is the second line => compute step */
            {
                windAngle_step=val-prev;
                if(windAngle_step<0)
                {
                    QMessageBox::warning(0,QObject::tr("Lecture de polaire"),
                        QString(QObject::tr("Fichier %1 invalide - step angle non positif")).arg(fname));
                    file.close();
                    clearPolar();
                    return;
                }
            }
            else if(windAngle_step != (val-prev)) /* we have to check if step is constant */
            {
                QMessageBox::warning(0,QObject::tr("Lecture de polaire"),
                        QString(QObject::tr("Fichier %1 invalide - step angle non constant (val=%2,step=%3)"))
                                .arg(fname).arg(val).arg(val-prev));
                file.close();
                clearPolar();
                return;
            }
        }
        /* we can always set the max*/
        windAngle_max=val;
        prev=val;
        /* create the line */
        dataLine = new float[nbWindSpeedVal];
        for(int i=0;i<nbWindSpeedVal;i++)
        {
            getFloat(i+1,valFloat);
            dataLine[i]=valFloat;
        }
        polar_data.append(dataLine);
    }
    loaded=true;
    file.close();
}

void Polar::printPolar(void)
{
    for(int j=0;j<(windAngle_max-windAngle_min)/windAngle_step;j++)
    {
        QString str=QString().setNum(j*windAngle_step)+"\t";
        for(int i=0;i<(windSpeed_max-windSpeed_min)/windSpeed_step;i++)
            str+=(QString().setNum((polar_data[j])[i])+"\t");
        qWarning()<< str;
    }
}

float Polar::getSpeed(float windSpeed, float angle)
{

    int val;
    int infSpeed_idx,infAngle_idx;
    float a,b,c,d;
    float infSpeed,supSpeed;
    float boatSpeed;
    
    angle=qAbs(qRound(angle));

    /* ugly fix to manage out of bound angle or wind speed*/
    if(angle>=windAngle_max) angle=windAngle_max-0.1;
    if(angle<windAngle_min) return 0;

    if(windSpeed<windSpeed_min) return 0;
    if(windSpeed>=windSpeed_max) windSpeed=windSpeed_max-0.1;

    /* find index in data structure */
    val=(int)windSpeed;
    infSpeed_idx =(float)((val-windSpeed_min)/windSpeed_step);
    val=(int)angle;
    infAngle_idx =(float)((val-windAngle_min)/windAngle_step);

    /* find data */
    a = (polar_data[infAngle_idx])[infSpeed_idx];
    b = (polar_data[infAngle_idx+1])[infSpeed_idx];
    c = (polar_data[infAngle_idx])[infSpeed_idx+1];
    d = (polar_data[infAngle_idx+1])[infSpeed_idx+1];

    /* compute speed: linear interpolation on angle (constant wind speed) and then on wind speed */
    infSpeed = a + (angle-(windAngle_min+infAngle_idx*windAngle_step))*(b-a)/(windAngle_step);
    supSpeed = c + (angle-(windAngle_min+infAngle_idx*windAngle_step))*(d-c)/(windAngle_step);
    boatSpeed = infSpeed+(windSpeed-(windSpeed_min+infSpeed_idx*windSpeed_step))*
            (supSpeed-infSpeed)/(windSpeed_step);
    
    return boatSpeed;
}

void Polar::clearPolar(void)
{
/*clear previous polar data*/
    while(polar_data.count()!=0)
    {
        delete polar_data.last();
        polar_data.removeLast();
    }
}
