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
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>
#include <QDateTime>
#include <QProgressDialog>
#include "MainWindow.h"
#include "Polar.h"
#include "dataDef.h"
#include "parser.h"
#include "Orthodromie.h"
#include "settings.h"
#include "boat.h"

Polar::Polar(MainWindow * mainWindow)
{
    loaded=false;
    nbUsed=0;
    this->mainWindow=mainWindow;
}

Polar::Polar(QString fname,MainWindow * mainWindow)
{
    loaded=false;
    nbUsed=0;
    this->mainWindow=mainWindow;
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

#define getDouble(INC,RES) { \
    bool ok;                 \
    RES=list[INC].toDouble(&ok);  \
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
    isCsv=true;
    loaded=false;
    clearPolar();
    if(this->mainWindow->getSelectedBoat() && this->mainWindow->getSelectedBoat()->getType()==BOAT_REAL)
        coeffPolar=Settings::getSetting("polarEfficiency",100).toInt()/100.0;
    else
        coeffPolar=1.0;

    //qWarning() << "Opening polar " << fname;

    name=fname;
    QString nameF = "polar/"+fname+".csv";
    QFile file(nameF);
    if (fname.endsWith(".csv",Qt::CaseInsensitive) || fname.endsWith(".pol",Qt::CaseInsensitive))
    {
        nameF="polar/"+fname;
        file.setFileName(nameF);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text ))
        {
            QMessageBox::warning(0,QObject::tr("Lecture de polaire"),
                QString(QObject::tr("Impossible d'ouvrir le fichier %1")).arg(name));
            return;
        }

        isCsv=fname.endsWith("csv",Qt::CaseInsensitive);
    }
    else
    {
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text ))
        {
            isCsv=false;
            nameF = "polar/"+fname+".pol";
            file.setFileName(nameF);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text ))
            {
                 QMessageBox::warning(0,QObject::tr("Lecture de polaire"),
                     QString(QObject::tr("Impossible d'ouvrir le fichier %1 (ni en .csv ni en .pol)")).arg(name));
                 return;
            }
        }
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
    if(isCsv)
        list = line.split(';');
    else
        list = line.split('\t');
    if(list[0].toUpper() != "TWA\\TWS" && list[0].toUpper() != "TWA/TWS" && list[0].toUpper() != "TWA")
    {
        QMessageBox::warning(0,QObject::tr("Lecture de polaire"),
             QString(QObject::tr("Fichier %1 invalide (doit commencer par TWA\\TWS et non '%2')"))
                        .arg(fname)
                        .arg(list[0]));
        file.close();
        return;
    }
    list.removeLast();
    int i;
    for(i=1;i<list.count();i++) tws.append(list[i].toDouble());
    while(true)
    {
        line=stream.readLine();
        if(line.isNull()) break;
        if (isCsv)
            list = line.split(";");
        else
            list = line.split("\t");
        twa.append(list[0].toDouble());
        for(i=1;i<list.count();i++)
        {
            if(i>tws.count()) break;
            polar_data.append(list[i].toDouble()*this->coeffPolar);
        }
        while(i<=tws.count())
            polar_data.append(0);
    }
    mid_twa=qRound(twa.count()/2);
    mid_tws=qRound(tws.count()/2);
    /* polaire chargee */

