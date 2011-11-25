#ifndef LOCATION_H_
#define LOCATION_H_

#include <iostream>

using namespace std;

/* struct for representing locations in the grid. */
struct Location {
  int row, col;

  Location() {
    row = col = 0;
  }

  Location(int r, int c) {
    row = r;
    col = c;
  }
};

ostream& operator<<(ostream& os, const Location &a);

#endif //LOCATION_H_
