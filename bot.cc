#include "bot.h"

int dleft[] = { WEST, NORTH, EAST, SOUTH };
int dright[] = { EAST, SOUTH, WEST, NORTH };
int dbehind[] = { SOUTH, WEST, NORTH, EAST };

// constructor
Bot::Bot() : nextAntId(0) {};
Bot::Bot(int rows, int cols) : nextAntId(0), state(rows, cols) {};

// plays a single game of Ants.
void Bot::playGame() {
  // reads the game parameters and sets up
  cin >> state;
  state.setup();
  setup();
  endTurn();

  // continues making moves while the game is not over
  while (cin >> state) {
    state.updateVisionInformation();
    makeMoves();
    endTurn();
  }
};

void Bot::setup() {
  // determine all currently unseen tiles
  for (int row = 0; row < state.rows; row += 4)
    for (int col = 0; col < state.cols; col += 4)
      unseen.insert(Location(row, col));
}

// finishes the turn
void Bot::endTurn() {
  if(state.turn > 0)
    state.reset();
  state.turn++;
  cout << "go" << endl;
}

bool Bot::doMoveDirection(const Location &a, int d) {
  Location b = state.getLocation(a, d);
  if (!state[b].isWater && !orders.count(b)) {
    orders.insert(b);
    state.makeMove(a, d);
    straight[a] = d;
    return true;
  }
  else {
    return false;
  }
}

bool Bot::doMoveLocation(const Location &a, const Location &b) {
  vector<int> ds = state.getDirections(a, b);
  for (vector<int>::iterator dp = ds.begin(); dp != ds.end(); dp++) {
    if (doMoveDirection(a, *dp)) {
      return true;
    }
  }
  return false;
}

bool routeCmp(const Route &a, const Route &b) {
  return a.distance < b.distance;
}

bool Bot::doMoveRoute(Route &r, set<Location> &antsUsed) {
  if (r.steps.size() > 1) {
    const Location &ant = r.steps.front();
    if (!antsUsed.count(ant) && doMoveLocation(ant, r.steps[1])) {
      antsUsed.insert(r.steps.front());
      r.steps.pop_front();
      r.distance--;
    }
  }
}

bool Bot::doMoveRoutes(list< Route >& routes, set<Location> &antsUsed,
                       set<Location> targets) {
  for (list< Route >::iterator r = routes.begin(); r != routes.end(); r++) {
    if (targets.count((*r).steps.back())) {
      if ((*r).steps.size() > 1) {
        Location &ant = (*r).steps.front();
        if (!antsUsed.count(ant) && doMoveLocation(ant, (*r).steps[1])) {
          antsUsed.insert((*r).steps.front());
          (*r).steps.pop_front();
          (*r).distance--;
        }
      }
    }
  }
}

inline bool isVisibleAndNotHill(Square &s) { return s.isVisible && !s.isHill; }
inline bool isVisibleAndNotFood(Square &s) { return s.isVisible && !s.isFood; }
inline bool isVisibleAndNotEnemy(Square &s) { return s.isVisible && s.ant <= 0; }
inline bool isVisibleAndNotMyAnt(Square &s) { return s.isVisible && s.ant != 0; }
inline bool isVisible(Square &s) { return s.isVisible; }

void Bot::insertAll(set<Location> &to, vector<Location> &from) {
  to.insert(from.begin(), from.end());
}

void Bot::removeIf(set<Location> &locs, bool(*pred)(Square &)) {
  for (set<Location>::iterator p = locs.begin(); p != locs.end();)
    if (pred(state.grid[(*p).row][(*p).col]))
      locs.erase(*p++);
    else
      ++p;
}

void Bot::updateMemory(set<Location> &memory, vector<Location> &seen,
                       bool(*pred)(Square &)) {
  insertAll(memory, seen);
  removeIf(memory, pred);
}

