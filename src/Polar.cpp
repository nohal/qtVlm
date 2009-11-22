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
#include <QDateTime>

#include "Polar.h"

Polar::Polar()
{
    loaded=false;
    nbUsed=0;
}

Polar::Polar(QString fname)
{
    loaded=false;
    nbUsed=0;
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
        loaded=false;          \
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
         loaded=false;          \
         return;                \
    }                          \
}

void Polar::setPolarName(QString fname)
{
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
    list = line.split('\t');
    if(list[0] != "TWA\\TWS")
    {
        QMessageBox::warning(0,QObject::tr("Lecture de polaire"),
             QString(QObject::tr("Fichier %1 invalide")).arg(fname));
        file.close();
        return;
    }
    list.removeLast();
    int i;
    for(i=1;i<list.count();i++) tws.append(list[i].toFloat());
    while(true)
    {
        line=stream.readLine();
        if(line.isNull()) break;
        list = line.split("\t");
        twa.append(list[0].toFloat());
        for(i=1;i<list.count();i++)
        {
            if(i>tws.count()) break;
            polar_data.append(list[i].toFloat());
        }
        while(i<=tws.count())
            polar_data.append(0);
    }
    mid_twa=qRound(twa.count()/2);
    mid_tws=qRound(tws.count()/2);
    /* polaire chargée */
    loaded=true;
/* pre-calculate B-VMG for every tws at 0.1 precision with a twa step of 1 and then .1 */
    float ws=qRound(0);
    float wa=qRound(0);
    float bvmg,bvmg_d,bvmg_u,wa_u,wa_d,wa_limit;
    do
    {
        bvmg_u=bvmg_d=qRound(0);
        do
        {
//          qWarning()<<ws<<" "<<wa;
            bvmg=getSpeed(ws,wa)*cos(degToRad(wa));
            if(bvmg_u<bvmg)
            {
                bvmg_u=bvmg;
                wa_u=wa;
            }
            if(bvmg_d>bvmg)
            {
                bvmg_d=bvmg;
                wa_d=wa;
            }
            wa=qRound(wa+1);
        } while(wa<=181.00);
        wa=wa_u-1;
        wa_limit=wa_u+1;
        if(wa<0) wa=qRound(0);
        if(wa_limit<1) wa_limit=qRound(1);
        if(wa_limit>180) wa_limit=qRound(180);
        bvmg_u=qRound(0);
        do
        {
//                qWarning()<<ws<<" "<<wa;
            bvmg=getSpeed(ws,wa)*cos(degToRad(wa));
            if(bvmg_u<bvmg)
            {
                bvmg_u=bvmg;
                wa_u=wa;
            }
            wa=wa+0.1;
        } while(wa<=(wa_limit+0.1000));
        wa=wa_d-1;
        wa_limit=wa_d+1;
        if(wa<0) wa=qRound(0);
        if(wa_limit<1) wa_limit=qRound(1);
        if(wa_limit>180) wa_limit=qRound(180);
        bvmg_d=qRound(0);
        do
        {
//                qWarning()<<ws<<" "<<wa;
            bvmg=getSpeed(ws,wa)*cos(degToRad(wa));
            if(bvmg_d>bvmg)
            {
                bvmg_d=bvmg;
                wa_d=wa;
            }
            wa=wa+0.1;
        }while(wa<=wa_limit+0.1);
        best_vmg_up.append(wa_u);
        best_vmg_down.append(wa_d);
        wa=qRound(0);
        ws=ws+.1;
    }while(ws<=60.1);
    file.close();
}

void Polar::printPolar(void)
{
//    for(int j=0;j<(windAngle_max-windAngle_min)/windAngle_step;j++)
//    {
//        QString str=QString().setNum(j*windAngle_step)+"\t";
//        for(int i=0;i<(windSpeed_max-windSpeed_min)/windSpeed_step;i++)
//            str+=(QString().setNum((polar_data[j])[i])+"\t");
//        qWarning()<< str;
//    }
}

