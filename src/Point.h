// **********************************************************
//   File: Point.h
//   Description: Definition of class Point
//
//   Author: Carlos Moreno
// **********************************************************

#ifndef __POINT_H__
#define __POINT_H__
#include<qglobal.h>
class Segment;
class Triangle;
class PolyGon;

class Point 
{
public:

        // Constructor
    explicit Point (double _x = 0, double _y = 0)
        : x(_x), y(_y) {}

        // Geometric and miscelaneous operations
    void draw (int radius = 0) const;
        // Platform dependent - implemented for Win32

    bool is_on (const Segment &) const;

    bool is_inside (const Triangle &) const;
    bool is_inside (const PolyGon &) const;

        // Arithmetic operators
    Point operator+ (const Point & p) const;
    Point operator- (const Point & p) const;
    Point operator* (double r) const;
    double operator* (const Point & p) const;  // Scalar product
    Point operator/ (double r) const;

        // Comparison operators
    bool operator== (const Point & p) const;
    bool operator!= (const Point & p) const;

    bool operator< (const Point & p) const;
    bool operator> (const Point & p) const;

        // get utility functions
    double get_x () const
    {
        return x;
    }

    double get_y () const
    {
        return y;
    }

private:
    double x,y;
};
Q_DECLARE_TYPEINFO(Point,Q_MOVABLE_TYPE);


inline Point operator* (double x, const Point & p)
{
    return p*x;     // Invoke member-function
}


const Point origin (0,0);

#endif
