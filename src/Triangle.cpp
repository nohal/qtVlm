// **********************************************************
//   File: Triangle.cpp
//   Description: Implementation of class Triangle
//
//   Author: Carlos Moreno
// **********************************************************

#include "Triangle.h"
#include "Segment.h"

#include <math.h>


double Triangle::signed_area () const
{
    return (v1.get_x() * (v2.get_y() - v3.get_y()) +
            v2.get_x() * (v3.get_y() - v1.get_y()) +
            v3.get_x() * (v1.get_y() - v2.get_y()));
}


bool Triangle::contains (const Point & p) const
{
    return turn (v1,v2,p) == turn (v2,v3,p) &&
           turn (v2,v3,p) == turn (v3,v1,p);
}


//void Triangle::draw () const
//{
//    Segment (v1,v2).draw();
//    Segment (v2,v3).draw();
//    Segment (v1,v3).draw();
//}


double Triangle::area () const
{
    return fabs (signed_area());
}


int Triangle::orientation () const
{
    double s_a = signed_area();

    return s_a > 0 ? 1 : (s_a < 0 ? -1 : 0);
}


bool Triangle::operator== (const Triangle & t) const
{
    return v1 == t.v1 && v2 == t.v2 && v3 == t.v3;
}


bool Triangle::operator!= (const Triangle & t) const
{
    return !(*this == t);
}
