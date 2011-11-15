#include "Location.h"

bool operator<(const Location &a, const Location &b) {
  if (a.row < b.row)
    return true;
  else if (a.row > b.row) 
    return false;
  else
    return a.col < b.col;
}

bool operator==(const Location &a, const Location &b) {
  return a.row == b.row && a.col == b.col;
}

bool operator!=(const Location &a, const Location &b) {
  return a.row != b.row || a.col != b.col;
}

std::ostream& operator<<(std::ostream &os, const Location &loc) {
  os << "(" << loc.row << "," << loc.col << ")";
  return os;
}

