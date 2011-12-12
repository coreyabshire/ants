#include <cassert>
#include <cmath>
#include <algorithm>
#include "state.h"

Square::Square() : inf(kFactors, 0.0) {
  isVisible = isWater = isHill = isFood = isKnown = isUsed = 0;
  index = -1;
  isFood2 = isHill2 = 0;
  ant = hillPlayer = hillPlayer2 = -1;
  inf[UNKNOWN] = 1.0;
  sumAttacked = 0;
  attacked = v1i(kMaxPlayers, 0);
  fighting = v1i(kMaxPlayers, 0);
  status = v1i(kMaxPlayers, 0);
}

float lerp(float a, float b, float t) {
  return a + t * (b - a);
}

int add(int a, int b, int m) {
  return (a + b + m) % m;
}

int delta(int a, int b, int m) {
  int d = abs(a - b);
  return min(d, m - d);
}

//resets the information for the square except water information
void Square::reset() {
  isVisible = isHill2 = isFood2 = isUsed = 0;
  index = -1;
  ant = hillPlayer2 = -1;
  if (!isWater) {
    if (!isKnown)
      inf[UNKNOWN] = lerp(inf[UNKNOWN], 2.0, 0.01);
    else if (hillPlayer == 0)
      inf[UNKNOWN] = lerp(inf[UNKNOWN], 2.0, 0.04);
    else
      inf[UNKNOWN] = lerp(inf[UNKNOWN], 0.5, 0.01);
  }
  if (sumAttacked > 0) {
    sumAttacked = 0;
    for (int i = 0; i < kMaxPlayers; i++) {
      attacked[i] = 0;
      fighting[i] = 0;
      status[i] = 0;
    }
  }
}

void Square::markVisible() {
  isVisible = isKnown = 1;
  isFood = isFood2;
  isHill = isHill2;
  hillPlayer = hillPlayer2;
  if (!isWater)
    inf[UNKNOWN] = lerp(inf[UNKNOWN], 0.0, 0.3);
}

void Square::setAnt(int sindex, int sant, bool sisUsed) {
  index = sindex;
  ant = sant;
  isUsed = sisUsed;
}

bool Square::moveAntTo(Square &bs) {
  assert(!isUsed);
  assert(isAnt());
  assert(bs.isCleared());
  assert(!bs.isWater);
  assert(!bs.isFood);
  int tIndex, tAnt;
  tIndex = index;
  tAnt = ant;
  clearAnt();
  bs.index = tIndex;
  bs.ant = tAnt;
  bs.isUsed = true;
  return true;
}

bool Square::isAnt() {
  return index != -1 && ant != -1;
}

bool Square::isCleared() {
  return index == -1 && ant == -1 && isUsed == false;
}

void Square::clearAnt() {
  index = -1;
  ant = -1;
  isUsed = false;
}

// constructor
State::State() : turn(0), players(2), gameover(0) {
  bug.open("./debug.txt");
  sim = &defaultSim;
}

State::State(Sim *sim) : turn(0), players(2), gameover(0), sim(sim) {
  bug.open("./debug.txt");
}

// deconstructor
State::~State() {
  bug.close();
}

State::State(int rows, int cols) : rows(rows), cols(cols), turn(0), players(2), gameover(0) {
  bug.open("./debug.txt");
  sim = &defaultSim;
  spawnradius = 1;
  attackradius = 5;
  viewradius = 77;
  setup();
}    

void State::setInfluenceParameter(int f, float w, float d, float l) {
  weights[f] = w;
  decay[f] = d;
  loss[f] = l;
}

