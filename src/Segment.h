// **********************************************************
//   File: Segment.h
//   Description: Definition of class Segment
//
//   Author: Carlos Moreno
// **********************************************************

#ifndef __SEGMENT_H__
#define __SEGMENT_H__

#include "Point.h"


class Segment
{
public:

        // Constructors
    explicit Segment (double x1 = 0, double y1 = 0, 
                      double x2 = 0, double y2 = 0)
        : p1 (x1, y1), p2 (x2, y2) {}

    explicit Segment (const Point & _p1, const Point & _p2)
        : p1 (_p1), p2 (_p2) {}

        // Geometric and miscelaneous operations
    void draw () const;
            // Platform dependent - Implemented for Win32

    bool intersects (const Segment &) const;
    bool contains (const Point &) const;
            // returns true if point is on the segment

        // Comparison operators
    bool operator== (const Segment & s) const;
    bool operator!= (const Segment & s) const;

        // get utility functions
    const Point & get_p1 () const
    {
        return p1;
    }

    const Point & get_p2 () const
    {
        return p2;
    }

private:
    Point p1, p2;
};
Q_DECLARE_TYPEINFO(Segment,Q_MOVABLE_TYPE);

#endif
