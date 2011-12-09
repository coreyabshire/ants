#include "state.h"

Square::Square() : inf(kFactors, 0.0) {
  isVisible = isWater = isHill = isFood = isKnown = isWarzone = 0;
  good = goodmove = bad = badmove = 0;
  enemies = 0;
  isUsed = false;
  id = -1;
  index = -1;
  isFood2 = isHill2 = 0;
  inf[LAND] = 1.0;
  direction = NOMOVE;
  battle = -1;
  ant = hillPlayer = hillPlayer2 = -1;
  ant2 = -1;
  id2 = -1;
  sumAttacked = 0;
  attacked = v1i(kMaxPlayers, 0);
  fighting = v1i(kMaxPlayers, 0);
  status = v1i(kMaxPlayers, 0);
}

//resets the information for the square except water information
void Square::reset() {
  isVisible = isHill2 = isFood2 = isWarzone = 0;
  good = goodmove = bad = badmove = 0;
  enemies = 0;
  isUsed = false;
  index = -1;
  direction = NOMOVE;
  battle = -1;
  ant = hillPlayer2 = ant2 = -1;
  // inf[VISIBLE] *= loss[VISIBLE];
  //for (vector<int>::iterator i = points.begin(); i != points.end(); i++)
  //  *i = 0;
  deadAnts.clear();
  if (sumAttacked > 0) {
    sumAttacked = 0;
    for (int i = 0; i < kMaxPlayers; i++) {
      attacked[i] = 0;
      fighting[i] = 0;
      status[i] = 0;
    }
  }
}

void Square::markVisible(int turn) {
  isVisible = isKnown = 1;
  isFood = isFood2;
  isHill = isHill2;
  hillPlayer = hillPlayer2;
  lastSeen = turn;
  inf[VISIBLE] = 1.0;
}

void Square::setAnt(int sid, int sindex, int sant, bool sisUsed) {
  id = sid;
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
  int tId, tIndex, tAnt;
  tId = id;
  tIndex = index;
  tAnt = ant;
  clearAnt();
  bs.id = tId;
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
  id = -1;
  index = -1;
  ant = -1;
  isUsed = false;
}

// constructor
State::State() : turn(0), players(2), gameover(0), nextId(0) {
  bug.open("./debug.txt");
  sim = &defaultSim;
}

State::State(Sim *sim) : turn(0), players(2), gameover(0), nextId(0), sim(sim) {
  bug.open("./debug.txt");
}

// deconstructor
State::~State() {
  bug.close();
}

State::State(int rows, int cols) : rows(rows), cols(cols), turn(0), players(2), gameover(0), nextId(0) {
  bug.open("./debug.txt");
  sim = &defaultSim;
  spawnradius2 = 1;
  attackradius2 = 5;
  viewradius2 = 77;
  spawnradius = sqrt(spawnradius2);
  attackradius = sqrt(attackradius2);
  viewradius = sqrt(viewradius2);
  battleradius = attackradius + 2.0;
  battleradius2 = (int)(battleradius * battleradius);
  setup();
}    

void State::setInfluenceParameter(int f, float w, float d, float l) {
  weights[f] = w;
  decay[f] = d;
  loss[f] = l;
}

