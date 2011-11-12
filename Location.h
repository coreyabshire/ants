#ifndef LOCATION_H_
#define LOCATION_H_

#include <iostream>

/*
    struct for representing locations in the grid.
*/
struct Location
{
    int row, col;

    Location()
    {
        row = col = 0;
    };

    Location(const Location& loc) : row(loc.row), col(loc.col) {};

    Location(int r, int c)
    {
        row = r;
        col = c;
    };

};

bool operator<(const Location &a, const Location &b);
bool operator==(const Location &a, const Location &b);
std::ostream& operator<<(std::ostream &os, const Location &loc);

#endif //LOCATION_H_
