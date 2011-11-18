#ifndef BOT_H_
#define BOT_H_

#include <map>
#include <set>
#include <queue>
#include <deque>
#include <algorithm>
#include <list>
#include "State.h"

using namespace std;

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
  list< Route > xroutes;
  int nextAntId;

  map<Location,int> straight;
  map<Location,int> lefty;
  map<Location,Route> routes;

  void goLefty(set<Location> &antsUsed);  //makes moves for a single turn
  bool doMoveDirection(const Location &ant, int d);
  bool doMoveLocation(const Location &antLoc, const Location &destLoc);
  bool doMoveRoute(Route &route, set<Location> &antsUsed);
  bool doMoveRoutes(list< Route >& routes, set<Location> &antsUsed,
                    set<Location> targets);
  vector<Location> shortestPath(const Location &a, const Location &b);
  void insertAll(set<Location> &to, vector<Location> &from);
  void removeIf(set<Location> &locs, bool(*pred)(Square &));
  void updateMemory(set<Location> &memory, vector<Location> &seen, bool(*pred)(Square &));
  bool search(Location &start, set<Location> &ends, Route &route);
  void search(set<Location> &sources, set<Location> &targets, list< Route > &routes);
  void search(set<Location> &sources, list< Route > &routes);
  int search(set<Location> &sources, const Location &target, list< Route > &routes, int maxCount);
  bool search(Location &start, Location &goal, Route &route);
};

#endif //BOT_H_