/* pre-calculate B-VMG for every tws at 0.1 precision with a twa step of 1 and then .1 */

    double ws=qRound(0);
    double wa=qRound(0);
    double bvmg,bvmg_d,bvmg_u,wa_u,wa_d,wa_limit;
    maxSpeed=qRound(0);
    do
    {
        wa_u=qRound(0);
        wa_d=qRound(180);
        bvmg_u=bvmg_d=qRound(0);
        double speed;
        do
        {
            speed=myGetSpeed(ws,wa,true);
            if(speed>maxSpeed) maxSpeed=speed;
            bvmg=speed*cos(degToRad(wa));
            if(bvmg_u<bvmg) //bvmg is positive here
            {
                bvmg_u=bvmg;
                wa_u=wa;
            }
            if(bvmg_d>bvmg) //bvmg is negative here
            {
                bvmg_d=bvmg;
                wa_d=wa;
            }
            wa=qRound(wa+1);
        } while(wa<181.00);
        wa=wa_u-1;
        wa_limit=wa_u+1;
        if(wa<0) wa=qRound(0);
        if(wa_limit<1) wa_limit=qRound(1);
        if(wa_limit>180) wa_limit=qRound(180);
        bvmg_u=qRound(0);
        do
        {
            speed=myGetSpeed(ws,wa,true);
            if(speed>maxSpeed) maxSpeed=speed;
            bvmg=speed*cos(degToRad(wa));
            if(bvmg_u<bvmg)
            {
                bvmg_u=bvmg;
                wa_u=wa;
            }
            wa=wa+0.1;
        } while(wa<(wa_limit+0.1000));
        wa=wa_d-1;
        wa_limit=wa_d+1;
        if(wa<0) wa=qRound(0);
        if(wa_limit<1) wa_limit=qRound(1);
        if(wa_limit>180) wa_limit=qRound(180);
        bvmg_d=qRound(0);
        do
        {
            speed=myGetSpeed(ws,wa,true);
            if(speed>maxSpeed) maxSpeed=speed;
            bvmg=speed*cos(degToRad(wa));
            if(bvmg_d>bvmg)
            {
                bvmg_d=bvmg;
                wa_d=wa;
            }
            wa=wa+0.1;
        }while(wa<wa_limit+0.1);
        best_vmg_up.append(wa_u);
        best_vmg_down.append(wa_d);
        wa=qRound(0);
        ws=ws+.1;
    }while(ws<60.1);
    loaded=true;
    QFileInfo fi(file.fileName());
    QString nameFVmg = "polar/"+fi.baseName()+".vmg";
    fileVMG.setFileName(nameFVmg);
    if (fileVMG.open(QIODevice::ReadOnly | QIODevice::Text ))
    {
        if(fileVMG.size()<4329500 || fileVMG.size()>4329700)
            fileVMG.remove();
        else
        {
            QFileInfo info1(fileVMG);
            QFileInfo info2(file);
            if(this->mainWindow->isStartingUp && info1.lastModified()<info2.lastModified())
                fileVMG.remove();
            else
            {
                file.close();
                return;
            }
        }
    }
    file.close();
//precalculate regular VMGs
    //qWarning() << "Start computing vmg";
    fileVMG.open(QIODevice::WriteOnly | QIODevice::Text );
    double vmg=0;
    QTextStream sVmg(&fileVMG);
    QString ssVmg;
    QProgressDialog * progress;
    progress=new QProgressDialog((QWidget*)mainWindow);
    progress->setWindowModality(Qt::ApplicationModal);
    progress->setLabelText(tr("Pre-calcul des valeurs de VMG pour ")+fname);
    progress->setMaximum(600);
    progress->setMinimum(0);
    progress->setCancelButton(NULL);
    progress->setMaximumHeight(100);
    progress->show();
    for (int tws=0;tws<601;tws++)
    {
        progress->setValue(tws);
        for (int twa=0;twa<1801;twa++)
        {
            bvmgWind((double) twa/10.0,(double) tws/10.0,&vmg);
            ssVmg.sprintf("%.4d",qRound(A360(vmg)*10.0));
            sVmg<<ssVmg;
        }
    }
    fileVMG.close();
    if(!fileVMG.open(QIODevice::ReadOnly | QIODevice::Text ))
        qWarning()<<"fileVMG could not be re-opened!";
    progress->close();
    delete progress;
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

double Polar::getBvmgUp(double windSpeed)
{
    if(!loaded)
        return 0;
    if ( qRound(windSpeed*10) <= best_vmg_up.count()-1 )
        return(best_vmg_up[qRound(windSpeed*10)]);
    else
        return(best_vmg_up.last());
}
double Polar::getBvmgDown(double windSpeed)
{
    if(!loaded)
        return 0;
    if ( qRound(windSpeed*10) <= best_vmg_down.count()-1 )
        return(best_vmg_down[qRound(windSpeed*10)]);
    else
        return(best_vmg_down.last());
}

