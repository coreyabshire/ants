#ifndef ANT_H_
#define ANT_H_

#include <map>
#include <set>
#include <queue>
#include <deque>
#include <algorithm>
#include <list>
#include "State.h"
#include "Route.h"
#include "Search.h"

using namespace std;

class Ant {
 public:
  int id;
  int mode;
  Route route;
};

#endif // ANT_H_
