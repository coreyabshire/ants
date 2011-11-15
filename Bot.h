#ifndef BOT_H_
#define BOT_H_

#include <map>
#include <set>
#include <queue>
#include <algorithm>
#include "State.h"
#include "Route.h"
#include "Search.h"

using namespace std;

class Bot {
 public:
  State state;
  Bot();
  void playGame();   //plays a single game of Ants
  void setup();      //set up the bot on initial turn
  void makeMoves();  //makes moves for a single turn
  void endTurn();    //indicates to the engine that it has made its moves

  set<Location> orders;
  set<Location> unseen;
  set<Location> myAnts, myHills, enemyAnts, enemyHills, food;

  bool doMoveDirection(const Location &ant, int d);
  bool doMoveLocation(const Location &antLoc, const Location &destLoc);
  bool doMoveRoutes(vector<Route>& routes, map<Location, Search> &searches,
                    set<Location> &antsUsed);
  vector<Location> shortestPath(const Location &a, const Location &b);
  void insertAll(set<Location> &to, vector<Location> &from);
  void removeIf(set<Location> &locs, bool(*pred)(Square &));
  void updateMemory(set<Location> &memory, vector<Location> &seen, bool(*pred)(Square &));
  bool search(Location &start, set<Location> &ends, Route &route);
  void search(map<Location, Search> &searches,
              set<Location> &sources,
              vector<Route> &foodRoutes,
              vector<Route> &hillRoutes,
              vector<Route> &unseenRoutes);
};

#endif //BOT_H_
