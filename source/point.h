#ifndef POINT_H_INCLUDED
#define POINT_H_INCLUDED
#include <math.h>

struct Point
{
    int x;
    int y;
};

float distanceBetweenPoints(const Point& a, const Point& b)
{
    return sqrt( float(a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y) );
}

#endif // POINT_H_INCLUDED