// sets the state up
void State::setup() {
  iterations = 20;
  grid = vector< vector<Square> >(rows, vector<Square>(cols, Square()));
  int n = max(rows, cols);
  dlookup = v2i(n, v1i(n, 0));
  weights = v1f(kFactors, 0.0);
  decay = v1f(kFactors, 0.0);
  loss = v1f(kFactors, 0.0);
  setInfluenceParameter(FOOD,     0.25,  0.10,  1.00);
  setInfluenceParameter(TARGET,   1.00,  0.25,  1.00);
  setInfluenceParameter(UNKNOWN,  0.01,  0.01,  1.00);
  // build distance lookup tables
  for (int r = 0; r < n; r++)
    for (int c = 0; c < n; c++)
      dlookup[r][c] = r * r + c * c;
  // build offset list
  for (int r = (1 - n); r < n; r++)
    for (int c = (1 - n); c < n; c++)
      offsets.push_back(Offset(r, c, dlookup[abs(r)][abs(c)]));
  sort(offsets.begin(), offsets.end());
  // capture key offsets
  vector<Offset>::iterator oi = offsets.begin();
  offsetSelf = oi++;
  offsetFirst = oi++;
  for (; (oi < offsets.end()) && (*oi).d <= spawnradius;  oi++); spawnEnd  = oi;
  for (; (oi < offsets.end()) && (*oi).d <= attackradius; oi++); attackEnd = oi;
  for (; (oi < offsets.end()) && (*oi).d <= viewradius;   oi++); viewEnd   = oi;
  // special attack offsets
  for (oi = offsetSelf; oi < attackEnd; oi++) {
    Offset &o = *oi;
    for (int d = 0; d < TDIRECTIONS; d++) {
      int r = o.r + DIRECTIONS[d][0];
      int c = o.c + DIRECTIONS[d][1];
      Offset b(r, c, dlookup[abs(r)][abs(c)]);
      if (find(aoneOffsets.begin(), aoneOffsets.end(), b) == aoneOffsets.end())
        aoneOffsets.push_back(b);
    }
  }
  //sort(aoneOffsets.begin(), aoneOffsets.end());
}

// resets all non-water squares to land and clears the bots ant vector
void State::reset() {
  ants.clear();
  hills.clear();
  food.clear();
  moves.clear();
  moveFrom.clear();
  for(int r = 0; r < rows; r++)
    for(int c = 0; c < cols; c++)
      grid[r][c].reset();
}

void Sim::makeMove(const Loc &a, int d) {
  cout << "o " << (int) a.r << " " << (int) a.c << " " << CDIRECTIONS[d] << endl;
}

void Sim::go() {
  cout << "go" << endl;
}

void State::clearAnts(v1i &va) {
  for (size_t i = 0; i < va.size(); i++)
    antSquareAt(va[i])->clearAnt();
}  

void State::writeMoves() {
  for (size_t i = 0; i < ants.size(); i++) {
    Loc &a = moveFrom[i];
    Loc &b = ants[i];
    int d = moves[i];
    Square &bs = grid[b.r][b.c];
    if (bs.ant == 0 && d != NOMOVE)
      sim->makeMove(a, d);
  }
}

void State::makeMove(int i, int d) {
  Loc a = ants[i];
  Square &as = grid[a.r][a.c];
  if (d != NOMOVE) {
    Loc b = getLoc(a, d);
    Square &bs = grid[b.r][b.c];
    assert(as.isUsed == false);
    assert(bs.isUsed == false);
    if (as.moveAntTo(bs)) {
      moves[i] = d;
      ants[i] = b;
    }
  }
  else {
    grid[a.r][a.c].isUsed = true;
  }
}

void State::undoMove(int i) {
  const Loc a = ants[i];
  Square &as = grid[a.r][a.c];
  int d = moves[i];
  if (d != NOMOVE) {
    d = UDIRECTIONS[d];
    Loc b = getLoc(a, d);
    Square &bs = grid[b.r][b.c];
    bs.ant = as.ant;
    bs.index = as.index;
    as.ant = -1;
    as.index = -1;
    bs.isUsed = false;
    moves[i] = -1;
    ants[i] = b;
  }
  else {
    as.isUsed = false;
  }
}

Loc State::addOffset(const Loc &a, const Offset &o) {
  return Loc(add(a.r, o.r, rows), add(a.c, o.c, cols));
}

int State::distance(const Loc &a, const Loc &b) {
  return dlookup[delta(a.r, b.r, rows)][delta(a.c, b.c, cols)];
}