// sets the state up
void State::setup() {
  nSquares = nUnknown = rows * cols;
  nSeen = nVisible = 0;
  battle = -1;
  nextAntId = 0;
  grid = vector< vector<Square> >(rows, vector<Square>(cols, Square()));
  int n = max(rows, cols);
  distance2Grid = v2i(n, v1i(n, 0));
  distanceGrid = vector< vector<double> >(n, vector<double>(n, 0.0));
  battleradius = (attackradius + 2.0);
  battleradius2 = round(battleradius * battleradius);
  // weights enum { VISIBLE, LAND, FOOD, TARGET, UNKNOWN, ENEMY, FRIEND };
  weights = vector<float>(kFactors, 0.0);
  decay = vector<float>(kFactors, 0.0);
  loss = vector<float>(kFactors, 0.0);
  setInfluenceParameter(VISIBLE,  0.00,  0.00,  0.90);
  setInfluenceParameter(LAND,     0.00,  0.10,  1.00);
  setInfluenceParameter(FOOD,     0.80,  0.12,  1.00);
  setInfluenceParameter(TARGET,   1.00,  0.20,  1.00);
  setInfluenceParameter(UNKNOWN,  0.50,  0.10,  1.00);
  setInfluenceParameter(ENEMY,    0.10,  0.01,  1.00);
  setInfluenceParameter(FRIEND,   0.00,  0.01,  1.00);
// [kFactors] = {0.0, 0.0, 0.8, 1.0, 0.2, 0.1, 0.0};
// [kFactors] =   {0.0, 0.1, 0.2, 0.25, 0.1, 0.1, 0.1};
// [kFactors] =    {0.9, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};

  // build distance lookup tables
  for (int r = 0; r < n; r++) {
    for (int c = 0; c < n; c++) {
      int d = r * r + c * c;
      distance2Grid[r][c] = d;
      distanceGrid[r][c] = sqrt(d);
    }
  }
  // build offset list
  for (int r = (1 - n); r < n; r++) {
    for (int c = (1 - n); c < n; c++) {
      int ar = abs(r), ac = abs(c);
      offsets.push_back(Offset(distanceGrid[ar][ac], distance2Grid[ar][ac], r, c));
    }
  }
  sort(offsets.begin(), offsets.end());
  // capture key offsets
  vector<Offset>::iterator offset = offsets.begin();
  offsetSelf = offset++;
  offsetFirst = offset++;
  for (; (offset < offsets.end()) && (*offset).d2 <= spawnradius2; offset++);
  spawnEnd = offset;
  for (; (offset < offsets.end()) && (*offset).d2 <= attackradius2; offset++);
  attackEnd = offset;
  for (; (offset < offsets.end()) && (*offset).d2 <= battleradius2; offset++);
  battleEnd = offset;
  for (; (offset < offsets.end()) && (*offset).d2 <= viewradius2; offset++);
  viewEnd = offset;

  // special attack offsets
  for (vector<Offset>::iterator oi = offsetSelf; oi < attackEnd; oi++) {
    Offset &o = *oi;
    for (int d = 0; d < TDIRECTIONS; d++) {
      int r = o.r + DIRECTIONS[d][0];
      int c = o.c + DIRECTIONS[d][1];
      int ar = abs(r);
      int ac = abs(c);
      Offset b(distanceGrid[ar][ac], distance2Grid[ar][ac], r, c);
      if (find(aoneOffsets.begin(), aoneOffsets.end(), b) == aoneOffsets.end()) {
        aoneOffsets.push_back(b);
      }
    }
  }
  //sort(aoneOffsets.begin(), aoneOffsets.end());
}

// resets all non-water squares to land and clears the bots ant vector
void State::reset() {
  nVisible = 0;
  battle = -1;
  ants.clear();
  hills.clear();
  food.clear();
  moves.clear();
  moveFrom.clear();
  dir.clear();
  for(int row = 0; row < rows; row++)
    for(int col = 0; col < cols; col++)
      grid[row][col].reset();
}

void Sim::makeMove(const Location &loc, int d) {
  cout << "o " << (int) loc.row << " " << (int) loc.col << " " << CDIRECTIONS[d] << endl;
}

void Sim::go() {
  cout << "go" << endl;
}

ostream& operator<<(ostream& os, const vector<int> &a);

void State::clearAnts(v1i &va) {
  for (size_t i = 0; i < va.size(); i++)
    antSquareAt(va[i])->clearAnt();
}  

void State::writeMoves() {
  for (size_t i = 0; i < ants.size(); i++) {
    Location &a = moveFrom[i];
    Location &b = ants[i];
    int d = moves[i];
    Square &bs = grid[b.row][b.col];
    if (bs.ant == 0 && d != NOMOVE)
      sim->makeMove(a, d);
  }
}

int State::countEnemies(const Location &a) {
  int n = 0;
  Square &as = grid[a.row][a.col];
  assert(as.ant != -1);
  for (vector<Offset>::iterator o = offsetSelf; o != attackEnd; o++) {
    Location b = addOffset(a, *o);
    Square &bs = grid[b.row][b.col];
    if (bs.ant != -1 && as.ant != bs.ant)
      ++n;
  }
  return n;
}

