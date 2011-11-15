#ifndef ROUTE_H_
#define ROUTE_H_

#include <vector>
#include <deque>
#include <map>
#include "Location.h"

using namespace std;

// represents a route from one location to another
class Route {
 public:
  deque<Location> steps;
  Location start;
  Location end;
  int distance;

  Route(const Location& start, const Location& end, map<Location,Location> &p);
};

bool operator<(const Route &a, const Route &b);
bool operator==(const Route &a, const Route &b);

#endif //ROUTE_H_
