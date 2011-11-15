#include "Route.h"

Route::Route(const Location& start, const Location& end, map<Location,Location> &p)
{
  for (Location c = end; c != start; c = p[c]) {
    steps.push_front(c);
    distance++;
  }
  steps.push_front(start);
  distance++;
};

bool operator<(const Route &a, const Route &b) {
  return a.distance < b.distance;
}

bool operator==(const Route &a, const Route &b) {
  return a.distance == b.distance
      && a.start == b.start
      && a.end == b.end;
}
