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
#include <QDomDocument>

#include "MainWindow.h"
#include "Polar.h"
#include "dataDef.h"
#include "parser.h"
#include "Orthodromie.h"
#include "settings.h"
#include "boat.h"
#include "AngleUtil.h"

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
    fileType=POLAR_NONE;
    loaded=false;
    clearPolar();
    if(this->mainWindow->getSelectedBoat() && this->mainWindow->getSelectedBoat()->get_boatType()==BOAT_REAL)
        coeffPolar=Settings::getSetting(polar_efficiency).toInt()/100.0;
    else
        coeffPolar=1.0;

    //qWarning() << "Opening polar" << fname<<"with coeff"<<coeffPolar << " appfolder: " << appFolder.value("polar");

    name=fname;
    QString nameF;
    QFile file;
    if (fname.endsWith(".csv",Qt::CaseInsensitive) || fname.endsWith(".pol",Qt::CaseInsensitive) || fname.endsWith(".xml",Qt::CaseInsensitive))
    {
        nameF=appFolder.value("polar")+fname;
        file.setFileName(nameF);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text ))
        {
            QMessageBox::warning(0,QObject::tr("Lecture de polaire"),
                QString(QObject::tr("Impossible d'ouvrir le fichier %1")).arg(name));
            return;
        }

        if(fname.endsWith("csv",Qt::CaseInsensitive))
            fileType=POLAR_CSV;
        else if(fname.endsWith("pol",Qt::CaseInsensitive))
            fileType=POLAR_POL;
        else if(fname.endsWith("xml",Qt::CaseInsensitive))
            fileType=POLAR_XML;
    }
    else
    {
        //qWarning() << "Base dir " << appFolder.value("polar");
        nameF=appFolder.value("polar")+fname+".csv";
        file.setFileName(nameF);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text ))
        {
            nameF = appFolder.value("polar")+fname+".pol";
            file.setFileName(nameF);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text ))
            {
                nameF = appFolder.value("polar")+fname+".xml";
                file.setFileName(nameF);
                if (!file.open(QIODevice::ReadOnly | QIODevice::Text ))
                {
                    QMessageBox::warning(0,QObject::tr("Lecture de polaire"),
                        QString(QObject::tr("Impossible d'ouvrir le fichier %1 (ni en .csv ni en .pol)")).arg(name));
                    return;
                }
                else
                    fileType=POLAR_XML;
            }
            else
                fileType=POLAR_POL;
        }
        else
            fileType=POLAR_CSV;
    }

    switch(fileType) {
        case POLAR_CSV:
        case POLAR_POL:
            loadPolar_csvPol(&file,fileType,nameF);
            file.close();
            break;
        case POLAR_XML:
            loadPolar_xml(&file,fileType,nameF);
            file.close();
            break;
        default:
            return;
    }

#if 0
    qWarning()<<"polar data for"<<nameF;
    QString debug="xxx.x ";
    foreach(double dd,tws)
        debug+=QString().sprintf("%04.1f ",dd);
    qWarning()<<debug;
    QListIterator<double> i1(polar_data);
    QListIterator<double> i2(twa);
    while(i2.hasNext())
    {
        debug=QString().sprintf("%05.1f ",i2.next());
        for(int nn=0;nn<tws.count();++nn)
            debug+=QString().sprintf("%04.1f ",i1.next());
        qWarning()<<debug;
    }
#endif

    mid_twa=qRound(twa.count()/2.0);
    mid_tws=qRound(tws.count()/2.0);
    /* polaire chargee */

