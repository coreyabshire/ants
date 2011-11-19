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

// A square in the grid.
class Square {
 public:
  bool isVisible, isWater, isHill, isFood, isSeen;
  int ant, hillPlayer, lastSeen;
  std::vector<int> deadAnts;

  Square() {
    isVisible = isWater = isHill = isFood = isSeen = 0;
    ant = hillPlayer = -1;
  };

  //resets the information for the square except water information
  void reset() {
    isVisible = isHill = isFood = 0;
    ant = hillPlayer = -1;
    deadAnts.clear();
  };

  void markVisible(int turn) {
    isVisible = isSeen = 1;
    lastSeen = turn;
  };
};

std::ostream& operator<<(std::ostream& os, const Square &square);

// A grid location.
class Location {
 public:
  short int row, col;
  Location() : row(0), col(0) {};
  Location(const Location& loc) : row(loc.row), col(loc.col) {};
  Location(short int r, short int c) : row(r), col(c) {};
};

bool operator<(const Location &a, const Location &b);
bool operator==(const Location &a, const Location &b);
bool operator!=(const Location &a, const Location &b);
std::ostream& operator<<(std::ostream &os, const Location &loc);

// store current state information
class State {
 public:
  int rows, cols, turn, turns, noPlayers;
  unsigned short int nSquares, nUnknown, nSeen, nVisible;
  double attackradius, spawnradius, viewradius;
  short int attackradius2, spawnradius2, viewradius2;
  double loadtime, turntime;
  vector<double> scores;
  bool gameover;
  int64_t seed;

  vector< vector<Square> > grid;
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
  vector<int> getDirections(const Location &a, const Location &b);
  void markVisible(const Location& a);

  void updateVisionInformation();

  Square& squareAt(Location a) { return grid[a.row][a.col]; }
  Square& operator[](Location a) { return squareAt(a); }
};

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
