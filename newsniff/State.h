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
#include <algorithm>

#include "Timer.h"
#include "Bug.h"
#include "Square.h"
#include "Location.h"
#include "Offset.h"

/*
  constants
*/
const int TDIRECTIONS = 4;
const char CDIRECTIONS[4] = {'N', 'E', 'S', 'W'};
const int DIRECTIONS[4][2] = { {-1, 0}, {0, 1}, {1, 0}, {0, -1} };      //{N, E, S, W}
const float weights[kFactors] = {1.0, 1.9, 0.2, 0.0};

/*
  struct to store current state information
*/
struct State {
  int rows, cols, turn, turns, noPlayers;
  double attackradius, spawnradius, viewradius;
  int attackradius2, spawnradius2, viewradius2;
  double loadtime, turntime;
  std::vector<double> scores;
  bool gameover;

  std::vector< std::vector<Square> > grid;
  std::vector<Location> myAnts, enemyAnts, myHills, enemyHills, food;
  std::vector<Offset> offsets;
  std::vector< std::vector<double> > distanceLookup;
  std::vector< std::vector<int> > distanceSquaredLookup;

  Timer timer;
  Bug bug;

  /*
    Functions
  */
  State();
  ~State();

  void setup();
  void reset();

  void makeMove(const Location &loc, int direction);

  double distance(const Location &loc1, const Location &loc2);
  Location getLocation(const Location &startLoc, int direction);

  void updateVisionInformation();
  void updateInfluenceInformation();
  void dumpInfluenceInformation();

  inline int addWrap(int a, int b, int max);
  inline Location addOffset(const Location &a, const Offset &o);
};

std::ostream& operator<<(std::ostream &os, const State &state);
std::istream& operator>>(std::istream &is, State &state);

#endif //STATE_H_
