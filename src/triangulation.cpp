// **********************************************************
//   File: triangulation.cpp
//   Description: Triangulation algorithm.  The function 
//                triangulate() is called by client code,
//                and the function recursive_triangulation()
//                is used by triangulate() to execute the 
//                recursive algorithm
//
//   Author: Carlos Moreno
// **********************************************************

#include "Polygon.h"
#include "Point.h"
#include "Triangle.h"

static void recursive_triangulation (PolyGon &, list<Triangle> &);


void triangulate (const PolyGon & input_P, list<Triangle> & output)
{
    PolyGon P = input_P;    // Make a working copy of parameter

    output.clear ();        // Only the first time.  Recursive 
                            // calls should not clear the list 
                            // of triangles
    
    recursive_triangulation (P, output);
}


static void recursive_triangulation (PolyGon & P, list<Triangle> & output)
{
    PolyGon::iterator current = P.begin();

    
    if ((current+3) == current)   // If PolyGon contains 3 vertices
    {
        output.push_back (Triangle (*current, *(current+1), *(current+2)));
        return;
    }

    //bool done = false;
    do
    {
        Triangle t (*(current-1), *current, *(current+1));
        if (t.orientation() == counter_clockwise)   // if convex vertex
        {
            bool empty_triangle = true;
            PolyGon::const_iterator i = current+2,
                                    farthest_point = current-1;
            do
            {

                if (t.contains (*i))
                {
                    empty_triangle = false;
                    if (Triangle(*(current+1), *(current-1), *i).area() > 
                        Triangle(*(current+1), *(current-1), *farthest_point).area())
                    {
                        farthest_point = i;
                    }
                }
                i++;
            }
            while (i != current-1);

            if (empty_triangle)     // send it to the triangulation
            {
                output.push_back (t);
                current = P.remove (current);
                current--;
                if ((current+3) == current)     // If P became a triangle, done
                {
                    output.push_back (Triangle (*current, *(current+1), *(current+2)));
                    return;
                }
            }
            else    // t not empty ==> split the PolyGon and triangulate both halves
            {
                PolyGon first_half, second_half;

                for (i = current; i != farthest_point; i++)
                {
                    first_half.push_back (*i);
                }
                first_half.push_back (*farthest_point);

                recursive_triangulation (first_half, output);

                for (i = farthest_point; i != current; i++)
                {
                    second_half.push_back (*i);
                }
                second_half.push_back (*current);

                recursive_triangulation (second_half, output);

                return;
            }
        }
        current++;
    }
    while (true);
}
