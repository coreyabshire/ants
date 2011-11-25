#include "Bot.h"

using namespace std;

void Bot::playGame() {
  cin >> state;
  state.setup();
  directions = vvc(state.rows, vector<char>(state.cols, -1));
  endTurn();
  while (cin >> state) {
    state.updateVisionInformation();
    makeMoves();
    endTurn();
  }
}

const char N = 0, E = 1, S = 2, W = 3;
// The base dirs: N, E, S, W
char LEFT[]   = { W, N, E, S };
char RIGHT[]  = { E, S, W, N };
char BEHIND[] = { S, W, N, E };

void Bot::makeMoves() {
  state.bug << "turn " << state.turn << ":" << endl;
  state.bug << state << endl;
  state.diffuse();
  vvc newDirections(state.rows, vector<char>(state.cols, -1));
  vvb destinations(state.rows, vector<bool>(state.cols, false));
  for (int ant = 0; ant < (int)state.myAnts.size(); ant++) {
    Location a = state.myAnts[ant];
    if (directions[a.row][a.col] == -1) {
      directions[a.row][a.col] = rand() % 4;
    }
    int d = directions[a.row][a.col];
    double maxAroma = 0;
    Location v = state.getLocation(a, d);
    Square &vs = state.grid[v.row][v.col];
    for (int nd = 0; nd < 4; nd++) {
      Location b = state.getLocation(a, nd);
      Square &bs = state.grid[b.row][b.col];
      if (bs.aroma > maxAroma && !destinations[b.row][b.col]) {
        maxAroma = bs.aroma;
        d = nd;
      }
    }
    Location c = state.getLocation(a, d);
    Square &cs = state.grid[c.row][c.col];
    if (!cs.isWater && !destinations[c.row][c.col] && cs.ant == -1) {
      state.makeMove(a, d);
      destinations[c.row][c.col] = true;
      newDirections[c.row][c.col] = d;
    }
    else {
      destinations[a.row][a.col] = true;
      newDirections[a.row][a.col] = d;
    }
  }
  directions = newDirections;
  state.bug << "time taken: " << state.timer.getTime() << "ms" << endl << endl;
}

void Bot::endTurn() {
  if (state.turn > 0)
    state.reset();
  state.turn++;
  cout << "go" << endl;
}
