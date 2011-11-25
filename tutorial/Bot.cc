#include "Bot.h"
#include <algorithm>

using namespace std;

Bot::Bot() {
}

void Bot::playGame() {
  cin >> state;
  state.setup();
  endTurn();

  while (cin >> state) {
    state.updateVisionInformation();
    makeMoves();
    endTurn();
  }
}

bool operator<(const Route &a, const Route &b) { return a.d < b.d; }
ostream& operator<<(ostream& os, const Location &a) { return os << a.row << "," << a.col; }
ostream& operator<<(ostream& os, const Route &r) { return os << r.a << " " << r.b << " " << r.d; }
void Bot::makeMoves() {
  state.bug << "turn " << state.turn << ":" << endl;
  state.bug << state << endl;
  vector< vector<bool> > available(state.rows, vector<bool>(state.cols, true));
  vector<Route> routes;
  for (int i = 0; i < state.food.size(); i++) {
    const Location &f = state.food[i];
    for (int j = 0; j < state.myAnts.size(); j++) {
      const Location &a = state.myAnts[j];
      routes.push_back(Route(a, f, state.distance(a, f)));
    }
  }
  sort(routes.begin(), routes.end());
  for (int i = 0; i < routes.size(); i++) {
    state.bug << "route " << state.turn << " " << routes[i] << endl;
  }
  for (int ant = 0; ant < state.myAnts.size(); ant++) {
    for (int d = 0; d < TDIRECTIONS; d++) {
      Location loc = state.getLocation(state.myAnts[ant], d);
      if (!state.grid[loc.row][loc.col].isWater && available[loc.row][loc.col]) {
        state.makeMove(state.myAnts[ant], d);
        available[loc.row][loc.col] = false;
        break;
      }
    }
  }
  state.bug << "time taken: " << state.timer.getTime() << "ms" << endl << endl;
}

//finishes the turn
void Bot::endTurn() {
  if(state.turn > 0)
    state.reset();
  state.turn++;
  cout << "go" << endl;
}
