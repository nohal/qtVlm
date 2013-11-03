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

#include <QDebug>
#include <QDateTime>

#include "GribRecord.h"

GribRecord::GribRecord(void) {
    ok=false;
    /* init time/date data */
    refYear=0;
    refMonth=0;
    refDay=0;
    refHour=0;
    refMinute=0;
    refSecond=0;
    deltaPeriod=0;
    data=NULL;
}
GribRecord::~GribRecord()
{    
    if(data)
        delete[] data;
}

void  GribRecord::set_dataType(int t) {
    if(t!=DATA_NOTDEF)
        dataType = t;
    dataKey = makeKey(dataType, levelType, levelValue);
}

time_t GribRecord::makeDate(unsigned int year,unsigned int month,unsigned int day,
                            unsigned int hour,unsigned int min,unsigned int sec)
{
    QDateTime dt = QDateTime(QDate(year,month,day),QTime(hour,0,0),Qt::UTC);
    dt = dt.addSecs(min*60+sec);
    return dt.toTime_t();
}

void GribRecord::multiplyAllData(double k) {
    for (unsigned int j=0; j<Nj; j++)
        for (unsigned int i=0; i<Ni; i++)
            if (hasValue(i,j))
                data[j*Ni+i] *= k;
}

void GribRecord::unitConversion(void) {
    switch(dataType) {
        case DATA_PRECIP_RATE: // mm/s -> mm/h
            multiplyAllData(3600);
            break;
        case DATA_PRECIP_TOT: // mm/period -> mm/h
            if(deltaPeriod!=0)
                multiplyAllData(1.0/(double)deltaPeriod);
            break;
        case DATA_WIND_VX: // msToKts
        case DATA_WIND_VY:
        case DATA_CURRENT_VX:
        case DATA_CURRENT_VY:
            multiplyAllData(msToKts_cst);
            break;

    }
    if(dataType==DATA_CURRENT_VX || dataType==DATA_CURRENT_VY)
    {
        //qWarning()<<dataType<<levelType<<levelValue;
        levelType=DATA_LV_MSL;
        levelValue=0;
    }
}

//===============================================================================================
bool GribRecord::getValue_TWSA(double px, double py,double * a00,double * a01,double * a10,double * a11,bool debug) {
    int sigDj;

    if (!ok || Di==0 || Dj==0)
        return false;

    if(!a00 || !a01 || !a10 || !a11)
        return false;

    if (!isPointInMap(px,py)) {
        px += 360.0;               // tour du monde a droite ?
        if (!isPointInMap(px,py)) {
            px -= 2*360.0;              // tour du monde a gauche ?
            if (!isPointInMap(px,py))
                return false;
        }
    }

    // 00 10      point is in a square
    // 01 11
    /*note that (int) truncates, for instance (int) -3.5 returns 3, while floor(-3.5) returns -4*/
    int i0 = (int) floor((px-Lo1)/Di);  // point 00
    int j0 = (int) floor((py-La1)/Dj);
    int j0_init=j0;
    int i1;

    if(isFull && px>=Lo2)
        i1=0;
    else
        i1=i0+1;

    if(((py-La1)/Dj)-j0==0.0) {
        if(debug)
            qWarning() << "on grib point";
        sigDj=0;
    }
    else {
        if(Dj<0) {
            sigDj=-1;
            j0++;
        }
        else
            sigDj=1;
    }

    if(debug) {
        qWarning() << "Lo1=" << Lo1 << ", La1=" << La1 << ", Di=" << Di << ", Dj" << Dj;
        qWarning() << "Rec date = " << this->get_curDate();
        qWarning() << "px=" << px << ", py=" << py << ", i0=" << i0 << ", j0=" << j0 << ", i1=" << i1 << ", j1=" << j0+sigDj;
    }

    int nbval = 0;     // how many values in grid ?
    if (hasValue(i0,   j0))
        nbval ++;
    if (hasValue(i1, j0))
        nbval ++;
    if (hasValue(i0,   j0+sigDj))
        nbval ++;
    if (hasValue(i1, j0+sigDj))
        nbval ++;

    if(debug)
        qWarning() << "Nb Val =" << nbval;

    if (nbval != 4)
        return false;

    *a00 = getValue(i0,   j0);
    *a01 = getValue(i0,   j0+sigDj);
    *a10 = getValue(i1, j0);
    *a11 = getValue(i1, j0+sigDj);

    if(debug) {
        qWarning() << "val around 00\n" << *a00
                   << "\n" << getValue(i0-1,   j0_init)
                   << "\n" << getValue(i0-1,   j0_init+1)
                   << "\n" << getValue(i0,   j0_init+1)
                   << "\n" << getValue(i0+1,   j0_init+1)
                   << "\n" << getValue(i0+1,   j0_init)
                   << "\n" << getValue(i0+1,   j0_init-1)
                   << "\n" << getValue(i0,   j0_init-1)
                   << "\n" << getValue(i0-1,   j0_init-1) << "\n";
    }

    return true;
}