float Polar::getBvmgUp(float windSpeed)
{
    if(!loaded)
        return 0;
    return(best_vmg_up[qRound(windSpeed*10)]);
}
float Polar::getBvmgDown(float windSpeed)
{
    if(!loaded)
        return 0;
    return(best_vmg_down[qRound(windSpeed*10)]);
}
float Polar::getSpeed(float windSpeed, float angle)
{

    int i1,i2,k1,k2,k;
    float a,b,c,d;
    float infSpeed,supSpeed;
    float boatSpeed;

    if(!loaded)
        return 0;

    angle=qAbs(angle);
    if(windSpeed>tws.last()) windSpeed=tws.last();
    if(windSpeed<tws.first()) windSpeed=tws.first();
    if(angle>twa.last()) angle=twa.last();
    if(angle<twa.first()) angle=twa.first();
    if (angle>=twa[mid_twa])
        k=mid_twa;
    else
        k=0;
    for(i2=k;i2<twa.count();i2++)
    {
        if(angle<=twa[i2])
            break;
    }
    if(twa[i2]==angle)
        i1=i2;
    else
        i1=i2-1;
    if (windSpeed>=tws[mid_tws])
        k=mid_tws;
    else
        k=0;
    for(k2=k;k2<tws.count();k2++)
    {
        if(windSpeed<=tws[k2])
            break;
    }
    if(tws[k2]==windSpeed)
        k1=k2;
    else
        k1=k2-1;
    a=polar_data[tws.count()*i1+k1];
    b=polar_data[tws.count()*i2+k1];
    c=polar_data[tws.count()*i1+k2];
    d=polar_data[tws.count()*i2+k2];
    if(i1==i2)
        infSpeed=a;
    else
        infSpeed=a+(angle-twa[i1])*(b-a)/(twa[i2]-twa[i1]);
    if(i1==i2)
        supSpeed=c;
    else
        supSpeed=c+(angle-twa[i1])*(d-c)/(twa[i2]-twa[i1]);
    if(supSpeed==infSpeed)
        boatSpeed=infSpeed;
    else
        boatSpeed=infSpeed+(windSpeed-tws[k1])*(supSpeed-infSpeed)/(tws[k2]-tws[k1]);
    return boatSpeed;
}

void Polar::clearPolar(void)
{
/*clear previous polar data*/
    while(polar_data.count()!=0)
    {
        polar_data.removeLast();
    }
    while(tws.count()!=0)
    {
        tws.removeLast();
    }
    while(twa.count()!=0)
    {
        twa.removeLast();
    }
}

polarList::polarList(void)
{
    polars.clear();
}

polarList::~polarList(void)
{
    while(polars.count())
        delete polars.first();
    polars.clear();
}

Polar * polarList::needPolar(QString fname)
{
    if(fname=="")
        return NULL;

    Polar * res=NULL;
    QListIterator<Polar*> i (polars);

    while(i.hasNext())
    {
        Polar * item = i.next();
        if(item->getName()==fname)
        {
            res=item;
            item->nbUsed++;
            break;
        }
    }
    if(!res)
    {
        res = new Polar(fname);
        if(res)
        {
            res->nbUsed++;
            polars.append(res);
        }
    }

    return res;
}

void polarList::releasePolar(QString fname)
{
    QListIterator<Polar*> i (polars);
    while(i.hasNext())
    {
        Polar * item = i.next();
        if(item->getName()==fname)
        {
            item->nbUsed--;
            if(item->nbUsed==0)
            {
                polars.removeAll(item);
                delete item;
            }
            break;
        }
    }
}

void polarList::stats(void)
{
    qWarning() << "Nb polar: " << polars.count();
    QListIterator<Polar*> i (polars);
    int k=0;
    while(i.hasNext())
    {
        Polar * item = i.next();
        qWarning() << k << ": " << item->getName() << "(nb used=" << item->nbUsed << ")";
        k++;
    }
}