int State::assertEnemyCountsCorrect() {
  for (size_t i = 0; i < ants.size(); i++) {
    Location a = ants[i];
    Square &as = grid[a.row][a.col];
    assert(as.enemies == countEnemies(a));
  }
  return 1;
}

void State::adjustEnemyCount(const Location &a, int i) {
  Square &as = grid[a.row][a.col];
  assert(as.ant != -1);
  for (vector<Offset>::iterator o = offsetSelf; o != attackEnd; o++) {
    Location b = addOffset(a, *o);
    Square &bs = grid[b.row][b.col];
    if (bs.ant != -1 && as.ant != bs.ant) {
      as.enemies += i;
      bs.enemies += i;
    }
  }
}

void State::makeMove(int i, int d) {
  Location a = ants[i];
  Square &as = grid[a.row][a.col];
  if (d != NOMOVE) {
    if (as.isWarzone >= 0)
      adjustEnemyCount(a, -1);
    Location b = getLocation(a, d);
    Square &bs = grid[b.row][b.col];
    assert(as.isUsed == false);
    assert(bs.isUsed == false);
    if (as.moveAntTo(bs)) {
      moves[i] = d;
      ants[i] = b;
    }
    if (as.isWarzone >= 0)
      adjustEnemyCount(b, 1);
  }
  else {
    grid[a.row][a.col].isUsed = true;
  }
}

void State::undoMove(int i) {
  const Location a = ants[i];
  Square &as = grid[a.row][a.col];
  int d = moves[i];
  if (d != NOMOVE) {
    if (as.isWarzone >= 0)
      adjustEnemyCount(a, -1);
    d = UDIRECTIONS[d];
    Location b = getLocation(a, d);
    Square &bs = grid[b.row][b.col];
    bs.ant = as.ant;
    bs.id = as.id;
    bs.index = as.index;
    as.ant = -1;
    as.id = -1;
    as.index = -1;
    bs.isUsed = false;
    moves[i] = -1;
    ants[i] = b;
    if (as.isWarzone >= 0)
      adjustEnemyCount(b, 1);
  }
  else {
    as.isUsed = false;
  }
}

bool State::tryMoves(v1i &va, v1i &vm) {
  vector<Location> pa;
  vector<Location> pb;
  v1i sId, sIndex, sAnt;
  vector< vector<bool> > used(rows, vector<bool>(cols, false));
  // store
  for (size_t i = 0; i < va.size(); i++) {
    Location &a = ants[va[i]];
    Square &as = grid[a.row][a.col];
    Location b = getLocation(a, vm[i]);
    Square &bs = grid[b.row][b.col];
    assert(!as.isUsed);
    if (bs.isWater || bs.isFood || bs.isUsed || used[b.row][b.col])
      return false;
    // prevent stomping on an ant that is not going to be otherwise moved in this set?
    if (!bs.isCleared() && find(va.begin(), va.end(), bs.index) == va.end())
      return false;
    used[b.row][b.col] = true;
    pa.push_back(a);
    pb.push_back(b);
    sId.push_back(as.id);
    sIndex.push_back(as.index);
    sAnt.push_back(as.ant);
  }
  for (size_t i = 0; i < va.size(); i++)
    if (antSquareAt(va[i])->isWarzone)
      adjustEnemyCount(pa[i], -1);
  clearAnts(va);
  // write
  for (size_t i = 0; i < va.size(); i++) {
    ants[va[i]] = pb[i];
    moves[va[i]] = vm[i];
    antSquareAt(va[i])->setAnt(sId[i], sIndex[i], sAnt[i], true);
  }
  for (size_t i = 0; i < va.size(); i++)
    if (antSquareAt(va[i])->isWarzone)
      adjustEnemyCount(pb[i], 1);
  assert(assertEnemyCountsCorrect());
  return true;
}