// returns the new location from moving in a given direction with the edges wrapped
Loc State::getLoc(const Loc &a, int d) {
  return Loc(add(a.r, DIRECTIONS[d][0], rows), add(a.c, DIRECTIONS[d][1], cols));
}

Loc State::getLoc(const Loc &a, const Loc &o) {
  return Loc(add(a.r, o.r, rows), add(a.c, o.c, cols));
}

bool State::hasAntConsistency() {
  for (int r = 0; r < rows; r++)
    for (int c = 0; c < cols; c++)
      if (grid[r][c].ant == -1)
        assert(grid[r][c].isCleared());
      else if (grid[r][c].ant == 0)
        assert(grid[r][c].index >= 0);
      else
        assert(grid[r][c].index >= 0);
  return true;
}

void State::update() {
  assert(hasAntConsistency());
  updateVision();
  updateInfluence();
}

void State::updateVision() {
  for (vector<Loc>::iterator a = ants.begin(); a != ants.end(); a++) {
    Square &as = grid[(*a).r][(*a).c];
    if (as.ant == 0)
      for (vector<Offset>::iterator o = offsetSelf; o != viewEnd; o++)
        squareAt(addOffset(*a, *o))->markVisible();
  }
  v3b used(rows, v2b(cols, v1b(players, false)));
  for (vector<Loc>::iterator a = ants.begin(); a != ants.end(); a++) {
    Square &as = grid[a->r][a->c];
    if (!used[a->r][a->c][as.ant]) {
      used[a->r][a->c][as.ant] = true;
      for (vector<Offset>::iterator o = aoneOffsets.begin(); o != aoneOffsets.end(); o++) {
        Loc b = addOffset(*a, *o);
        Square &bs = grid[b.r][b.c];
        bs.attacked[as.ant]++;
        bs.sumAttacked++;
        for (int i = 0; i < players; i++)
          if (i != as.ant)
            (bs.fighting[i])++;
      }
    }
  }
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      Square &as = grid[r][c];
      if (as.sumAttacked > 0) {
        for (int i = 0; i < players; i++) {
          int best = as.sumAttacked;
          for (int j = 0; j < players; j++)
            if (j != i)
              if (as.fighting[j] < best)
                best = as.fighting[j];
          as.status[i] = as.fighting[i] < best ? SAFE : as.fighting[i] == best ? KILL : DIE;
        }
      }
    }
  }
}

float Square::coefficient(int f) {
  return ant == 0 ? 0.1 : ant >  0 ? 1.0 :  1.0;
}

bool Square::canDiffuse() {
  return !isWater;
}

float Square::isSource(int f) {
  return (isFood && f == FOOD) || (hillPlayer > 0 && f == TARGET);
}

void State::computeInfluence(v3f &temp) {
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      Loc a(r, c);
      Square &as = grid[a.r][a.c];
      if (as.canDiffuse()) {
        vector<float> nsumu(kFactors, 0.0);
        for (int d = 0; d < TDIRECTIONS; d++) {
          Loc b = getLoc(a, d);
          Square &bs = grid[b.r][b.c];
          for (int f = 0; f < kFactors; f++)
            nsumu[f] += bs.inf[f] - as.inf[f];
        }
        for (int f = 0; f < kFactors; f++)
          temp[r][c][f] = as.isSource(f) ? 1.0 : as.coefficient(f) * (as.inf[f] + (decay[f] * nsumu[f]));
      }
      else {
        for (int f = 0; f < kFactors; f++)
          temp[r][c][f] = 0.0;
      }
    }
  }
}

void State::writeInfluence(v3f &temp) {
  for (int r = 0; r < rows; r++)
    for (int c = 0; c < cols; c++)
      for (int f = 0; f < kFactors; f++)
        grid[r][c].inf[f] = max(0.001f, temp[r][c][f]);
}

void State::updateInfluence() {
  static v3f temp(rows, v2f(cols, v1f(kFactors, 0.0)));
  for (int i = 0; i < iterations; i++) {
    computeInfluence(temp);
    writeInfluence(temp);
  }
}

void State::updatePlayers(int player) {
  if (player > players)
    players = player;
}

