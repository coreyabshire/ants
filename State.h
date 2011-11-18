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
#include <stdint.h>

#include "Timer.h"
#include "Bug.h"
#include "Square.h"
#include "Location.h"

// constants
const int TDIRECTIONS = 4;
const char CDIRECTIONS[4] = {'N', 'E', 'S', 'W'};
const int DIRECTIONS[4][2] = { {-1, 0}, {0, 1}, {1, 0}, {0, -1} }; //{N, E, S, W}
const int NORTH = 0, EAST = 1, SOUTH = 2, WEST = 3;

// store current state information
class State {
 public:
  int rows, cols,
    turn, turns,
    noPlayers;
  double attackradius, spawnradius, viewradius;
  short int attackradius2, spawnradius2, viewradius2;
  double loadtime, turntime;
  std::vector<double> scores;
  bool gameover;
  int64_t seed;

  std::vector< std::vector<Square> > grid;
  std::vector<Location> myAnts, enemyAnts, myHills, enemyHills, food;

  Timer timer;
  Bug bug;

  State();
  ~State();

  void setup();
  void reset();

  void makeMove(const Location &loc, int direction);

  double distance(const Location &loc1, const Location &loc2);
  int distance2(const Location &loc1, const Location &loc2);
  int manhattan(const Location &a, const Location &b);
  Location getLocation(const Location &startLoc, int direction);
  std::vector<int> getDirections(const Location &a, const Location &b);

  void updateVisionInformation();
};

std::ostream& operator<<(std::ostream &os, const State &state);
std::istream& operator>>(std::istream &is, State &state);

#endif //STATE_H_