void State::undoMoves(v1i &va) {
  // for (int i = 0; i < va.size(); i++) {
  //   bug << "to undo ant"
  //       << " " << va[i] << " " << ants[va[i]] << " " << moves[va[i]] << endl;
  // }
  vector<Location> pa;
  vector<Location> pb;
  v1i sId, sIndex, sAnt;
  // store
  for (size_t i = 0; i < va.size(); i++) {
    Location &a = ants[va[i]];
    Square &as = grid[a.row][a.col];
    Location b = getLocation(a, UDIRECTIONS[moves[va[i]]]);
    pa.push_back(a);
    pb.push_back(b);
    sId.push_back(as.id);
    sIndex.push_back(as.index);
    sAnt.push_back(as.ant);
  }
  for (size_t i = 0; i < va.size(); i++)
    if (antSquareAt(va[i])->isWarzone)
      adjustEnemyCount(pa[i], -1);
  // clear
  clearAnts(va);
  // write
  for (size_t i = 0; i < va.size(); i++) {
    ants[va[i]] = pb[i];
    moves[va[i]] = NOMOVE;
    antSquareAt(va[i])->setAnt(sId[i], sIndex[i], sAnt[i], false);
  }
  for (size_t i = 0; i < va.size(); i++)
    if (antSquareAt(va[i])->isWarzone)
      adjustEnemyCount(pb[i], 1);
  assert(assertEnemyCountsCorrect());
}

void State::printAnts(v1i &va) {
  for (size_t i = 0; i < va.size(); i++) {
    Location &a = ants[va[i]];
    Square &as = grid[a.row][a.col];
    bug << "ant " << i << " " << va[i] << " " << a << " " << as.ant
        << " " << moves[va[i]] << " " << CDIRECTIONS[moves[va[i]]] << endl;
  }
}

bool State::payoffWin(v1i &ants) {
  v1i dead(players, 0);
  updateDead(ants, dead);
  int d = 0;
  for (int i = 0; i < players; i++) {
    d += dead[i];
  }
  int u = d - (2*dead[0]);
  for (int i = 1; i < players; i++) {
    if ((d - (2*dead[i])) >= u)
      return false;
  }
  return true;
}

// how to check if an ant dies
// for every ant:
//   for each enemy in range of ant (using attackradius2):
//     if (enemies(of ant) in range of ant) >= (enemies(of enemy) in range of enemy) then
//       the ant is marked dead (actual removal is done after all battles are resolved)
//       break out of enemy loop

int State::assertAllAnts(v1i &is) {
  int nonAnts = 0;
  for (size_t i = 0; i < is.size(); i++) {
    Location &a = ants[is[i]];
    Square &as = grid[a.row][a.col];
    bool isAnt = as.ant >= 0;
    if (!isAnt)
      ++nonAnts;
    assert(isAnt);
  }
  return nonAnts;
}

Location State::addOffset(const Location &a, const Offset &o) {
  return Location(addWrap(a.row, o.r, rows), addWrap(a.col, o.c, cols));
}

// returns the euclidean distance between two locations with the edges wrapped
double State::distance(const Location &a, const Location &b) {
  int r = diffWrap(a.row, b.row, rows);
  int c = diffWrap(a.col, b.col, cols);
  return distanceGrid[r][c];
}

int State::distance2(const Location &a, const Location &b) {
  int r = diffWrap(a.row, b.row, rows);
  int c = diffWrap(a.col, b.col, cols);
  return distance2Grid[r][c];
}

// returns the euclidean distance between two locations with the edges wrapped
int State::manhattan(const Location &a, const Location &b) {
  int r = abs(a.row - b.row),
      c = abs(a.col - b.col),
      dr = min(r, rows - r),
      dc = min(c, cols - c);
  return dr + dc;
}

// returns the new location from moving in a given direction with the edges wrapped
Location State::getLocationNoWrap(const Location &loc, int direction) {
  return Location( (loc.row + DIRECTIONS[direction][0]),
                   (loc.col + DIRECTIONS[direction][1]));
}

// returns the new location from moving in a given direction with the edges wrapped
Location State::getLocation(const Location &loc, int direction) {
  return Location( (loc.row + DIRECTIONS[direction][0] + rows) % rows,
                   (loc.col + DIRECTIONS[direction][1] + cols) % cols );
}

Location State::getLocation(const Location &loc, const Location &off) {
  return Location( (loc.row + off.row + rows) % rows,
                   (loc.col + off.col + cols) % cols );
}

Location State::randomLocation() {
  return Location(rand() % rows, rand() % cols);
}

void State::markVisible(const Location& a) {
  if (!squareAt(a)->isKnown) {
    --nUnknown;
    ++nSeen;
  }
  if (!squareAt(a)->isVisible) {
    ++nVisible;
  }
  squareAt(a)->markVisible(turn);
}

