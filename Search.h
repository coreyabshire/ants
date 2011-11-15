#ifndef SEARCH_H_
#define SEARCH_H_

#include <map>
#include <set>
#include "Location.h"
#include "State.h"

class Search {
 public:
  Location start;
  std::map<Location,int> distances;
  std::map<Location,Location> predecessors;
  std::set<Location> expanded;
  std::queue<Location> remaining;
  int food, hills, unseen;

  Search() {};
  Search(const Location &start);
  int distance(const Location& dest);
  Location step(const Location& dest);
};

#endif //SEARCH_H_
