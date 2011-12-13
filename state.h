#ifndef STATE_H_
#define STATE_H_

#include <cstdlib>
#include <iostream>
#include <vector>
#include <deque>

#include "Timer.h"
#include "Bug.h"

using namespace std;

typedef vector<int> v1i;
typedef vector<v1i> v2i;
typedef vector<v2i> v3i;

typedef vector<float> v1f;
typedef vector<v1f> v2f;
typedef vector<v2f> v3f;

typedef vector<bool> v1b;
typedef vector<v1b> v2b;
typedef vector<v2b> v3b;

// constants
enum { NOMOVE, NORTH, EAST, SOUTH, WEST };
const int TDIRECTIONS = 5;
const char CDIRECTIONS[TDIRECTIONS] = {'-', 'N', 'E', 'S', 'W'};
const int DIRECTIONS[TDIRECTIONS][2]  = { {0, 0}, {-1, 0}, {0,  1}, { 1, 0}, {0, -1} }; //{-, N, E, S, W}
const int UDIRECTIONS[TDIRECTIONS] = { NOMOVE, SOUTH, WEST, NORTH, EAST };

enum { FOOD, TARGET, UNKNOWN };
const int kFactors = 3;
const int kMaxPlayers = 10;

enum { NORMAL, ATTACK, EVADE };
enum { SAFE, KILL, DIE };

// A square in the grid.
class Square {
 public:
  bool isVisible, isWater, isHill, isFood, isKnown, isFood2, isHill2, isUsed;
  int index, ant, hill, hill2, sumAttacked, attacked, fighting, best, status;
  v1f inf;

  Square();
  void reset();
  void markVisible();
  void setAnt(int sindex, int sant, bool sisUsed);
  bool isAnt();
  void clearAnt();
  bool isCleared();
  bool isEnemy() { return ant > 0; }
  bool moveAntTo(Square &bs);
  float coefficient(int f);
  bool canDiffuse();
  float isSource(int f);
};

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

struct Off {
  int r, c, d;
  Off(int r, int c, int d) : r(r), c(c), d(d) {};
};

bool operator<(const Off &a, const Off &b);
bool operator==(const Off &a, const Off &b);
bool operator!=(const Off &a, const Off &b);
ostream& operator<<(ostream& os, const Off &o);

typedef vector<Off> v1o;
typedef vector<v1o> v2o;
typedef vector<v2o> v3o;

typedef vector<Square> v1s;
typedef vector<v1s> v2s;
typedef vector<v2s> v3s;

typedef vector<Loc> v1l;
typedef vector<v1l> v2l;
typedef vector<v2l> v3l;

class Sim {
 public:
  Sim() {};
  virtual void makeMove(const Loc &a, int d);
  virtual void go();
};

int add(int a, int b, int m);
int delta(int a, int b, int m);

// store current state information
class State {
 public:
  int rows, cols, turn, turns, players;
  int attackradius, spawnradius, battleradius, viewradius;
  double loadtime, turntime;
  
  v1o offsets, aoneOffsets;
  v1o::iterator offsetSelf, offsetFirst, attackEnd, spawnEnd, viewEnd;
  int iterations;
  v1f weights, decay, loss;

  vector<double> scores;
  bool gameover;
  int64_t seed;

  Sim defaultSim;
  Sim *sim;

  v2s grid;
  v2i dlookup;

  v1l ants, hills, food;
  v1l moveFrom;
  v1i moves;

  Timer timer;
  Bug bug;

  State();
  State(Sim *sim);
  State(int rows, int cols);
  ~State();

  void setup();
  void reset();

  void writeMoves();
  void makeMove(int i, int d);

  int distance(const Loc &a, const Loc &b);
  Loc getLoc(const Loc &startLoc, int direction);
  v1i getDirections(const Loc &a, const Loc &b);
  Loc addOff(const Loc &a, const Off &o);

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
  void updateInfluence();
  void computeInfluence(v3f &temp);
  void writeInfluence(v3f &temp);

  Square *antSquareAt(int i) { return squareAt(ants[i]); }
  Square *squareAt(Loc a) { return &(grid[a.r][a.c]); }
};

istream& operator>>(istream &is, State &state);
ostream& operator<<(ostream& os, const v1i &a);

#endif //STATE_H_