void State::initDir() {
  dir = v2i(ants.size(), v1i(TDIRECTIONS, kDirichletAlpha));
}

void State::updateDir(int i) {
  vector<float> score(TDIRECTIONS, 0.0);
  for (int d = 0; d < TDIRECTIONS; d++) {
    // if (tryMove(i, d)) {
    //   score[d] = computeScore();
    //   undoMove(i);
    // }
  }
}

// SCORE
// proximity of my ants to food
// proximity of my ants to enemy hills
// proximity of enemy ants to food
// proximity of enemy ants to my hills
// proximity of my ants to each other depending on if there is food around
// proximity of my ants to my hill
// number of my ants that would die
// number of enemy ants that would die
// int State::score() {
// }

int State::pickRandomAnt() {
  return rand() % ants.size();
}

bool State::hasAntConsistency() {
  bug << "checking ant consistency " << turn << endl;
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      if (grid[r][c].ant == -1) {
        assert(grid[r][c].isCleared());
      }
      else if (grid[r][c].ant == 0) {
        assert(grid[r][c].index >= 0);
      }
      else {
        assert(grid[r][c].index >= 0);
      }
    }
  }
  return true;
}

void State::update() {
  assert(hasAntConsistency());
  updateVision();
  updateInfluence(10);
  assert(assertEnemyCountsCorrect());
  //initDir();
}