double Polar::getSpeed(double windSpeed, double angle)
{
    return myGetSpeed(windSpeed,angle,false);
}

double Polar::myGetSpeed(double windSpeed, double angle, bool force)
{
    //qWarning() << "My get speed";
    int i1,i2,k1,k2,k;
    double a,b,c,d;
    double infSpeed,supSpeed;
    double boatSpeed;

    if(!loaded && !force)
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
    if(i2==twa.count())
        i2=twa.count()-1;
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
    if(k2==tws.count())
        k2=tws.count()-1;
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
        if(k1==k2) //due to rounding problems this can occurs even if (infSpeed!=supSpeed)
            boatSpeed=(infSpeed+supSpeed)/2.0;
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

/***************************************************************************************/
/* VMG                                                                                 */
/*                                                                                     */
/* Part of the code comes from C lib of VLM: (c) 2008 by Yves Lafon                    */
/* see http://www.virtual-loup-de-mer.org/ or http://dev.virtual-loup-de-mer.org/vlm/  */
/* Contact: <yves@raubacapeu.net>                                                      */
/***************************************************************************************/

void Polar::bvmg(double bt_longitude,double bt_latitude, double wp_longitude, double wp_latitude,
                  double w_angle, double w_speed,
                  double *heading, double *wangle)
{
    double maxwangle,twaOrtho;
    double maxheading;
    double wanted_heading;

    w_angle=degToRad(w_angle);
    Orthodromie orth(bt_longitude,bt_latitude,wp_longitude,wp_latitude);
    wanted_heading=orth.getAzimutRad();
    twaOrtho = w_angle - wanted_heading;
    myBvmgWind(twaOrtho,w_speed,&maxwangle);
    maxheading = fmod((wanted_heading-maxwangle+twaOrtho), TWO_PI);
    if (maxheading < 0)
    {
      maxheading += TWO_PI;
    }
    maxwangle = fmod(maxwangle, TWO_PI);
    if (maxwangle > PI)
    {
      maxwangle -= TWO_PI;
    } else if (maxwangle < -PI)
    {
      maxwangle += TWO_PI;
    }
#if 0
  qWarning() << "BVMG: Wind " << w_speed << "kts " << radToDeg(w_angle);
  qWarning() << "BVMG Wind Angle : wanted_heading " << radToDeg(wanted_heading);
  qWarning() << "BVMG Wind Angle : heading " << radToDeg(maxheading) << ", wind angle " << radToDeg(maxwangle);
#endif /* DEBUG */
    *heading = radToDeg(maxheading);
    *wangle = radToDeg(maxwangle);
}
void Polar::getBvmg(double twaOrtho, double tws, double *twaVMG)
{
    double twa;
    bool babord=false;
//    double debug=twaOrtho;
    twaOrtho=((double)qRound(twaOrtho*10.0))/10.0;
    if (twaOrtho<0)
    {
        twaOrtho=-twaOrtho;
        babord=true;
    }
    if(twaOrtho>180)
    {
        babord=!babord;
        twaOrtho=360-twaOrtho;
    }
    twaOrtho=twaOrtho*10.0;
    char data[5];
    fileVMG.seek(qRound(tws*10)*4*1801+qRound(twaOrtho)*4);
    fileVMG.read(data,4);
    data[4]='\0';
    twa=atoi(data);
//    if(twa>3600)
//        qWarning()<<"lol";
    twa=twa/10.0;
    if(babord)
    {
        twa=-twa;
    }
//    qWarning()<<"twaOrtho="<<debug<<" vmg="<<twa;
    *twaVMG=twa;
}
void Polar::bvmgWind(double twaOrtho, double tws, double *twaVMG)
{
    double twa;
    myBvmgWind(degToRad(twaOrtho),tws,&twa);
    twa = fmod(twa, TWO_PI);
    if (twa > PI)
    {
      twa -= TWO_PI;
    } else if (twa < -PI)
    {
      twa += TWO_PI;
    }
    *twaVMG=radToDeg(twa);
}
void Polar::myBvmgWind(double w_angle, double w_speed, double *wangle)
{    /* FIXME, this can be optimized a lot */
    double speed=0;
    double t_heading=0;
    double t=0;
    double imax = 90;
    double t_max = -100;
    double t_max2 = -100;
    int i=0;

    /* -90 to +90 form desired diretion */
    for (i=0; i<imax; i++)
    {
        t_heading = w_angle + degToRad(((double)i));
        speed = this->getSpeed(w_speed, A180(radToDeg(t_heading)));

        if (speed < 0.0)
        {
            continue;
        }
        t = speed * cos(degToRad(((double)i)));
        if (t > t_max)
        {
            t_max = t;
            *wangle = t_heading;
        } else if ( t_max - t > (t_max/20.0))
        {
            break;  /* cut if lower enough from current maximum */
        }
    }

    for (i=0; i<imax; i++)
    {
        t_heading = w_angle - degToRad(((double)i));
        speed = this->getSpeed(w_speed,  A180(radToDeg(t_heading)));
        if (speed < 0.0)
        {
          continue;
        }
        t = speed * cos(- degToRad(((double)i)));
        if (t > t_max2)
        {
          t_max2 = t;
          if (t > t_max)
          {
            t_max = t;
            *wangle = t_heading;
          }
        } else if (t_max2 - t > (t_max2/20.0))
        {
          break;
        }
    }
}

double Polar::A180(double angle)
{
    if(qAbs(angle)>180)
    {
        if(angle<0)
            angle=360+angle;
        else
            angle=angle-360;
    }
    return angle;
}
double Polar::A360(double hdg)
{
    if(hdg>=360) hdg=hdg-360;
    if(hdg<0) hdg=hdg+360;
    return hdg;
}

/********************/
/*  VLM polar List  */
/********************/
polarList::polarList(inetConnexion * inet, MainWindow * mainWindow) : inetClient(inet)
{
    polars.clear();
    loadList.clear();
    isLoading=false;
    this->mainWindow=mainWindow;
}

polarList::~polarList(void)
{
    while(polars.count())
        delete polars.first();
    polars.clear();
}

void polarList::getPolar(QString fname)
{
    //qWarning() << "get polar for " << fname;
    if(fname=="") {
        emit polarLoaded(fname,NULL);
        return ;
    }
    loadList.append(fname);
    if(!isLoading)
    {
        //qWarning() << "Starting loop";
        loadPolars();
    }
}


void polarList::loadPolars(void)
{
    isLoading=true;
    while(loadList.count()!=0)
    {
        QString fname=loadList.takeFirst();
        //qWarning() << "loop on " << fname;
        Polar * res=NULL;
        QListIterator<Polar*> i (polars);

        while(i.hasNext())
        {
            Polar * item = i.next();
            if(item->getName()==fname)
            {
                //qWarning() << "polar already loaded";
                res=item;
                item->nbUsed++;
                break;
            }
        }
        if(!res)
        {
            //qWarning() << "loading polar from disk";
            res = new Polar(fname,mainWindow);
            if(res && res->isLoaded())
            {
                res->nbUsed++;
                polars.append(res);
            }
            else
                res=NULL;
        }

        if(res==NULL)
            qWarning() << "Polar not found";
        //qWarning() << "Sending polar " << fname;
        emit polarLoaded(fname,res);
    }
    isLoading=false;
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

void polarList::get_polarList(void)
{
    if(!hasInet() || hasRequest() )
    {
        qWarning() << "getPolarList: bad state";
        return;
    }

    QString page;
    QTextStream(&page) << "/ws/polarlist.php";

    inetGet(0,page);

}

void polarList::requestFinished(QByteArray res)
{
    QJson::Parser parser;
    bool ok;

    QList<QVariant> result = parser.parse (res, &ok).toList();
    if (!ok) {
        qWarning() << "Error parsing json data " << res;
        qWarning() << "Error: " << parser.errorString() << " (line: " << parser.errorLine() << ")";
    }

    qWarning() << "Polar list: " ;

    foreach (QVariant polar, result)
    {
        qWarning() << "\t-" << polar.toString();
    }

}