void Bot::search(set<Location> &sources, set<Location> &targets, list<Route> &routes) {
  set<Location> unassignedAnts(sources.begin(), sources.end());
  set<Location> remainingTargets(targets.begin(), targets.end());
  set<Location> expanded;
  deque<Location> searchQueue(unassignedAnts.begin(), unassignedAnts.end());
  map<Location, Search> searches;
  for (set<Location>::iterator p = unassignedAnts.begin(); p != unassignedAnts.end(); p++) {
    searches[*p] = Search(*p);
  }
  while (!searchQueue.empty()) {
    Location& antLoc = searchQueue.front();
    Search &search = searches[antLoc];
    if (!search.remaining.empty()) {
      Location& u = search.remaining.front();
      if (remainingTargets.count(u)) {
        routes.push_back(Route(antLoc, u, search.predecessors));
        remainingTargets.erase(u);
        unassignedAnts.erase(antLoc);
      }
      for (int d = 0; d < TDIRECTIONS; d++) {
        Location v = state.getLocation(u, d);
        if (!state.grid[v.row][v.col].isWater) {
          if (!expanded.count(v)) {
            int distance = search.distances[u] + 1;
            expanded.insert(v);
            search.distances[v] = distance;
            search.predecessors[v] = u;
            search.remaining.push(v);
          }
        }
      }
      search.remaining.pop();
      if (!search.remaining.empty()) {
        if (!unassignedAnts.empty() && !remainingTargets.empty()) {
          searchQueue.push_back(antLoc);
        }
      }
      searchQueue.pop_front();
    }
  }
}


int Bot::search(set<Location> &sources, const Location &target, list<Route> &routes, int maxCount) {
  int count = 0;
  int expansionCount = 0;
  set<Location> expanded;
  queue<Location> remaining;
  map<Location,Location> predecessors;
  map<Location,int> distances;
  remaining.push(target);
  while (!remaining.empty()) {
    Location& u = remaining.front();
    if (sources.count(u)) {
      Route r = Route(target, u, predecessors);
      r.flip();
      routes.push_back(r);
      count++;
      if (count >= maxCount) {
        state.bug << "expansion count " << expansionCount << " count " << count << endl;
        return count;
      }
    }
    for (int d = 0; d < TDIRECTIONS; d++) {
      Location v = state.getLocation(u, d);
      if (!state[v].isWater) {
        if (!expanded.count(v)) {
          int distance = distances[u] + 1;
          expansionCount++;
          expanded.insert(v);
          distances[v] = distance;
          predecessors[v] = u;
          remaining.push(v);
        }
      }
    }
    remaining.pop();
  }
  state.bug << "expansion count " << expansionCount << " count " << count << endl;
  return count;
}

// Find a route from start to the first end it comes across.
bool Bot::search(Location &start, set<Location> &ends, Route &route) {
  map<Location,int> distances;
  map<Location,Location> predecessors;
  set<Location> expanded;
  queue<Location> remaining;
  remaining.push(start);
  while (!remaining.empty()) {
    Location& u = remaining.front();
    if (ends.count(u)) {
      route.start = start;
      route.end = u;
      route.distance = distances[u];
      return true;
    }
    for (int d = 0; d < TDIRECTIONS; d++) {
      Location v = state.getLocation(u, d);
      if (!state.grid[v.row][v.col].isWater) {
        if (!expanded.count(v)) {
          expanded.insert(v);
          distances[v] = distances[u] + 1;
          predecessors[v] = u;
          remaining.push(v);
        }
      }
    }
    remaining.pop();
  }
  return false;
}

// Find a route from start to the first end it comes across.
bool Bot::search(Location &start, Location &goal, Route &route) {
  map<Location,int> f, g, h;
  map<Location,Location> p; // came from
  set<Location> X; // closed
  Fcmp fcmp(&f);
  priority_queue<Location,vector<Location>,Fcmp> Q(fcmp); // open
  g[start] = 0;
  h[start] = state.manhattan(start, goal);
  f[start] = g[start] + h[start];
  Q.push(start);
  while (!Q.empty()) {
    Location u = Q.top();
    if (u == goal) {
      route = Route(start, goal, p);
      return true;
    }
    X.insert(u);
    Q.pop();
    for (int d = 0; d < TDIRECTIONS; d++) {
      Location v = state.getLocation(u, d);
      if (!state.grid[v.row][v.col].isWater) {
        if (!X.count(v)) {
          if (!g.count(v) || ((g[v] + 1) < g[v])) {
            p[v] = u;
            g[v] = g[v] + 1;
            h[v] = state.manhattan(v, goal);
            f[v] = g[v] + h[v];
            Q.push(v);
          }
        }
      }
    }
  }
  return false;
}

int Bot::numAttackAnts(int totalAnts) {
  return min((double)totalAnts / 4.0, 7.0);
}

