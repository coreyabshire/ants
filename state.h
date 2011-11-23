#ifndef STATE_H_
#define STATE_H_

#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <cmath>
#include <string>
#include <vector>
#include <queue>
#include <stack>
#include <set>
#include <stdint.h>
#include <vector>
#include <deque>
#include <map>
#include <algorithm>

#include "Timer.h"
#include "Bug.h"

using namespace std;

// constants
const int TDIRECTIONS = 4;
const char CDIRECTIONS[4] = {'N', 'E', 'S', 'W'};
const int DIRECTIONS[4][2] = { {-1, 0}, {0, 1}, {1, 0}, {0, -1} }; //{N, E, S, W}
const int NORTH = 0, EAST = 1, SOUTH = 2, WEST = 3;

enum { LAND, FOOD, TARGET, UNKNOWN, ENEMY };
const int kFactors = 5;
const float weights[kFactors] = {0.0, 1.0, 1.9, 0.2, 0.0};

// A square in the grid.
class Square {
 public:
  bool isVisible, isWater, isHill, isFood, isKnown, isFood2, isHill2;
  bool isLefty, isStraight;
  int direction;
  int ant, hillPlayer, hillPlayer2, lastSeen;
  double foodScent;
  float inf[kFactors];
  vector<int> deadAnts;

  Square();
  void reset();
  void markVisible(int turn);
  float influence();
};

ostream& operator<<(ostream& os, const Square &square);

// A grid location.
class Location {
 public:
  int row, col;
  Location() : row(0), col(0) {};
  Location(const Location& loc) : row(loc.row), col(loc.col) {};
  Location(short int r, short int c) : row(r), col(c) {};
};

bool operator<(const Location &a, const Location &b);
bool operator==(const Location &a, const Location &b);
bool operator!=(const Location &a, const Location &b);
ostream& operator<<(ostream &os, const Location &loc);

struct Offset {
  double d;
  int d2, r, c;
  Offset(double d, int d2, int r, int c) : d(d), d2(d2), r(r), c(c) {};
};

bool operator<(const Offset &a, const Offset &b);
ostream& operator<<(ostream& os, const Offset &o);

// store current state information
class State {
 public:
  int rows, cols, turn, turns, noPlayers;
  unsigned short int nSquares, nUnknown, nSeen, nVisible;
  double attackradius, spawnradius, viewradius;
  short int attackradius2, spawnradius2, viewradius2;
  vector<Offset> offsets;
  double loadtime, turntime;
  vector<double> scores;
  bool gameover;
  int64_t seed;

  vector< vector<Square> > grid;
  vector< vector<double> > distanceGrid;
  vector< vector<int> > distance2Grid;

  vector<Location> myAnts, enemyAnts, myHills, enemyHills, food;

  Timer timer;
  Bug bug;

  State();
  State(int rows, int cols);
  ~State();

  void setup();
  void reset();

  void makeMove(const Location &loc, int direction);

  double distance(const Location &loc1, const Location &loc2);
  int distance2(const Location &loc1, const Location &loc2);
  int manhattan(const Location &a, const Location &b);
  Location getLocation(const Location &startLoc, int direction);
  Location getLocation(const Location &loc, const Location &off);
  Location getLocationNoWrap(const Location &loc, int direction);
  Location randomLocation();
  vector<int> getDirections(const Location &a, const Location &b);
  void markVisible(const Location& a);
  void calcOffsets(int radius2, vector<Location> &offsets);
  inline Location addOffset(const Location &a, const Offset &o);

  void updateVisionInformation();
  void updateInfluenceInformation();
  void dumpInfluenceInformation();

  Square& squareAt(Location a) { return grid[a.row][a.col]; }
  Square& operator[](Location a) { return squareAt(a); }
};

inline int addWrap(int a, int b, int max);
inline int diffWrap(int a, int b, int max);
inline Location addOffset(const Location &a, const Offset &o);

ostream& operator<<(ostream &os, const State &state);
istream& operator>>(istream &is, State &state);

// represents a route from one location to another
class Route {
 public:
  deque<Location> steps;
  Location start, end;
  int distance;

  Route() : start(Location(0,0)), end(Location(0,0)), distance(0), steps(deque<Location>()) {};
  Route(const Location& start, const Location& end, map<Location,Location> &p);
  Route(const Route &r) {
    steps = r.steps;
    start = r.start;
    end = r.end;
    distance = r.distance;
  };
  
  void flip();

};

bool operator<(const Route &a, const Route &b);
bool operator==(const Route &a, const Route &b);
ostream& operator<<(ostream& os, const Route &r);

#endif //STATE_H_
