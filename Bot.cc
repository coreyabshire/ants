#include "Bot.h"

int dleft[] = { WEST, NORTH, EAST, SOUTH };
int dright[] = { EAST, SOUTH, WEST, NORTH };
int dbehind[] = { SOUTH, WEST, NORTH, EAST };

// constructor
Bot::Bot() : nextAntId(0) {};

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
  for (int row = 0; row < state.rows; row += 4) {
    for (int col = 0; col < state.cols; col += 4) {
      unseen.insert(Location(row, col));
    }
  }
}

// finishes the turn
void Bot::endTurn() {
  if(state.turn > 0)
    state.reset();
  state.turn++;
  cout << "go" << endl;
}

bool Bot::doMoveDirection(const Location &antLoc, int d) {
  Location newLoc = state.getLocation(antLoc, d);
  Square& square = state.grid[newLoc.row][newLoc.col];
  if (!square.isWater && !orders.count(newLoc)) {
    orders.insert(newLoc);
    state.makeMove(antLoc, d);
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

void Bot::search(set<Location> &sources, set<Location> &targets, list< Route > &routes) {
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

void Bot::search(set<Location> &sources, set<Location> &targets, list< Route > &routes, int maxCount) {
  set<Location> unassignedAnts(sources.begin(), sources.end());
  set<Location> remainingTargets(targets.begin(), targets.end());
  map<Location,int> targetCounts;
  set<Location> expanded;
  deque<Location> searchQueue(unassignedAnts.begin(), unassignedAnts.end());
  map<Location, Search> searches;
  for (set<Location>::iterator p = unassignedAnts.begin(); p != unassignedAnts.end(); p++) {
    searches[*p] = Search(*p);
  }
  for (set<Location>::iterator p = remainingTargets.begin(); p != remainingTargets.end(); p++) {
    targetCounts[*p] = 0;
  }
  while (!searchQueue.empty()) {
    Location& antLoc = searchQueue.front();
    Search &search = searches[antLoc];
    if (!search.remaining.empty()) {
      Location& u = search.remaining.front();
      if (remainingTargets.count(u)) {
        routes.push_back(Route(antLoc, u, search.predecessors));
        targetCounts[u]++;
        if (targetCounts[u] >= maxCount) {
          remainingTargets.erase(u);
        }
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

void Bot::search(set<Location> &sources, list< Route > &routes) {
  set<Location> unassignedAnts(sources.begin(), sources.end());
  set<Location> remainingFood(food.begin(), food.end());
  set<Location> remainingHills(enemyHills.begin(), enemyHills.end());
  set<Location> remainingUnseen(unseen.begin(), unseen.end());

  for (list< Route >::iterator r = routes.begin(); r != routes.end(); r++) {
    Location a = (*r).steps.front();
    Location b = (*r).steps.back();
    state.bug << "processing route " << a << " to " << b << endl;
    if (remainingFood.count(b)) {
      state.bug << "processing route to remove food" << endl;
      unassignedAnts.erase(a);
      remainingFood.erase(b);
    }
    else if (remainingHills.count(b)) {
      state.bug << "processing route to remove hill" << endl;
      unassignedAnts.erase(a);
      remainingHills.erase(b);
    }
    else if (remainingUnseen.count(b)) {
      state.bug << "processing route to remove unseen" << endl;
      unassignedAnts.erase(a);
      remainingUnseen.erase(b);
    }
    else {
      state.bug << "processing route should not get here" << endl;
    }
    state.bug << "processing route " << a << " to " << b << " complete" << endl;
  }

  deque<Location> searchQueue(unassignedAnts.begin(), unassignedAnts.end());
  map<Location, Search> searches;
  for (set<Location>::iterator p = unassignedAnts.begin(); p != unassignedAnts.end(); p++) {
    searches[*p] = Search(*p);
  }

  // search breadth first across all ants iteratively
  while (!searchQueue.empty()) {
    Location& antLoc = searchQueue.front();
    Search &search = searches[antLoc];
    if (!search.remaining.empty()) {
      Location& u = search.remaining.front();

      // If this location has one of the remaining food, then assign
      // the current ant to collect it.  Ants may be assigned multiple
      // food, but each food will only be assigned to one ant.
      if (remainingFood.count(u) && search.hills == 0) {
        routes.push_back(Route(antLoc, u, search.predecessors));
        remainingFood.erase(u);
        search.food++;
        unassignedAnts.erase(antLoc);
      }

      // If this location has an enemy hill, then assign the current
      // ant to attack it unless it is already assigned to collect food.
      if (enemyHills.count(u) && search.food == 0) {
        routes.push_back(Route(antLoc, u, search.predecessors));
        remainingHills.erase(u);
        unassignedAnts.erase(antLoc);
        search.hills++;
      }

      // If this location has not been seen by any ant, then assign
      // the current ant to try to see it.
      if (remainingUnseen.count(u) && search.food == 0 && search.hills == 0) {
        routes.push_back(Route(antLoc, u, search.predecessors));
        remainingUnseen.erase(u);
        unassignedAnts.erase(u);
        search.unseen++;
      }

      // Expand this location for this ant, so that the neighboring
      // locations are inspected the next time its this ants turn.
      for (int d = 0; d < TDIRECTIONS; d++) {
        Location v = state.getLocation(u, d);
        if (!state.grid[v.row][v.col].isWater) {
          if (!search.expanded.count(v)) {
            int distance = search.distances[u] + 1;
            search.expanded.insert(v);
            search.distances[v] = distance;
            search.predecessors[v] = u;
            search.remaining.push(v);
          }
        }
      }

      search.remaining.pop();
      if (!search.remaining.empty()) {
        if (!unassignedAnts.empty()) {
          if (!remainingFood.empty() ||
              (search.hills == 0 && search.food == 0 && search.unseen == 0)) {
            searchQueue.push_back(antLoc);
          }
        }
      }
      searchQueue.pop_front();
    }
  }
}

// makes the bots moves for the turn
void Bot::makeMoves() {
  orders.clear();
  xroutes.clear();

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

  set<Location> antsUsed;
  for (list< Route >::iterator r = xroutes.begin(); r != xroutes.end();) {
    Location &a = (*r).steps.front();
    Location &b = (*r).steps.back();
    state.bug << "looking at route " << a << " to " << b << endl;
    if (!myAnts.count(a)) {
      state.bug << "erasing route because its not an ant" << endl;
      xroutes.erase(r++);
    }
    else if (!food.count(b) && !enemyHills.count(b) && !unseen.count(b)) {
      state.bug << "erasing route because its not a target" << endl;
      xroutes.erase(r++);
    }
    else if ((*r).distance <= 1) {
      state.bug << "erasing route because its too short" << endl;
      xroutes.erase(r++);
    }
    else {
      state.bug << "reusing routes " << a << " to " << b << endl;
      ++r;
    }
  }

  search(myAnts, enemyHills, xroutes, 5);
  search(myAnts, food, xroutes);
  xroutes.sort(routeCmp);
  doMoveRoutes(xroutes, antsUsed, enemyHills);
  doMoveRoutes(xroutes, antsUsed, food);
  //  doMoveRoutes(xroutes, antsUsed, unseen);

  goLefty(antsUsed);
  state.bug << "time taken: " << state.timer.getTime() << endl << endl;
}

void Bot::goLefty(set<Location> &antsUsed) {
  set<Location> destinations;
  map<Location,int> newStraight;
  map<Location,int> newLefty;
  for (vector<Location>::iterator p = state.myAnts.begin(); p != state.myAnts.end(); p++) {
    if (antsUsed.count(*p))
      continue;
    if (!straight.count(*p) && !lefty.count(*p))
      straight[*p] = ((*p).row % 2 == 0) ?
          (((*p).col % 2 == 0) ? NORTH : SOUTH) :
          (((*p).col % 2 == 0) ? EAST : WEST);
    if (straight.count(*p)) {
      int d = straight[*p];
      Location dest = state.getLocation(*p, d);
      if (!state.grid[dest.row][dest.col].isWater) {
        if (state.grid[dest.row][dest.col].ant == -1 && !destinations.count(dest)) {
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
        if (!state.grid[dest.row][dest.col].isWater) {
          if (state.grid[dest.row][dest.col].ant == -1 && !destinations.count(dest)) {
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

