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

typedef vector<float> v1f;
typedef vector<v1f> v2f;
typedef vector<v2f> v3f;

// constants
enum { NOMOVE, NORTH, EAST, SOUTH, WEST };
const int TDIRECTIONS = 5;
const char CDIRECTIONS[TDIRECTIONS] = {'-', 'N', 'E', 'S', 'W'};
const int DIRECTIONS[TDIRECTIONS][2]  = { {0, 0}, {-1, 0}, {0,  1}, { 1, 0}, {0, -1} }; //{-, N, E, S, W}
const int UDIRECTIONS[TDIRECTIONS] = { NOMOVE, SOUTH, WEST, NORTH, EAST };
const int kDirichletAlpha = 1;

enum { VISIBLE, LAND, FOOD, TARGET, UNKNOWN, ENEMY, FRIEND };
const int kFactors = 7;
const int kMaxPlayers = 10;

enum { NORMAL, ATTACK, EVADE };
enum { SAFE=0, KILL=1, DIE=2 };
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
  v1f inf;
  v1i deadAnts;
  int sumAttacked;
  v1i attacked;
  v1i fighting;
  v1i status;

  Square();
  void reset();
  void markVisible(int turn);
  //  float influence();
  void setAnt(int sid, int sindex, int sant, bool sisUsed);
  bool isAnt();
  void clearAnt();
  bool isCleared();
  bool isEnemy() { return ant > 0; }
  bool moveAntTo(Square &bs);
  float coefficient(int f);
  bool canDiffuse();
  float isSource(int f);
};

ostream& operator<<(ostream& os, const Square &square);

// A grid location.
class Loc {
 public:
  int r, c;
  Loc() : r(0), c(0) {};
  Loc(const Loc& a) : r(a.r), c(a.c) {};
  Loc(short int r, short int c) : r(r), c(c) {};
};

bool operator<(const Loc &a, const Loc &b);
bool operator==(const Loc &a, const Loc &b);
bool operator!=(const Loc &a, const Loc &b);
ostream& operator<<(ostream &os, const Loc &loc);

struct Offset {
  int d2, r, c;
  Offset(int d2, int r, int c) : d2(d2), r(r), c(c) {};
};

bool operator<(const Offset &a, const Offset &b);
bool operator==(const Offset &a, const Offset &b);
bool operator!=(const Offset &a, const Offset &b);
ostream& operator<<(ostream& os, const Offset &o);

class Sim {
 public:
  Sim() {};
  virtual void makeMove(const Loc &a, int d);
  virtual void go();
};

class Agent {
 public:
  int id;
  Loc loc;
  int mode;
  Agent(int id, const Loc& loc) : id(id), loc(loc), mode(ATTACK) {};
  virtual ~Agent() {};
};

inline int add(int a, int b, int max) {
  return (a + b + max) % max;
}

inline int delta(int a, int b, int w) {
  int d = abs(a - b);
  return min(d, w - d);
}

// store current state information
class State {
 public:
  int rows, cols, turn, turns, players;
  int attackradius, spawnradius, battleradius, viewradius;
  double loadtime, turntime;
  
  int nSquares, nUnknown, nSeen, nVisible;
  vector<Offset> offsets;
  vector<Offset>::iterator offsetSelf, offsetFirst, attackEnd, spawnEnd, battleEnd, viewEnd;
  vector<Offset> aoneOffsets;
  vector< vector<Offset> > aEnterOffsets;
  vector< vector<Offset> > aLeaveOffsets;
  vector<double> scores;
  bool gameover;
  int64_t seed;
  int nextId;
  int battle;
  vector<float> weights;
  vector<float> decay;
  vector<float> loss;

  Sim defaultSim;
  Sim *sim;

  vector< vector<Square> > grid;
  v2i distanceGrid;

  vector<Loc> ants, hills, food;
  vector<Loc> moveFrom;
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

  int distance(const Loc &loc1, const Loc &loc2);
  Loc getLoc(const Loc &startLoc, int direction);
  Loc getLoc(const Loc &loc, const Loc &off);
  Loc getLocNoWrap(const Loc &loc, int direction);
  Loc randomLoc();
  v1i getDirections(const Loc &a, const Loc &b);
  void markVisible(const Loc& a);
  void calcOffsets(int radius2, vector<Loc> &offsets);
  Loc addOffset(const Loc &a, const Offset &o);
  bool hasAntConsistency();

  void putAnt(int r, int c, int player);
  void putDead(int r, int c, int player);
  void putHill(int r, int c, int player);
  void putFood(int r, int c);
  void putWater(int r, int c);
  void updatePlayers(int player);

  void clearAnts(v1i &va);

  void setInfluenceParameter(int f, float w, float d, float l);
  void update();
  void updateVision();
  void updateInfluence(int iterations);
  void updateDead(v1i &is, v1i &dead);
  void initDir();
  void updateDir(int i);
  int pickRandomAnt();
  bool payoffWin(v1i &ants);
  int assertAllAnts(v1i &is);
  void adjustEnemyCount(const Loc &a, int i);
  int countEnemies(const Loc &a);
  int assertEnemyCountsCorrect();
  void computeInfluenceBlend(v3f &temp);
  void computeInfluenceLinear(v3f &temp);
  void writeInfluence(v3f &temp);

  Square *antSquareAt(int i) { return squareAt(ants[i]); }
  Square *squareAt(Loc a) { return &(grid[a.r][a.c]); }
};

ostream& operator<<(ostream &os, const State &state);
istream& operator>>(istream &is, State &state);

ostream& operator<<(ostream& os, const v1i &a);

#endif //STATE_H_
