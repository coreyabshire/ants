#ifndef ROUTE_H_
#define ROUTE_H_

#include <vector>
#include "Location.h"

/*
    struct for representing a square in the grid.
*/
class Route
{
public:
    Location start;
    Location end;
    int distance;

    Route(const Location& start, const Location& end, int distance);
};

bool operator<(const Route &a, const Route &b);
bool operator==(const Route &a, const Route &b);

#endif //SQUARE_H_