// Makes the ants moves for the turn.
void Bot::makeMoves() {
  orders.clear();

  // keep ants from moving onto our own hills and preventing spawning
  insertAll(orders, state.myHills);
    
  state.bug << "turn " << state.turn << ":" << endl;
  state.bug << state << endl;
  state.bug << "unseen " << unseen.size() << endl;

  updateMemory(enemyHills, state.enemyHills, isVisibleAndNotHill);
  updateMemory(food, state.food, isVisibleAndNotFood);
  updateMemory(enemyAnts, state.enemyAnts, isVisibleAndNotEnemy);
  updateMemory(myAnts, state.myAnts, isVisibleAndNotMyAnt);
  updateMemory(myHills, state.myHills, isVisibleAndNotHill);
  removeIf(unseen, isVisible);

  // determine ranges from my ants to enemy ants
  vector<Manhattan> ranges;
  for (set<Location>::iterator a = myAnts.begin(); a != myAnts.end(); a++)
    for (set<Location>::iterator b = enemyAnts.begin(); b != enemyAnts.end(); b++)
      ranges.push_back(Manhattan(*a, *b, state.distance2(*a, *b)));
  state.bug << "range count " << ranges.size() << endl;
  sort(ranges.begin(), ranges.end());

  int pauseradius2 = (int) pow((state.attackradius + 1.0), 2);
  set<Location> antsUsed;
  xroutes.clear();
  for (set<Location>::iterator p = enemyHills.begin(); p != enemyHills.end(); p++)
    search(myAnts, *p, xroutes, numAttackAnts(myAnts.size()));
  search(myAnts, food, xroutes);
  xroutes.sort(routeCmp);
  doMoveRoutes(xroutes, antsUsed, enemyHills);
  doMoveRoutes(xroutes, antsUsed, food);

  goLefty(antsUsed);
  state.bug << "unknown "
            << state.turn << " "
            << state.myAnts.size() << " "
            << state.nVisible << " "
            << state.nSeen << " "
            << state.nUnknown << " "
            << state.nSquares << " "
            << ((double)state.nVisible / (double)state.nSquares) << " "
            << ((double)state.nSeen / (double)state.nSquares) << " " 
            << ((double)state.nUnknown / (double)state.nSquares) << endl;
  state.bug << "time taken: " << state.timer.getTime() << endl << endl;
}

// General left favoring wall exploring algorithm.
void Bot::goLefty(set<Location> &antsUsed) {
  set<Location> destinations;
  map<Location,int> newStraight;
  map<Location,int> newLefty;
  for (vector<Location>::iterator p = state.myAnts.begin(); p != state.myAnts.end(); p++) {
    if (antsUsed.count(*p))
      continue;
    if (!straight.count(*p) && !lefty.count(*p))
      //straight[*p] = ((*p).row % 2 == 0) ?
      //   (((*p).col % 2 == 0) ? NORTH : SOUTH) :
      //   (((*p).col % 2 == 0) ? EAST : WEST);
      straight[*p] = rand() % 4;
    if (straight.count(*p)) {
      int d = straight[*p];
      Location dest = state.getLocation(*p, d);
      Square &square = state[dest];
      if (!square.isWater && !square.hillPlayer == 0) {
        if (square.ant == -1 && !destinations.count(dest)) {
          state.makeMove(*p, d);
          newStraight[dest] = d;
          destinations.insert(dest);
        }
        else {
          newStraight[*p] = dleft[d];
          destinations.insert(*p);
        }
      }
      else {
        lefty[*p] = dright[d];
      }
    }
    if (lefty.count(*p)) {
      int d = lefty[*p];
      int ds[] = { dleft[d], d, dright[d], dbehind[d] };
      for (int i = 0; i < 4; i++) {
        int nd = ds[i];
        Location dest = state.getLocation(*p, nd);
        Square &square = state[dest];
        if (!square.isWater && !square.hillPlayer == 0) {
          if (square.ant == -1 && !destinations.count(dest)) {
            state.makeMove(*p, nd);
            newLefty[dest] = nd;
            destinations.insert(dest);
            break;
          }
          else {
            newStraight[*p] = dright[d];
            destinations.insert(*p);
            break;
          }
        }
      }
    }
  }
  straight = newStraight;
  lefty = newLefty;
}


Search::Search(const Location &start) : start(start), food(0), hills(0), unseen(0) {
  expanded.insert(start);
  distances[start] = 0;
  predecessors[start] = start;
  remaining.push(start);
}

int Search::distance(const Location& dest) {
  return distances[dest];
}

Location Search::step(const Location& dest) {
  Location loc = dest;
  while (predecessors.count(loc) && predecessors[loc] != start) {
    loc = predecessors[loc];
  }
  return loc;
}

bool operator<(const Manhattan &a, const Manhattan &b) {
  return a.d < b.d;
}
