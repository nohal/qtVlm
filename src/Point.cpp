// **********************************************************
//   File: Point.cpp
//   Description: Implementation of class Point
//
//   Author: Carlos Moreno
// **********************************************************

#include "Point.h"
#include "Segment.h"
#include "Triangle.h"
#include "Polygon.h"

//#include "graphic_interface.h"


bool Point::is_on (const Segment & s) const
{
    return s.contains (*this);
}


bool Point::is_inside (const Triangle & t) const
{
    return t.contains (*this);
}


bool Point::is_inside (const PolyGon & P) const
{
    return P.contains (*this);
}


//void Point::draw (int radius) const
//{
//    draw_point (x, y, radius);
//}


Point Point::operator+ (const Point & p) const
{
    return Point (x + p.x, y + p.y);
}


Point Point::operator- (const Point & p) const
{
    return Point (x - p.x, y - p.y);
}


Point Point::operator* (double r) const
{
    return Point (r*x, r*y);
}


double Point::operator* (const Point & p) const
{
    return x * p.x + y * p.y;       // Scalar product
}


Point Point::operator/ (double r) const
{
    return Point (x/r, y/r);
}


bool Point::operator== (const Point & p) const
{
    return x == p.x && y == p.y;
}


bool Point::operator!= (const Point & p) const
{
    return !(*this == p);
}
