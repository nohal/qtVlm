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

#ifndef ANGLEUTIL_H
#define ANGLEUTIL_H

#include <QtGlobal>

class AngleUtil
{
    public:
        static double A360(const double &hdg);
        static double A180(double angle);
        static double myDiffAngle(const double &a1, const double &a2);
};

inline double AngleUtil::A360(const double &hdg)
{
    double newhdg=hdg;
    while (newhdg>=360.0) newhdg-=360.0;
    while (newhdg<0.0) newhdg+=360.0;
    return newhdg;
}

inline double AngleUtil::A180(double angle)
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

inline double AngleUtil::myDiffAngle(const double &a1,const double &a2)
{
    return qAbs(AngleUtil::A360(qAbs(a1)+ 180.0 -qAbs(a2)) -180.0);
}

#endif // ANGLEUTIL_H
