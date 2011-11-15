#ifndef LOCATION_H_
#define LOCATION_H_

#include <iostream>

// A grid location.
class Location {
 public:
  short int row, col;
  Location() : row(0), col(0) {};
  Location(const Location& loc) : row(loc.row), col(loc.col) {};
  Location(int r, int c) : row(r), col(c) {};
};

bool operator<(const Location &a, const Location &b);
bool operator==(const Location &a, const Location &b);
bool operator!=(const Location &a, const Location &b);
std::ostream& operator<<(std::ostream &os, const Location &loc);

#endif //LOCATION_H_
