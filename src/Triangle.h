// **********************************************************
//   File: Triangle.h
//   Description: Definition of class Triangle
//
//   Author: Carlos Moreno
// **********************************************************

#ifndef __TRIANGLE_H__
#define __TRIANGLE_H__

#include "Point.h"
#include <math.h>

const int left_turn = +1;
const int right_turn = -1;
const int collinear = 0;

const int counter_clockwise = left_turn;
const int clockwise = right_turn;


class Triangle  
{
public:

        //Constructor
    explicit Triangle (const Point & p1 = origin, 
                       const Point & p2 = origin, 
                       const Point & p3 = origin)
        : v1 (p1), v2 (p2), v3 (p3) {}

        // Geometric and miscelaneous operations
    void draw () const;

    bool contains (const Point &) const;
    double area () const;
    int orientation () const;

        // Comparison operators
    bool operator== (const Triangle & t) const;
    bool operator!= (const Triangle & t) const;

    bool operator> (const Triangle &) const;
    bool operator< (const Triangle &) const;

private:
    Point v1, v2, v3;

    double signed_area () const;
};


inline int turn (const Point & p1, const Point & p2, const Point & p3)
{
    return Triangle(p1,p2,p3).orientation();
}

#endif
