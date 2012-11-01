// **********************************************************
//   File: Polygon.cpp
//   Description: Implementation of class Polygon
//
//   Author: Carlos Moreno
// **********************************************************

#include "Polygon.h"
#include "Segment.h"
#include "Triangle.h"


PolyGon::iterator PolyGon::begin()
{
    return iterator (vertices.begin(), vertices.end());
}

PolyGon::const_iterator PolyGon::begin() const
{
    return const_iterator (vertices.begin(), vertices.end());
}

void PolyGon::push_back (const Point & p)
{
    vertices.push_back (p);
}


void PolyGon::push_front (const Point & p)
{
    vertices.push_front (p);
}


void PolyGon::insert (const iterator & i, const Point & p)
{
    vertices.insert (i.get_i(), p);
}


PolyGon::iterator PolyGon::remove (const iterator & target)
{
    if (!(vertices.empty()))
    {
        list<Point>::iterator ret_value;

        ret_value = vertices.erase (target.get_i());
        if (ret_value == vertices.end())
        {
            return iterator (vertices.begin(), vertices.end());
        }
        else
        {
            return iterator (ret_value, vertices.end());
        }
    }
    else
    {
        return iterator();      // return "null" iterator
    }
}


bool PolyGon::contains (const Point & p) const
{
        // Find a point outside the PolyGon
        //(point with max x + (1,0))

    const_iterator current = begin();
    Point p_max_x = *current++;

    do
    {
        if ((*current).get_x() > p_max_x.get_x())
        {
            p_max_x = *current;
        }
        ++current;
    }
    while (current != begin());

    Point outside = p_max_x + Point (1,0);
    Segment line (p, outside);

        // Now check the number of intersections of segment
        // p - outside with the edges of the PolyGon

        // The loop must start in a vertex that is not exactly
        // on the segment 'line'.

    int intersections = 0;

    current = begin();

    while ((*current).is_on (line))  ++current;

    PolyGon::const_iterator first_point = current;

    do
    {
        Point v = *current++;
        if (Segment (v, *current).intersects (line))
        {
            intersections++;
        }

        if ((*current).is_on (line))
        {
            while ((*++current).is_on (line));
                // Skip all the vertices that are on the line

            if (turn(line.get_p1(), line.get_p2(), v) ==
                turn (line.get_p1(), line.get_p2(), *current))
            {
                intersections--;
            }
                // If previous and next points are on the same
                // side of line, then line doesn't intersect the
                // PolyGon;  it merely touches it at the vertex.
                // Since one intersection was counted by the test
                // with the first edge, decrement the count.
        }
    }
    while (current != first_point);

    if (intersections % 2)  // if odd number of intersections
    {
        return true;
    }
    else
    {
        return false;
    }
}


//void PolyGon::draw (bool draw_vs, int v_radius) const
//{
//    const_iterator current = begin();
//
//    if (draw_vs)
//    {
//        (*current).draw (2 * v_radius);
//    }
//    do
//    {
//        if (draw_vs)
//        {
//            (*current).draw (v_radius);
//        }
//        Point p = *current++;
//        Segment (p, *current).draw();
//    }
//    while (current != begin());
//}
//

PolyGon PolyGon::convex_hull() const
{
    PolyGon hull;
    PolyGon::const_iterator input_iter = begin();
    PolyGon::iterator hull_iter;

        // The first three points go directly to the output

    Point p1 = *input_iter++;
    Point p2 = *input_iter++;
    Point p3 = *input_iter++;

        // Make sure the output is in counter-clockwise sense
    if (turn (p1, p2, p3) == left_turn)
    {
        hull.push_back (p1);
        hull.push_back (p2);
        hull.push_back (p3);
        hull.push_front (p3);
    }
    else
    {
        hull.push_back (p2);
        hull.push_back (p1);
        hull.push_back (p3);
        hull.push_front (p3);
    }
        // The last point has to be always inserted at both ends

    do
    {
        PolyGon::iterator first = hull.begin();
        PolyGon::iterator last = first - 1;
            // The list is circular

        Point p = *input_iter++;

        if (turn (*(first+1), *first, p) == left_turn ||
            turn (*(last-1), *last, p) == right_turn)
        {
            hull.push_back (p);
            hull.push_front (p);

                // Now fix the possible concavities after insertion
                // of new point (on both ends of the output list)

            first = hull.begin();
            last = first-1;

            while (turn (*first, *(first+1), *(first+2)) == right_turn)
            {
                hull.remove (first+1);
            }

            while (turn (*last, *(last-1), *(last-2)) == left_turn)
            {
                hull.remove (last-1);
            }
        }
    }
    while (input_iter != begin());

    hull.remove (hull.begin());     // Remove repeated point  (1st pt. = last pt.)

    return hull;
}


int PolyGon::orientation() const
{
    const_iterator current = begin();
    bool empty_triangle_found = false;
    Triangle t;

        // Find a triangle (three consecutive vertices of
        // the PolyGon) that doesn't contain any other
        // vertex of the PolyGon.

    do
    {
        t = Triangle (*(current-1), *current, *(current+1));
        bool t_is_empty = true;

        const_iterator i = begin();
        do
        {
            if ((*i).is_inside (t))
            {
                t_is_empty = false;
                break;
            }
            ++i;
        }
        while (i != begin());

        if (t_is_empty)
        {
            empty_triangle_found = true;
        }
        else
        {
            ++current;
        }
    }
    while (!empty_triangle_found);

        // Now we know that either all the points inside the
        // triangle are inside the PolyGon, are all are
        // outside the PolyGon.

        // Given the orientation of t, and checking if a
        // point inside t is also inside the PolyGon, we
        // determine the orientation of the PolyGon.

    Point p = (*(current-1) + *current + *(current+1)) / 3;

    if (p.is_inside (*this))    // if inside the PolyGon
    {
        if (t.orientation () == counter_clockwise)
        {
            return counter_clockwise;
        }
        else
        {
            return clockwise;
        }
    }
    else
    {
        if (t.orientation () == counter_clockwise)
        {
            return clockwise;
        }
        else
        {
            return counter_clockwise;
        }
    }
}