/* pre-calculate B-VMG for every tws at 0.1 precision with a twa step of 1 and then .1 */

    double ws=0.0;
    double wa=0.0;
    double bvmg,bvmg_d,bvmg_u,wa_u,wa_d,wa_limit;
    maxSpeed=-1.0;
    do
    {
        wa_u=0.0;
        wa_d=180.0;
        bvmg_u=bvmg_d=0;
        double speed;
        do
        {
            speed=myGetSpeed(ws,wa,true);
            if(speed>maxSpeed)
            {
                maxSpeed=speed;
                maxSpeedTwa=wa;
                maxSpeedTws=ws;
            }
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
        if(wa<0) wa=0.0;
        if(wa_limit<1) wa_limit=1.0;
        if(wa_limit>180) wa_limit=180.0;
        bvmg_u=0.0;
        do
        {
            speed=myGetSpeed(ws,wa,true);
            if(speed>maxSpeed)
            {
                maxSpeed=speed;
                maxSpeedTwa=wa;
                maxSpeedTws=ws;
            }
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
        if(wa<0) wa=0.0;
        if(wa_limit<1) wa_limit=1.0;
        if(wa_limit>180) wa_limit=180.0;
        bvmg_d=0.0;
        do
        {
            speed=myGetSpeed(ws,wa,true);
            if(speed>maxSpeed)
            {
                maxSpeed=speed;
                maxSpeedTwa=wa;
                maxSpeedTws=ws;
            }
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
        wa=0.0;
        ws=ws+.1;
    }while(ws<60.1);

    loaded=true;
    QFileInfo fi(file.fileName());
    QString nameFVmg = appFolder.value("polar")+fi.baseName()+".vmg";
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
            ssVmg.sprintf("%.4d",qRound(AngleUtil::A360(vmg)*10.0));
            sVmg<<ssVmg;
        }
    }
    fileVMG.close();
    if(!fileVMG.open(QIODevice::ReadOnly | QIODevice::Text ))
        qWarning()<<"fileVMG could not be re-opened!";
    progress->close();
    delete progress;
}
void Polar::getMaxSpeedData(double *bs, double *tws, double *twa) const
{
    *bs=maxSpeed;
    *tws=maxSpeedTws;
    *twa=maxSpeedTwa;
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

void Polar::loadPolar_csvPol(QFile *file,int fileType,QString fname) {
    QTextStream stream(file);
    QString line;
    QStringList list;
    /* read first line to see line length */
    line=stream.readLine();
    line.remove("\"");
    if(line.isNull())
    {
        QMessageBox::warning(0,QObject::tr("Lecture de polaire"),
             QString(QObject::tr("Fichier %1 vide")).arg(fname));
        return;
    }
    if(fileType==POLAR_CSV)
        list = line.split(';');
    else if(fileType==POLAR_POL)
        list = line.split('\t');
    else
        return;

    if(list[0].toUpper() != "TWA\\TWS" && list[0].toUpper() != "TWA/TWS" && list[0].toUpper() != "TWA")
    {
        QMessageBox::warning(0,QObject::tr("Lecture de polaire"),
             QString(QObject::tr("Fichier %1 invalide (doit commencer par TWA\\TWS et non '%2')"))
                        .arg(fname)
                        .arg(list[0]));
        return;
    }
    int i;
    for(i=1;i<list.count();++i) // saving first line = all TWS data
    {
        if(!tws.isEmpty() && list[i].toDouble()<=tws.last()) break;
        tws.append(list[i].toDouble());
    }
    bool missingTws0=false;
    if(tws.first()!=0.0)
    { // is first TWS set to 0
        missingTws0=true;
    }
    bool firstTWA=true;
    while(true)
    {
        line=stream.readLine(); // read a line
        if(line.isNull()) break;
        line.remove("\"");
        /* split according to file type */
        if (fileType==POLAR_CSV)
            list = line.split(";");
        else
            list = line.split("\t");
        if(firstTWA) // this is the first twa line => if not for twa=0, add a dummy line filled with 0
        {
            firstTWA=false;
            if(list.first().toDouble()!=0.0)
            {
                for(int t=0;t<tws.count();++t)
                {
                    polar_data.append(0);
                }
                if(missingTws0) // add a 0 for tws=0 if missing in tws
                    polar_data.append(0);
                twa.append(0.0);
            }
        }
        twa.append(list[0].toDouble()); // save TWA value
        if(missingTws0)
            polar_data.append(0); // add 0 if TWS=0 is missing in tws line
        for(i=1;i<list.count();++i)
        {
            if(i>tws.count()) break; // safe test in case we have more data that the number of tws in the first line
            polar_data.append(list[i].toDouble()*this->coeffPolar); // use coeffPolar param (qtvlm setting)
        }
        while(i<=tws.count()) // if missing data at the end, complete with 0
            polar_data.append(0);
    }
    if(missingTws0)
        tws.prepend(0.0);
}

void Polar::loadPolar_xml(QFile * file,int /*fileType*/,QString /*fname*/) {
    QString  errorStr;
    int errorLine;
    int errorColumn;
    QDomDocument doc;
    if(!doc.setContent(file,true,&errorStr,&errorLine,&errorColumn))
    {
        QMessageBox::warning(0,QObject::tr("Lecture de polaire ("),
                             QString("Erreur ligne %1, colonne %2:\n%3")
                             .arg(errorLine)
                             .arg(errorColumn)
                             .arg(errorStr));
        return ;
    }

    QDomElement root = doc.documentElement();

    //qWarning() << "Root: " << root.tagName();
    qWarning() << "Polar Name: " << root.firstChild().toText().data().simplified();

    QDomNode subNode= root.firstChild().nextSibling();
    // first find polar dim + TWS and TWA value

    twa.clear();
    tws.clear();

    bool twaRead=false;

    while(!subNode.isNull())
    {
        if(subNode.toElement().tagName() == "PolarCurve") {
            QDomNode polarCurve=subNode.firstChild();
            bool hasCurveIndex=false;
            while(!polarCurve.isNull())
            {
                if(!hasCurveIndex && polarCurve.toElement().tagName()=="PolarCurveIndex") {
                    tws.append(polarCurve.toElement().attribute("value").toInt(&hasCurveIndex));
                }

                if(twaRead)
                    break;

                if(hasCurveIndex && polarCurve.toElement().tagName()=="PolarItem") {
                    QDomNode polarItem=polarCurve.firstChild();

                    while(!polarItem.isNull()) {
                        if(polarItem.toElement().tagName()=="Angle") {
                            twa.append(polarItem.toElement().attribute("value").toInt());
                        }

                        polarItem=polarItem.nextSibling();
                    }
                }
                polarCurve = polarCurve.nextSibling();
            }
            if(hasCurveIndex)
                twaRead=true;
        }
        subNode = subNode.nextSibling();
    }

    //qWarning() << "Found " << tws.count() << " tws, " << twa.count() << " twa";

    if(tws.count()==0 || twa.count()==0)
        return;

    double* ary = new double[tws.count()*twa.count()];

    // now read polar data

    subNode= root.firstChild().nextSibling();

    while(!subNode.isNull())
    {
        if(subNode.toElement().tagName() == "PolarCurve") {
            QDomNode polarCurve=subNode.firstChild();
            int idx_tws=0;
            bool hasCurveIndex=false;
            while(!polarCurve.isNull())
            {
                if(!hasCurveIndex && polarCurve.toElement().tagName()=="PolarCurveIndex") {
                    int curveIndex=polarCurve.toElement().attribute("value").toInt(&hasCurveIndex);
                    if(hasCurveIndex) {
                        idx_tws=tws.indexOf(curveIndex);
                        if(idx_tws==-1)
                            hasCurveIndex=false;
                    }
                }

                if(hasCurveIndex && polarCurve.toElement().tagName()=="PolarItem") {
                    QDomNode polarItem=polarCurve.firstChild();
                    int idx_twa;
                    bool hasAngle=true;
                    double val;
                    bool hasVal=false;
                    while(!polarItem.isNull()) {
                        if(polarItem.toElement().tagName()=="Angle") {
                            int angle=polarItem.toElement().attribute("value").toInt(&hasAngle);
                            if(hasAngle) {
                                idx_twa=twa.indexOf(angle);
                                if(idx_twa==-1)
                                    hasAngle=false;
                            }
                        }
                        if(polarItem.toElement().tagName()=="Value") {
                            val=polarItem.toElement().attribute("value").toDouble(&hasVal);
                        }
                        polarItem=polarItem.nextSibling();
                    }
                    if(hasAngle && hasVal) {
                        ary[idx_tws+idx_twa*tws.count()]=val;
                    }
                }
                polarCurve = polarCurve.nextSibling();
            }
        }
        subNode = subNode.nextSibling();
    }

    // Copy array to QList, adding needed 0 if TWA/TWS 0,0 absent



    polar_data.clear();
    bool hasZeroTwa=true;
    bool hasZeroTws=true;
    // first step : create TWS list + append 0 if needed

    if(tws.first()!=0) {
        hasZeroTws=false;
    }

    // second check if we have TWA=0 value
    if( twa.first() != 0) {
        hasZeroTwa=false;
        // create twa=0 line
        if(!hasZeroTws)
            polar_data.append(0);
        for(int i=0;i<tws.count();++i)
            polar_data.append(0);
    }

    for(int i=0;i<twa.count();++i) {
        if(!hasZeroTws)
            polar_data.append(0);
        for(int j=0;j<tws.count();++j)
            polar_data.append(ary[j+i*tws.count()]);
    }

    if(!hasZeroTws)
        tws.prepend(0);

    if(!hasZeroTwa)
        twa.prepend(0);

    delete[] ary;
/*
    QString str="   ";
    for(int i=0;i<tws.count();++i)
        str += QString().setNum(tws.at(i)) + " ";
    qWarning() << str;
    for(int i=0;i<twa.count(); ++i) {
        str= QString().setNum(twa.at(i));
        str +=" ";
        for(int j=0;j<tws.count();++j) {
            str += QString().setNum(polar_data[j+i*tws.count()])+" ";
        }
        qWarning() << str;

    }
    */
}

QString Polar::get_fileTypeStr(void) {
    switch(fileType) {
        case POLAR_CSV:
            return "csv";
        case POLAR_POL:
            return "pol";
        case POLAR_XML:
            return "xml";
        default:
            return "none";
    }

}

double Polar::getBvmgUp(double windSpeed, bool engine)
{
    if(!loaded)
        return 0;
    if(windSpeed<0) return 0;
    double angle=0;
    int val=qRound(windSpeed*10);
    if ( val>0 && val<= best_vmg_up.count()-1 )
        angle=best_vmg_up[qRound(windSpeed*10)];
    else if(val<0)
        angle=best_vmg_up.first();
    else
        angle=best_vmg_up.last();
    if(engine && this->mainWindow->getSelectedBoat() && this->mainWindow->getSelectedBoat()->getMinSpeedForEngine()>0)
    {
        double bs=this->myGetSpeed(windSpeed,angle,false);
        if(bs<this->mainWindow->getSelectedBoat()->getMinSpeedForEngine())
            angle=0;
    }
    return angle;
}

double Polar::getBvmgDown(double windSpeed, bool engine)
{
    if(!loaded)
        return 0;
    if(windSpeed<0) return 0;
    double angle=0;
    int val=qRound(windSpeed*10);
    if ( val > 0 && val <= best_vmg_down.count()-1 )
        angle=best_vmg_down[qRound(windSpeed*10)];
    else if(val <0)
        angle=best_vmg_up.first();
    else
        angle=best_vmg_down.last();
    if(engine && this->mainWindow->getSelectedBoat() && this->mainWindow->getSelectedBoat()->getMinSpeedForEngine()>0)
    {
        double bs=this->getSpeed(windSpeed,angle,false);
        if(bs<this->mainWindow->getSelectedBoat()->getMinSpeedForEngine())
            angle=180;
    }
    return angle;
}

double Polar::getSpeed(double windSpeed, double angle, bool engine,bool * engineUsed)
{
    if(windSpeed<0) return 0;
    double bs=myGetSpeed(windSpeed,angle,false);
    if(engineUsed!=NULL)
        *engineUsed=false;
    if(engine && mainWindow->getSelectedBoat() && bs<this->mainWindow->getSelectedBoat()->getMinSpeedForEngine())
    {
        bs=this->mainWindow->getSelectedBoat()->getSpeedWithEngine();
        if(engineUsed!=NULL)
            *engineUsed=true;
    }
    return bs;
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
/* see http://www.v-l-m.org/ or http://dev.v-l-m.org/vlm/  */
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
        speed = this->getSpeed(w_speed, AngleUtil::A180(radToDeg(t_heading)));

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
        speed = this->getSpeed(w_speed,  AngleUtil::A180(radToDeg(t_heading)));
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
            qWarning() << "Polar not found: " << fname;
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

    inetGet(0,page,false);

}

void polarList::requestFinished(QByteArray res)
{
    QList<QVariant> result;
    if (!inetClient::JSON_to_list(res,&result)) {
        return;
    }

    qWarning() << "Polar list: " ;

    foreach (QVariant polar, result)
    {
        qWarning() << "\t-" << polar.toString();
    }

}
