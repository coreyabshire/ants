#include "Bot.h"

using namespace std;

//constructor
Bot::Bot() {};

//plays a single game of Ants.
void Bot::playGame() {
  //reads the game parameters and sets up
  cin >> state;
  state.setup();
  setup();
  endTurn();

  //continues making moves while the game is not over
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

bool Bot::doMoveLocation(const Location &antLoc, const Location &destLoc) {
  vector<int> directions = state.getDirections(antLoc, destLoc);
  for (vector<int>::iterator dp = directions.begin(); dp != directions.end(); dp++) {
    if (doMoveDirection(antLoc, *dp)) {
      return true;
    }
  }
  return false;
}

bool Bot::doMoveRoutes(vector<Route>& routes, map<Location, Search> &searches,
                       set<Location> &antsUsed) {
  sort(routes.begin(), routes.end());
  for (vector<Route>::iterator r = routes.begin(); r != routes.end(); r++) {
    if (!antsUsed.count((*r).start) &&
        doMoveLocation((*r).start, searches[(*r).start].step((*r).end))) {
      antsUsed.insert((*r).start);
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

// find the shortest route to the nearest end to start
bool Bot::search(Location &start, set<Location> &ends, Route &route) {
  map<Location,int> distances;
  map<Location,Location> predecessors;
  set<Location> expanded;
  queue<Location> remaining;
    
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

void Bot::search(map<Location, Search> &searches,
		 set<Location> &sources,
		 vector<Route> &foodRoutes,
		 vector<Route> &hillRoutes,
		 vector<Route> &unseenRoutes) {
  deque<Location> searchQueue(sources.begin(), sources.end());
  set<Location> unassignedAnts(sources.begin(), sources.end());
  set<Location> remainingFood(food.begin(), food.end());
  set<Location> remainingHills(enemyHills.begin(), enemyHills.end());
  set<Location> remainingUnseen(unseen.begin(), unseen.end());
  map<Location,int> distances;

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
        foodRoutes.push_back(Route(antLoc, u, search.distances[u]));
        remainingFood.erase(u);
        search.food++;
        unassignedAnts.erase(antLoc);
      }

      if (myAnts.count(u))
        search.nearestMyAnt = u;

      if (enemyAnts.count(u))
        search.nearestEnemyAnt = u;

      // If this location has an enemy hill, then assign the current
      // ant to attack it unless it is already assigned to collect food.
      if (enemyHills.count(u) && search.food == 0) {
        hillRoutes.push_back(Route(antLoc, u, search.distances[u]));
        remainingHills.erase(u);
        unassignedAnts.erase(antLoc);
        search.hills++;
      }

      // If this location has not been seen by any ant, then assign
      // the current ant to try to see it.
      if (remainingUnseen.count(u) && search.food == 0 && search.hills == 0) {
        unseenRoutes.push_back(Route(antLoc, u, search.distances[u]));
        remainingUnseen.erase(u);
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
            distances[u] = distance;
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

//makes the bots moves for the turn
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

  set<Location> antsUsed;
  vector<Route> foodRoutes;
  vector<Route> hillRoutes;
  vector<Route> unseenRoutes;
  map<Location, Search> searches;
    
  // initialize the search state
  for (set<Location>::iterator p = myAnts.begin(); p != myAnts.end(); p++) {
    searches[*p] = Search(*p);
  }

  search(searches, myAnts, foodRoutes, hillRoutes, unseenRoutes);

  // assign ants to the closest food
  doMoveRoutes(foodRoutes, searches, antsUsed);

  // assign ants to destroy enemy hills
  doMoveRoutes(hillRoutes, searches, antsUsed);

  // explore unseen areas
  doMoveRoutes(unseenRoutes, searches, antsUsed);

  state.bug << "route counts: "
            << foodRoutes.size() << " "
            << hillRoutes.size() << " "
            << unseenRoutes.size() << endl;

  state.bug << "time taken: " << state.timer.getTime() << endl << endl;
};

//finishes the turn
void Bot::endTurn() {
  if(state.turn > 0)
    state.reset();
  state.turn++;
  cout << "go" << endl;
};
