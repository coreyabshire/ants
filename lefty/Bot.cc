#include "Bot.h"

using namespace std;

void Bot::playGame() {
  cin >> state;
  state.setup();
  lefty = vvc(state.rows, vector<char>(state.cols, -1));
  straight = vvc(state.rows, vector<char>(state.cols, -1));
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

ostream& operator<<(ostream& os, const Location &a) { os << a.row << "," << a.col; }
void Bot::makeMoves() {
  state.bug << "turn " << state.turn << ":" << endl;
  state.bug << state << endl;
  vvc newLefty(state.rows, vector<char>(state.cols, -1));
  vvc newStraight(state.rows, vector<char>(state.cols, -1));
  vvb destinations(state.rows, vector<bool>(state.cols, false));
  for (int ant = 0; ant < (int)state.myAnts.size(); ant++) {
    Location a = state.myAnts[ant];
    if (straight[a.row][a.col] == -1 && lefty[a.row][a.col] == -1) {
      straight[a.row][a.col] = (a.row % 2) ? ((a.col % 2)?W:E) : ((a.col % 2)?S:N);
    }
    if (straight[a.row][a.col] != -1) {
      char d = straight[a.row][a.col];
      Location b = state.getLocation(a, d);
      Square &bs = state.grid[b.row][b.col];
      if (!bs.isWater && bs.hillPlayer != 0) {
        if (bs.ant < 0 && !destinations[b.row][b.col]) {
          state.makeMove(a, d);
          newStraight[b.row][b.col] = d;
          destinations[b.row][b.col] = true;
        }
        else {
          newStraight[a.row][a.col] = LEFT[d];
          destinations[a.row][a.col] = true;
        }
      }
      else {
        lefty[a.row][a.col] = RIGHT[d];
      }
    }
    if (lefty[a.row][a.col] != -1) {
      state.bug << "turning " << a << endl;
      char d = lefty[a.row][a.col];
      char ds[] = { LEFT[d], d, RIGHT[d], BEHIND[d] };
      for (int i = 0; i < TDIRECTIONS; i++) {
        char nd = ds[i];
        state.bug << "trying " << (int) d << " " << i << " " << (int) nd << endl;
        Location b = state.getLocation(a, nd);
        Square &bs = state.grid[b.row][b.col];
        if (!bs.isWater && bs.hillPlayer != 0) {
          if (bs.ant < 0 && !destinations[b.row][b.col]) {
            state.makeMove(a, nd);
            newLefty[b.row][b.col] = nd;
            destinations[b.row][b.col] = true;
            break;
          }
          else {
            newStraight[a.row][a.col] = RIGHT[d];
            destinations[a.row][a.col] = true;
            break;
          }
        }
      }
    }
  }
  straight = newStraight;
  lefty = newLefty;
  state.bug << "time taken: " << state.timer.getTime() << "ms" << endl << endl;
}

void Bot::endTurn() {
  if (state.turn > 0)
    state.reset();
  state.turn++;
  cout << "go" << endl;
}