void State::updateVision() {
  for (vector<Location>::iterator a = ants.begin(); a != ants.end(); a++) {
    Square &as = grid[(*a).row][(*a).col];
    if (as.ant == 0) {
      for (vector<Offset>::iterator o = offsetSelf; o != viewEnd; o++) {
        markVisible(addOffset(*a, *o));
      }
      for (vector<Offset>::iterator o = offsetSelf; o != battleEnd; o++) {
        Location b = addOffset(*a, *o);
        Square &bs = grid[b.row][b.col];
        if (bs.ant > 0) {
          if (bs.battle == -1 && as.battle == -1) {
            ++battle;
            as.battle = bs.battle = battle;
          }
          else if (as.battle == -1) {
            as.battle = bs.battle;
          }
          else {
            bs.battle = as.battle;
          }
        }
      }
    }
    for (vector<Offset>::iterator o = offsetSelf; o != attackEnd; o++) {
      Location b = addOffset(*a, *o);
      Square &bs = grid[b.row][b.col];
      if (as.ant == 0)
        bs.good++;
      else
        bs.bad++;
      if (bs.ant != -1 && as.ant != bs.ant)
        ++as.enemies;
    }
  }
  for (vector<Location>::iterator a = ants.begin(); a != ants.end(); a++) {
    Square &as = grid[(*a).row][(*a).col];
    if (as.battle >= 0) {
      for (vector<Offset>::iterator o = offsetSelf; o != battleEnd; o++) {
        Location b = addOffset(*a, *o);
        Square &bs = grid[b.row][b.col];
        bs.isWarzone = true;
      }
    }
  }
  for (vector<Location>::iterator a = ants.begin(); a != ants.end(); a++) {
    Square &as = grid[(*a).row][(*a).col];
    if (as.battle >= 0) {
      for (vector<Offset>::iterator o = aoneOffsets.begin(); o != aoneOffsets.end(); o++) {
        Location b = addOffset(*a, *o);
        Square &bs = grid[b.row][b.col];
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
      if (as.isWarzone && as.sumAttacked > 0) {
        for (int i = 0; i < players; i++) {
          int best = as.sumAttacked;
          for (int j = 0; j < players; j++)
            if (j != i)
              if (as.fighting[j] < best)
                best = as.fighting[j];
          if (as.fighting[i] < best)
            as.status[i] = SAFE;
          else if (as.fighting[i] == best)
            as.status[i] = KILL;
          else
            as.status[i] = DIE;
        }
      }
    }
  }
}

void State::updateDead(v1i &is, v1i &dead) {
  assertAllAnts(is);
  for (size_t i = 0; i < is.size(); i++) {
    Location &a = ants[is[i]];
    Square &as = grid[a.row][a.col];
    for (vector<Offset>::iterator o = offsetSelf; o != attackEnd; o++) {
      Location b = addOffset(a, *o);
      Square &bs = grid[b.row][b.col];
      if (bs.ant != -1 && as.ant != bs.ant) {
        if (as.enemies >= bs.enemies) {
          dead[as.ant] += 1;
          break;
        }
      }
    }
  }
}

void State::updateInfluence(int iterations) {
  vector< vector< vector<float> > > temp(rows, vector< vector<float> >(cols, vector<float>(kFactors, 0.0)));
  for (int i = 0; i < iterations; i++) {
    for (int r = 0; r < rows; r++) {
      for (int c = 0; c < cols; c++) {
        Location a(r, c);
        Square &as = grid[a.row][a.col];
        temp[r][c][VISIBLE] = as.inf[VISIBLE];
        vector<float> nsumu(kFactors, 0.0);
        for (int d = 0; d < TDIRECTIONS; d++) {
          Location b = getLocation(a, d);
          Square &bs = grid[b.row][b.col];
          for (int f = 0; f < kFactors; f++)
            nsumu[f] += bs.inf[f] - as.inf[f];
        }
        for (int f = 0; f < kFactors; f++) {
          float k = 0.0;
          if (as.isWater) {
            k = 0.0;
          }
          else if (as.isFood) {
            k = (f == FOOD) ? 1.0 : as.inf[f] + (decay[f] * nsumu[f]);
          }
          else if (as.hillPlayer == 0) {
            k = 0.0;
            if (as.ant == 0 && f == FRIEND)
              k = 1.0;
          }
          else if (as.hillPlayer > 0) {
            k = (f == TARGET) ? 1.0 : as.inf[f] + (decay[f] * nsumu[f]);
            if (as.ant > 0 && f == ENEMY)
              k = 1.0;
          }
          else if (as.ant == 0) {
            k = (f == FRIEND) ? 1.0 : 1.0 * (as.inf[f] + (decay[f] * nsumu[f]));
          }
          else if (as.ant > 0) {
            k = (f == ENEMY) ? 1.0 : 1.0 * (as.inf[f] + (decay[f] * nsumu[f]));
          }
          else if (!as.isKnown) {
            k = (f == UNKNOWN) ? 1.0 : as.inf[f] + (decay[f] * nsumu[f]);
          }
          else {
            k = as.inf[f] + (decay[f] * nsumu[f]);
          }
          temp[r][c][f] = k;
        }
      }
    }
    for (int r = 0; r < rows; r++) {
      for (int c = 0; c < cols; c++) {
        for (int f = 0; f < kFactors; f++) {
          grid[r][c].inf[f] = temp[r][c][f];
        }
      }
    }
  }
}

// This is the output function for a state. It will add a char map
// representation of the state to the output stream passed to it.
ostream& operator<<(ostream &os, const State &state) {
  for (int row = 0; row < state.rows; row++) {
    for (int col = 0; col < state.cols; col++) {
      if (state.grid[row][col].isWater)
        os << '%';
      else if (state.grid[row][col].isFood)
        os << '*';
      else if (state.grid[row][col].isHill)
        os << (char)('A' + state.grid[row][col].hillPlayer);
      else if (state.grid[row][col].ant >= 0)
        os << (char)('a' + state.grid[row][col].ant);
      else if (state.grid[row][col].isVisible)
        os << '.';
      else
        os << '?';
    }
    os << endl;
  }
  return os;
}

void State::updatePlayers(int player) {
  if (player > players)
    players = player;
}

void State::putWater(int row, int col) {
  grid[row][col].inf[LAND] = 0.0;
  grid[row][col].isWater = 1;
}

void State::putFood(int row, int col) {
  grid[row][col].isFood2 = 1;
  food.push_back(Location(row, col));
}

void State::putHill(int row, int col, int player) {
  updatePlayers(player);
  grid[row][col].isHill2 = 1;
  grid[row][col].hillPlayer2 = player;
  hills.push_back(Location(row, col));
}

void State::putDead(int row, int col, int player) {
  updatePlayers(player);
  if (player == 0)
    grid[row][col].id = -1;
  grid[row][col].deadAnts.push_back(player);
}

void State::putAnt(int row, int col, int player) {
  updatePlayers(player);
  Location a(row, col);
  Square &as = grid[row][col];
  as.ant = player;
  as.index = ants.size();
  assert(as.isUsed == false);
  moveFrom.push_back(a);
  moves.push_back(NOMOVE);
  ants.push_back(a);
}

//input function
istream& operator>>(istream &is, State &state) {
  int row, col, player;
  string inputType, junk;
  //finds out which turn it is
  while(is >> inputType) {
    if(inputType == "end") {
      state.gameover = 1;
      break;
    }
    else if(inputType == "turn") {
      is >> state.turn;
      break;
    }
    else { //unknown line
      getline(is, junk);
    }
  }
  if (state.turn == 0) {
    //reads game parameters
    while (is >> inputType) {
      if (inputType == "loadtime")
        is >> state.loadtime;
      else if (inputType == "turntime")
        is >> state.turntime;
      else if (inputType == "rows")
        is >> state.rows;
      else if (inputType == "cols")
        is >> state.cols;
      else if (inputType == "turns")
        is >> state.turns;
      else if (inputType == "player_seed")
        is >> state.seed;
      else if (inputType == "viewradius2") {
        is >> state.viewradius2;
        state.viewradius = sqrt(state.viewradius2);
      } 
      else if (inputType == "attackradius2") {
        is >> state.attackradius2;
        state.attackradius = sqrt(state.attackradius2);
      } 
      else if (inputType == "spawnradius2") {
        is >> state.spawnradius2;
        state.spawnradius = sqrt(state.spawnradius2);
      }
      else if (inputType == "ready") {
        state.timer.start();
        break;
      }
      else    //unknown line
        getline(is, junk);
    }
  }
  else {
    //reads information about the current turn
    while (is >> inputType) {
      if(inputType == "w") { //water square
        is >> row >> col;
        state.putWater(row, col);
      }
      else if (inputType == "f") { //food square
        is >> row >> col;
        state.putFood(row, col);
      }
      else if (inputType == "a") { //live ant square
        is >> row >> col >> player;
        state.putAnt(row, col, player);
      }
      else if (inputType == "d") { //dead ant square
        is >> row >> col >> player;
        state.putDead(row, col, player);
      }
      else if (inputType == "h") {
        is >> row >> col >> player;
        state.putHill(row, col, player);
      }
      else if (inputType == "players") { //player information
        is >> state.players;
      }
      else if (inputType == "scores") { //score information
        state.scores = vector<double>(state.players, 0.0);
        for (int p=0; p<state.players; p++)
          is >> state.scores[p];
      }
      else if (inputType == "go") { //end of turn input
        if (state.gameover)
          is.setstate(ios::failbit);
        else
          state.timer.start();
        break;
      }
      else { //unknown line
        getline(is, junk);
      }
    }
  }
  return is;
}

ostream& operator<<(ostream& os, const Square &square) {
  if (square.isVisible)
    os << "V";
  if (square.isWater)
    os << "W";
  if (square.isHill)
    os << "H";
  if (square.isFood)
    os << "F";
  if (square.ant >= 0)
    os << "," << square.ant;
  if (square.hillPlayer >= 0)
    os << "," << square.hillPlayer;
  return os;
}

bool operator<(const Location &a, const Location &b) {
  return a.col == b.col ? a.row < b.row : a.col < b.col;
}

bool operator==(const Location &a, const Location &b) {
  return a.row == b.row && a.col == b.col;
}

bool operator!=(const Location &a, const Location &b) {
  return a.row != b.row || a.col != b.col;
}

ostream& operator<<(ostream &os, const Location &loc) {
  return os << "(" << (int)loc.row << "," << (int)loc.col << ")";
}

ostream& operator<<(ostream& os, const Offset &o) {
  return os << o.d2 << " " << o.r << "," << o.c;
}

bool operator<(const Offset &a, const Offset &b) {
  return a.d2 == b.d2
      ? (a.r == b.r ? a.c < b.c : a.r < b.r)
      : (a.d2 < b.d2);
}

bool operator==(const Offset &a, const Offset &b) {
  return a.r == b.r && a.c == b.c && a.d2 == b.d2;
}

bool operator!=(const Offset &a, const Offset &b) {
  return a.r != b.r || a.c != b.c || a.d2 != b.d2;
}


