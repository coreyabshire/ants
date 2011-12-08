#ifndef STATE_H_
#define STATE_H_

#include <iostream>
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <vector>
#include <deque>
#include <algorithm>

#include "Timer.h"
#include "Bug.h"

using namespace std;

typedef vector<int> v1i;
typedef vector<v1i> v2i;
typedef vector<v2i> v3i;

// constants
enum { NOMOVE, NORTH, EAST, SOUTH, WEST };
const int TDIRECTIONS = 5;
const char CDIRECTIONS[TDIRECTIONS] = {'-', 'N', 'E', 'S', 'W'};
const int DIRECTIONS[TDIRECTIONS][2]  = { {0, 0}, {-1, 0}, {0,  1}, { 1, 0}, {0, -1} }; //{-, N, E, S, W}
const int UDIRECTIONS[TDIRECTIONS] = { NOMOVE, SOUTH, WEST, NORTH, EAST };
const int kDirichletAlpha = 1;

enum { VISIBLE, LAND, FOOD, TARGET, UNKNOWN, ENEMY, FRIEND };
const int kFactors = 7;
const float weights[kFactors] = {0.0, 0.0, 0.8, 1.0, 0.2, 0.1, 0.0};
const float decay[kFactors] =   {0.0, 0.1, 0.3, 0.4, 0.2, 0.2, 0.1};
const float loss[kFactors] =    {0.9, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};

enum { NORMAL, ATTACK, EVADE };
const int kModes = 3;

// A square in the grid.
class Square {
 public:
  bool isVisible, isWater, isHill, isFood, isKnown, isFood2, isHill2, isWarzone;
  bool isLefty, isStraight, isUsed;
  int direction;
  int id, index;
  int ant, hillPlayer, hillPlayer2, lastSeen;
  int ant2, id2;
  int good, goodmove, bad, badmove;
  int enemies;
  int battle;
  vector<float> inf;
  vector<int> deadAnts;

  Square(int players);
  void reset();
  void markVisible(int turn);
  float influence();
  void setAnt(int sid, int sindex, int sant, bool sisUsed);
  bool isAnt();
  void clearAnt();
  bool isCleared();
  bool isEnemy() { return ant > 0; }
  bool moveAntTo(Square &bs);

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

class Move {
 public:
  Location a;
  Location b;
  int d;
};

class Turn {
 public:
  vector<Move> moves;
  vector< vector<bool> > used;
};

struct Offset {
  double d;
  int d2, r, c;
  Offset(double d, int d2, int r, int c) : d(d), d2(d2), r(r), c(c) {};
};

bool operator<(const Offset &a, const Offset &b);
ostream& operator<<(ostream& os, const Offset &o);

class Sim {
 public:
  Sim() {};
  virtual void makeMove(const Location &a, int d);
  virtual void go();
};

class Agent {
 public:
  int id;
  Location loc;
  int mode;
  Agent(int id, const Location& loc) : id(id), loc(loc), mode(ATTACK) {};
  virtual ~Agent() {};
};

inline int addWrap(int a, int b, int max) {
  return (a + b + max) % max;
}

inline int diffWrap(int a, int b, int w) {
  int d = abs(a - b);
  return min(d, w - d);
}

inline int absdiff(int a, int b) {
  return a > b ? a - b : b - a;
}

// store current state information
class State {
 public:
  int rows, cols, turn, turns, players;
  int attackradius2, spawnradius2, battleradius2, viewradius2;
  double loadtime, turntime;
  
  int nSquares, nUnknown, nSeen, nVisible;
  double attackradius, spawnradius, battleradius, viewradius;
  vector<Offset> offsets;
  vector<Offset>::iterator offsetSelf, offsetFirst, attackEnd, spawnEnd, battleEnd, viewEnd;
  vector<double> scores;
  bool gameover;
  int64_t seed;
  int nextId;
  int battle;

  Sim defaultSim;
  Sim *sim;

  vector< vector<Square> > grid;
  vector< vector<double> > distanceGrid;
  v2i distance2Grid;

  vector<Location> ants, hills, food;
  vector<Location> moveFrom;
  v1i moves;
  v2i dir;

  Timer timer;
  Bug bug;

  int nextAntId;
  vector<Agent> agents;
  void createMissingAgents();

  State();
  State(Sim *sim);
  State(int rows, int cols);
  ~State();

  void setup();
  void reset();

  void writeMoves();
  void makeMove(int i, int d);
  void undoMove(int i);
  bool tryMoves(vector<int> &va, vector<int> &vm);
  void undoMoves(vector<int> &va);
  void printAnts(vector<int> &ants);
  
  double distance(const Location &loc1, const Location &loc2);
  int distance2(const Location &loc1, const Location &loc2);
  int manhattan(const Location &a, const Location &b);
  Location getLocation(const Location &startLoc, int direction);
  Location getLocation(const Location &loc, const Location &off);
  Location getLocationNoWrap(const Location &loc, int direction);
  Location randomLocation();
  v1i getDirections(const Location &a, const Location &b);
  void markVisible(const Location& a);
  void calcOffsets(int radius2, vector<Location> &offsets);
  Location addOffset(const Location &a, const Offset &o);
  bool hasAntConsistency();
  //  bitset<64> summarize(const Location &a);

  void putAnt(int row, int col, int player);
  void putDead(int row, int col, int player);
  void putHill(int row, int col, int player);
  void putFood(int row, int col);
  void putWater(int row, int col);
  void updatePlayers(int player);

  void clearAnts(vector<int> &va);

  void update();
  void updateVision();
  void updateInfluence(int iterations);
  void updateDead(v1i &is, v1i &dead);
  void initDir();
  void updateDir(int i);
  int pickRandomAnt();
  bool payoffWin(v1i &ants);
  int assertAllAnts(v1i &is);
  void adjustEnemyCount(const Location &a, int i);
  int countEnemies(const Location &a);
  int assertEnemyCountsCorrect();

  Square *antSquareAt(int i) { return squareAt(ants[i]); }
  Square *squareAt(Location a) { return &(grid[a.row][a.col]); }
};


ostream& operator<<(ostream &os, const State &state);
istream& operator>>(istream &is, State &state);

#endif //STATE_H_
