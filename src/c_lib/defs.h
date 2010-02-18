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

This code is mainly coming from vlmc module by vlm: www.virtual-loup-de-mer.org

***********************************************************************/

#ifndef __CLIB_DEFS_H
#define __CLIB_DEFS_H

#define degToRad(angle) (((angle)/180.0) * PI)
#define radToDeg(angle) (((angle)*180.0) / PI)

#define msToKts(speed) (1.9438445*(speed))
#define ktsToMs(speed) (0.51444444*(speed))

#define PI     M_PI
#define PI_2   M_PI_2
#define PI_4   M_PI_4
#define TWO_PI M_PI * 2

#endif
