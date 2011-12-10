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
  nSquares = nUnknown = rows * cols;
  nSeen = nVisible = 0;
  battle = -1;
  nextAntId = 0;
  grid = vector< vector<Square> >(rows, vector<Square>(cols, Square()));
  int n = max(rows, cols);
  distanceGrid = v2i(n, v1i(n, 0));
  battleradius = pow(sqrt(attackradius) + 2.0, 2.0);
  weights = v1f(kFactors, 0.0);
  decay = v1f(kFactors, 0.0);
  loss = v1f(kFactors, 0.0);
  setInfluenceParameter(VISIBLE,  0.00,  0.00,  0.90);
  setInfluenceParameter(LAND,     0.00,  0.10,  1.00);
  setInfluenceParameter(FOOD,     0.80,  0.12,  1.00);
  setInfluenceParameter(TARGET,   1.00,  0.20,  1.00);
  setInfluenceParameter(UNKNOWN,  0.40,  0.05,  1.00);
  setInfluenceParameter(ENEMY,    0.50,  0.10,  1.00);
  setInfluenceParameter(FRIEND,   0.00,  0.01,  1.00);

  // build distance lookup tables
  for (int r = 0; r < n; r++) {
    for (int c = 0; c < n; c++) {
      int d = r * r + c * c;
      distanceGrid[r][c] = d;
    }
  }
  // build offset list
  for (int r = (1 - n); r < n; r++) {
    for (int c = (1 - n); c < n; c++) {
      int ar = abs(r), ac = abs(c);
      offsets.push_back(Offset(distanceGrid[ar][ac], r, c));
    }
  }
  sort(offsets.begin(), offsets.end());
  // capture key offsets
  vector<Offset>::iterator offset = offsets.begin();
  offsetSelf = offset++;
  offsetFirst = offset++;
  for (; (offset < offsets.end()) && (*offset).d2 <= spawnradius; offset++);
  spawnEnd = offset;
  for (; (offset < offsets.end()) && (*offset).d2 <= attackradius; offset++);
  attackEnd = offset;
  for (; (offset < offsets.end()) && (*offset).d2 <= battleradius; offset++);
  battleEnd = offset;
  for (; (offset < offsets.end()) && (*offset).d2 <= viewradius; offset++);
  viewEnd = offset;
  // special attack offsets
  for (vector<Offset>::iterator oi = offsetSelf; oi < attackEnd; oi++) {
    Offset &o = *oi;
    for (int d = 0; d < TDIRECTIONS; d++) {
      int r = o.r + DIRECTIONS[d][0];
      int c = o.c + DIRECTIONS[d][1];
      Offset b(distanceGrid[abs(r)][abs(c)], r, c);
      if (find(aoneOffsets.begin(), aoneOffsets.end(), b) == aoneOffsets.end())
        aoneOffsets.push_back(b);
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

int State::countEnemies(const Loc &a) {
  int n = 0;
  Square &as = grid[a.r][a.c];
  assert(as.ant != -1);
  for (vector<Offset>::iterator o = offsetSelf; o != attackEnd; o++) {
    Loc b = addOffset(a, *o);
    Square &bs = grid[b.r][b.c];
    if (bs.ant != -1 && as.ant != bs.ant)
      ++n;
  }
  return n;
}

int State::assertEnemyCountsCorrect() {
  for (size_t i = 0; i < ants.size(); i++) {
    Loc a = ants[i];
    Square &as = grid[a.r][a.c];
    assert(as.enemies == countEnemies(a));
  }
  return 1;
}

void State::adjustEnemyCount(const Loc &a, int i) {
  Square &as = grid[a.r][a.c];
  assert(as.ant != -1);
  for (vector<Offset>::iterator o = offsetSelf; o != attackEnd; o++) {
    Loc b = addOffset(a, *o);
    Square &bs = grid[b.r][b.c];
    if (bs.ant != -1 && as.ant != bs.ant) {
      as.enemies += i;
      bs.enemies += i;
    }
  }
}

void State::makeMove(int i, int d) {
  Loc a = ants[i];
  Square &as = grid[a.r][a.c];
  if (d != NOMOVE) {
    if (as.isWarzone >= 0)
      adjustEnemyCount(a, -1);
    Loc b = getLoc(a, d);
    Square &bs = grid[b.r][b.c];
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
    grid[a.r][a.c].isUsed = true;
  }
}

void State::undoMove(int i) {
  const Loc a = ants[i];
  Square &as = grid[a.r][a.c];
  int d = moves[i];
  if (d != NOMOVE) {
    if (as.isWarzone >= 0)
      adjustEnemyCount(a, -1);
    d = UDIRECTIONS[d];
    Loc b = getLoc(a, d);
    Square &bs = grid[b.r][b.c];
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
  vector<Loc> pa;
  vector<Loc> pb;
  v1i sId, sIndex, sAnt;
  vector< vector<bool> > used(rows, vector<bool>(cols, false));
  // store
  for (size_t i = 0; i < va.size(); i++) {
    Loc &a = ants[va[i]];
    Square &as = grid[a.r][a.c];
    Loc b = getLoc(a, vm[i]);
    Square &bs = grid[b.r][b.c];
    assert(!as.isUsed);
    if (bs.isWater || bs.isFood || bs.isUsed || used[b.r][b.c])
      return false;
    // prevent stomping on an ant that is not going to be otherwise moved in this set?
    if (!bs.isCleared() && find(va.begin(), va.end(), bs.index) == va.end())
      return false;
    used[b.r][b.c] = true;
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
  vector<Loc> pa;
  vector<Loc> pb;
  v1i sId, sIndex, sAnt;
  // store
  for (size_t i = 0; i < va.size(); i++) {
    Loc &a = ants[va[i]];
    Square &as = grid[a.r][a.c];
    Loc b = getLoc(a, UDIRECTIONS[moves[va[i]]]);
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
    Loc &a = ants[va[i]];
    Square &as = grid[a.r][a.c];
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
    Loc &a = ants[is[i]];
    Square &as = grid[a.r][a.c];
    bool isAnt = as.ant >= 0;
    if (!isAnt)
      ++nonAnts;
    assert(isAnt);
  }
  return nonAnts;
}

Loc State::addOffset(const Loc &a, const Offset &o) {
  return Loc(add(a.r, o.r, rows), add(a.c, o.c, cols));
}

int State::distance(const Loc &a, const Loc &b) {
  return distanceGrid[delta(a.r, b.r, rows)][delta(a.c, b.c, cols)];
}

// returns the new location from moving in a given direction with the edges wrapped
Loc State::getLocNoWrap(const Loc &a, int d) {
  return Loc(a.r + DIRECTIONS[d][0], a.c + DIRECTIONS[d][1]);
}

// returns the new location from moving in a given direction with the edges wrapped
Loc State::getLoc(const Loc &a, int d) {
  return Loc(add(a.r, DIRECTIONS[d][0], rows), add(a.c, DIRECTIONS[d][1], cols));
}

Loc State::getLoc(const Loc &a, const Loc &o) {
  return Loc(add(a.r, o.r, rows), add(a.c, o.c, cols));
}

Loc State::randomLoc() {
  return Loc(rand() % rows, rand() % cols);
}

void State::markVisible(const Loc& a) {
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
  updateInfluence(20);
  assert(assertEnemyCountsCorrect());
  //initDir();
}

void State::updateVision() {
  for (vector<Loc>::iterator a = ants.begin(); a != ants.end(); a++) {
    Square &as = grid[(*a).r][(*a).c];
    if (as.ant == 0) {
      for (vector<Offset>::iterator o = offsetSelf; o != viewEnd; o++) {
        markVisible(addOffset(*a, *o));
      }
      for (vector<Offset>::iterator o = offsetSelf; o != battleEnd; o++) {
        Loc b = addOffset(*a, *o);
        Square &bs = grid[b.r][b.c];
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
      Loc b = addOffset(*a, *o);
      Square &bs = grid[b.r][b.c];
      if (as.ant == 0)
        bs.good++;
      else
        bs.bad++;
      if (bs.ant != -1 && as.ant != bs.ant)
        ++as.enemies;
    }
  }
  for (vector<Loc>::iterator a = ants.begin(); a != ants.end(); a++) {
    Square &as = grid[(*a).r][(*a).c];
    if (as.battle >= 0) {
      for (vector<Offset>::iterator o = offsetSelf; o != battleEnd; o++) {
        Loc b = addOffset(*a, *o);
        Square &bs = grid[b.r][b.c];
        bs.isWarzone = true;
      }
    }
  }
  vector< vector< vector<bool> > > used(rows, vector< vector<bool> >(cols, vector<bool>(players, false)));
  for (vector<Loc>::iterator ai = ants.begin(); ai != ants.end(); ai++) {
    Loc &a = *ai;
    Square &as = grid[a.r][a.c];
    if (as.battle >= 0) {
      if (!used[a.r][a.c][as.ant]) {
        used[a.r][a.c][as.ant] = true;
        for (vector<Offset>::iterator o = aoneOffsets.begin(); o != aoneOffsets.end(); o++) {
          Loc b = addOffset(a, *o);
          Square &bs = grid[b.r][b.c];
          bs.attacked[as.ant]++;
          bs.sumAttacked++;
          for (int i = 0; i < players; i++)
            if (i != as.ant)
              (bs.fighting[i])++;
        }
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
    Loc &a = ants[is[i]];
    Square &as = grid[a.r][a.c];
    for (vector<Offset>::iterator o = offsetSelf; o != attackEnd; o++) {
      Loc b = addOffset(a, *o);
      Square &bs = grid[b.r][b.c];
      if (bs.ant != -1 && as.ant != bs.ant) {
        if (as.enemies >= bs.enemies) {
          dead[as.ant] += 1;
          break;
        }
      }
    }
  }
}

float Square::coefficient(int f) {
  return ant == 0 ? 0.9
      :  ant >  0 ? 1.0
      :  1.0;
}

bool Square::canDiffuse() {
  return !isWater && hillPlayer != 0;
}

float Square::isSource(int f) {
  return (isFood && f == FOOD)
      || (hillPlayer > 0 && f == TARGET)
      || (ant > 0 && f == ENEMY)
      || (ant == 0 && f == FRIEND)
      || (!isKnown && f == UNKNOWN);
}

void State::computeInfluenceBlend(v3f &temp) {
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
          temp[r][c][f] = as.isSource(f) ? 1.0
              : as.coefficient(f) * (as.inf[f] + (decay[f] * nsumu[f]));
      }
      else {
        for (int f = 0; f < kFactors; f++)
          temp[r][c][f] = 0.0;
      }
    }
  }
}

void State::computeInfluenceLinear(v3f &temp) {
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      Loc a(r, c);
      Square &as = grid[a.r][a.c];
      if (as.canDiffuse()) {
        vector<float> hmax(kFactors, 0.0);
        for (int d = 0; d < TDIRECTIONS; d++) {
          Loc b = getLoc(a, d);
          Square &bs = grid[b.r][b.c];
          for (int f = 0; f < kFactors; f++) {
            float h = 0.90 * bs.inf[f];
            if (h > hmax[f])
              hmax[f] = h;
          }
        }
        for (int f = 0; f < kFactors; f++)
          temp[r][c][f] = as.isSource(f) ? 1.0
              : hmax[f];
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
        grid[r][c].inf[f] = temp[r][c][f];
}

void State::updateInfluence(int iterations) {
  static v3f temp(rows, v2f(cols, v1f(kFactors, 0.0)));
  for (int i = 0; i < iterations; i++) {
    computeInfluenceBlend(temp);
    writeInfluence(temp);
  }
}

// This is the output function for a state. It will add a char map
// representation of the state to the output stream passed to it.
ostream& operator<<(ostream &os, const State &state) {
  for (int r = 0; r < state.rows; r++) {
    for (int c = 0; c < state.cols; c++) {
      if (state.grid[r][c].isWater)
        os << '%';
      else if (state.grid[r][c].isFood)
        os << '*';
      else if (state.grid[r][c].isHill)
        os << (char)('A' + state.grid[r][c].hillPlayer);
      else if (state.grid[r][c].ant >= 0)
        os << (char)('a' + state.grid[r][c].ant);
      else if (state.grid[r][c].isVisible)
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

void State::putWater(int r, int c) {
  grid[r][c].inf[LAND] = 0.0;
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
  if (player == 0)
    grid[r][c].id = -1;
  grid[r][c].deadAnts.push_back(player);
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

//input function
istream& operator>>(istream &is, State &state) {
  int r, c, p;
  string type, junk;
  //finds out which turn it is
  while (is >> type) {
    if (type == "end") {
      state.gameover = 1;
      break;
    }
    else if (type == "turn") {
      is >> state.turn;
      break;
    }
    else {
      getline(is, junk);
    }
  }
  if (state.turn == 0) {
    while (is >> type) {
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
    }
  }
  else {
    while (is >> type) {
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


