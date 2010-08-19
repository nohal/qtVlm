// **********************************************************
//   File: Segment.cpp
//   Description: Implementation of class Segment
//
//   Author: Carlos Moreno
// **********************************************************

#include "Segment.h"
#include "Triangle.h"   // orientation function
//#include "graphic_interface.h"


bool Segment::intersects (const Segment & s) const
{
    return turn (p1, p2, s.p1) != turn (p1, p2, s.p2) &&
           turn (s.p1, s.p2, p1) != turn (s.p1, s.p2, p2);
}


bool Segment::contains (const Point & p) const
{
    return turn (p1, p2, p) == collinear &&
           ((p-p1) * (p-p2) < 0);

        // If the scalar product is negative, it means 
        // p1-p and p2-p are in opposite senses;  therefore, 
        // p is between p1 and p2
}


//void Segment::draw () const
//{
//    draw_line (p1.get_x(), p1.get_y(), p2.get_x(), p2.get_y());
//}


bool Segment::operator== (const Segment & s) const
{
    return p1 == s.p1 && p2 == s.p2;
}


bool Segment::operator!= (const Segment & s) const
{
    return !(*this == s);
}
