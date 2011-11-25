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

#include "Timer.h"
#include "Bug.h"
#include "Square.h"
#include "Location.h"

using namespace std;

const int TDIRECTIONS = 4;
const char CDIRECTIONS[4] = {'N', 'E', 'S', 'W'};
const int DIRECTIONS[4][2] = { {-1, 0}, {0, 1}, {1, 0}, {0, -1} }; //{N, E, S, W}

struct State {
  int rows, cols, turn, turns, noPlayers;
  double attackradius, spawnradius, viewradius;
  double loadtime, turntime;
  vector<double> scores;
  bool gameover;

  vector< vector<Square> > grid;
  vector<Location> myAnts, enemyAnts, myHills, enemyHills, food;

  Timer timer;
  Bug bug;

  State();
  ~State();

  void setup();
  void reset();

  void makeMove(const Location &loc, int direction);

  double distance(const Location &loc1, const Location &loc2);
  Location getLocation(const Location &startLoc, int direction);

  void updateVisionInformation();
};

ostream& operator<<(ostream &os, const State &state);
istream& operator>>(istream &is, State &state);

#endif //STATE_H_
