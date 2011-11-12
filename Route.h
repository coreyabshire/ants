#ifndef ROUTE_H_
#define ROUTE_H_

#include <vector>
#include "Location.h"

/*
    struct for representing a square in the grid.
*/
struct Route
{
    Location start;
    Location end;
    double distance;

    Route(const Location& start, const Location& end, double distance);
};

bool operator<(const Route &a, const Route &b);
bool operator==(const Route &a, const Route &b);

#endif //SQUARE_H_