void State::putWater(int r, int c) {
  grid[r][c].isWater = 1;
}

void State::putFood(int r, int c) {
  grid[r][c].isFood2 = 1;
  food.push_back(Loc(r, c));
}

void State::putHill(int r, int c, int player) {
  updatePlayers(player);
  grid[r][c].isHill2 = 1;
  grid[r][c].hillPlayer2 = player;
  hills.push_back(Loc(r, c));
}

void State::putDead(int r, int c, int player) {
  updatePlayers(player);
}

void State::putAnt(int r, int c, int player) {
  updatePlayers(player);
  Loc a(r, c);
  Square &as = grid[r][c];
  as.ant = player;
  as.index = ants.size();
  assert(as.isUsed == false);
  moveFrom.push_back(a);
  moves.push_back(NOMOVE);
  ants.push_back(a);
}

istream& operator>>(istream &is, State &state) {
  int r, c, p;
  string type, junk;
  while (is >> type)
    if      (type == "end")  { state.gameover = 1; break; }
    else if (type == "turn") { is >> state.turn;   break; }
    else getline(is, junk);
  if (state.turn == 0)
    while (is >> type)
      if      (type == "loadtime")      is >> state.loadtime;
      else if (type == "turntime")      is >> state.turntime;
      else if (type == "rows")          is >> state.rows;
      else if (type == "cols")          is >> state.cols;
      else if (type == "turns")         is >> state.turns;
      else if (type == "player_seed")   is >> state.seed;
      else if (type == "viewradius2")   is >> state.viewradius;
      else if (type == "attackradius2") is >> state.attackradius;
      else if (type == "spawnradius2")  is >> state.spawnradius;
      else if (type == "ready")         { state.timer.start(); break; }
      else getline(is, junk);
  else
    while (is >> type)
      if      (type == "w") { is >> r >> c;       state.putWater(r, c); }
      else if (type == "f") { is >> r >> c;       state.putFood(r, c);  }
      else if (type == "a") { is >> r >> c >> p;  state.putAnt(r, c, p); }
      else if (type == "d") { is >> r >> c >> p;  state.putDead(r, c, p); }
      else if (type == "h") { is >> r >> c >> p;  state.putHill(r, c, p); }
      else if (type == "players") { is >> state.players; }
      else if (type == "scores") {
        state.scores = vector<double>(state.players, 0.0);
        for (int p=0; p<state.players; p++)
          is >> state.scores[p];
      }
      else if (type == "go") {
        if (state.gameover)
          is.setstate(ios::failbit);
        else
          state.timer.start();
        break;
      }
      else getline(is, junk);
  return is;
}

ostream& operator<<(ostream& os, const Square &square) {
  if (square.isVisible)       os << "V";
  if (square.isWater)         os << "W";
  if (square.isHill)          os << "H";
  if (square.isFood)          os << "F";
  if (square.ant >= 0)        os << "," << square.ant;
  if (square.hillPlayer >= 0) os << "," << square.hillPlayer;
  return os;
}

bool operator<(const Loc &a, const Loc &b) {
  return a.c == b.c ? a.r < b.r : a.c < b.c;
}

bool operator==(const Loc &a, const Loc &b) {
  return a.r == b.r && a.c == b.c;
}

bool operator!=(const Loc &a, const Loc &b) {
  return a.r != b.r || a.c != b.c;
}

ostream& operator<<(ostream &os, const Loc &loc) {
  return os << "(" << (int)loc.r << "," << (int)loc.c << ")";
}

ostream& operator<<(ostream& os, const Offset &o) {
  return os << o.d << " " << o.r << "," << o.c;
}

bool operator<(const Offset &a, const Offset &b) {
  return a.d == b.d
      ? (a.r == b.r ? a.c < b.c : a.r < b.r)
      : (a.d < b.d);
}

bool operator==(const Offset &a, const Offset &b) {
  return a.r == b.r && a.c == b.c && a.d == b.d;
}

bool operator!=(const Offset &a, const Offset &b) {
  return a.r != b.r || a.c != b.c || a.d != b.d;
}