//===============================================================================================
// Interpolation pour donnees 1D
//

double GribRecord::getInterpolatedValue(double px, double py, bool numericalInterpolation) {
    double val;
    if (!ok || Di==0 || Dj==0) {
        return GRIB_NOTDEF;
    }
    if (!isPointInMap(px,py)) {
        px += 360.0;               // tour du monde a droite ?
        if (!isPointInMap(px,py)) {
            px -= 2*360.0;              // tour du monde a gauche ?
            if (!isPointInMap(px,py)) {
                return GRIB_NOTDEF;
            }
        }
    }

    int i0 = (int) floor((px-Lo1)/Di);  // point 00
    int j0 = (int) floor((py-La1)/Dj);

    double pi, pj;     // coord. in grid unit
    pi = (px-Lo1)/Di;
    pj = (py-La1)/Dj;

    int i1;
    int sigDj=1;

    if(isFull && px>=Lo2)
        i1=0;
    else
        i1=i0+1;

    // distances to 00
    double dx = pi-i0;
    double dy = pj-j0;

    if(((py-La1)/Dj)-j0==0.0)
        sigDj=0;

    bool h00,h01,h10,h11;
    int nbval = 0;     // how many values in grid ?
    if ((h00=hasValue(i0,   j0)))
        nbval ++;
    if ((h10=hasValue(i1, j0)))
        nbval ++;
    if ((h01=hasValue(i0,   j0+sigDj)))
        nbval ++;
    if ((h11=hasValue(i1, j0+sigDj)))
        nbval ++;

    if (nbval <3) {
        return GRIB_NOTDEF;
    }

    if (! numericalInterpolation) {
        if (dx < 0.5) {
            if (dy < 0.5)
                val = getValue(i0,   j0);
            else
                val = getValue(i0,   j0+sigDj);
        }
        else {
            if (dy < 0.5)
                val = getValue(i1,   j0);
            else
                val = getValue(i1,   j0+sigDj);
        }
        return val;
    }

    dx = (3.0 - 2.0*dx)*dx*dx;   // pseudo hermite interpolation
    dy = (3.0 - 2.0*dy)*dy*dy;

    double xa, xb, xc, kx, ky;
    // Triangle :
    //   xa  xb
    //   xc
    // kx = distance(xa,x)
    // ky = distance(xa,y)
    if (nbval == 4) {
        double x00 = getValue(i0,   j0);
        double x01 = getValue(i0,   j0+sigDj);
        double x10 = getValue(i1, j0);
        double x11 = getValue(i1, j0+sigDj);
        double x1 = (1.0-dx)*x00 + dx*x10;
        double x2 = (1.0-dx)*x01 + dx*x11;
        val =  (1.0-dy)*x1 + dy*x2;
        return val;
    }
    else {
        // here nbval==3, check the corner without data
        if (!h00) {
            //printf("! h00  %f %f\n", dx,dy);
            xa = getValue(i1, j0+sigDj);   // A = point 11
            xb = getValue(i0,   j0+sigDj);   // B = point 01
            xc = getValue(i1, j0);     // C = point 10
            kx = 1-dx;
            ky = 1-dy;
        }
        else if (!h01) {
            //printf("! h01  %f %f\n", dx,dy);
            xa = getValue(i1, j0);     // A = point 10
            xb = getValue(i1, j0+sigDj);   // B = point 11
            xc = getValue(i0,   j0);     // C = point 00
            kx = dy;
            ky = 1-dx;
        }
        else if (!h10) {
            //printf("! h10  %f %f\n", dx,dy);
            xa = getValue(i0,   j0+sigDj);     // A = point 01
            xb = getValue(i0,   j0);       // B = point 00
            xc = getValue(i1, j0+sigDj);     // C = point 11
            kx = 1-dy;
            ky = dx;
        }
        else {
            //printf("! h11  %f %f\n", dx,dy);
            xa = getValue(i0,   j0);    // A = point 00
            xb = getValue(i1, j0);    // B = point 10
            xc = getValue(i0,   j0+sigDj);  // C = point 01
            kx = dx;
            ky = dy;
        }
    }
    double k = kx + ky;
    if (k<0 || k>1) {
        val = GRIB_NOTDEF;
    }
    else if (k == 0) {
        val = xa;
    }
    else {
        // axes interpolation
        double vx = k*xb + (1-k)*xa;
        double vy = k*xc + (1-k)*xa;
        // diagonal interpolation
        double k2 = kx / k;
        val =  k2*vx + (1-k2)*vy;
    }
    return val;
}

void GribRecord::print_bitmap(void) {
    QString line;
    for(unsigned int j=0; j<Nj;++j) {
        line = "";
        for(unsigned int i=0;i<Ni;++i)
            line+=hasValue(i,j)?"1":"0";
        qWarning() << line;
    }
}
