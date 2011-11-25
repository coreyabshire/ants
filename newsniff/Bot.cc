#include "Bot.h"

using namespace std;

//constructor
Bot::Bot() {}

//plays a single game of Ants.
void Bot::playGame() {
  
  //reads the game parameters and sets up
  cin >> state;
  state.setup();
  endTurn();

  //continues making moves while the game is not over
  while(cin >> state) {
    state.updateVisionInformation();
    state.updateInfluenceInformation();
    makeMoves();
    endTurn();
  }
};

//makes the bots moves for the turn
void Bot::makeMoves() {
  state.bug << "turn " << state.turn << ":" << endl;
  state.bug << state << endl;

  //picks out moves for each ant
  vector< vector<bool> > used(state.rows, vector<bool>(state.cols, false));
  for (int ant = 0; ant < (int)state.myAnts.size(); ant++) {
    Location a = state.myAnts[ant];
    int bestd = -1;
    float bestf = 0.0;
    for (int d = 0; d < TDIRECTIONS; d++) {
      Location b = state.getLocation(a, d);
      Square &bs = state.grid[b.row][b.col];
      if (!used[b.row][b.col]) {
        float f = 0.0;
        for (int k = 0; k < kFactors; k++)
          f += bs.inf[k] * weights[k];
        if (f > bestf) {
          bestd = d;
          bestf = f;
        }
      }
    }
    if (bestd != -1) {
      Location b = state.getLocation(a, bestd);
      state.makeMove(a, bestd);
      used[b.row][b.col] = true;
    }
    else {
      used[a.row][a.col] = true;
    }
  }

  state.bug << "time taken: " << state.timer.getTime() << "ms" << endl << endl;
};

//finishes the turn
void Bot::endTurn() {
  if(state.turn > 0)
    state.reset();
  state.turn++;

  cout << "go" << endl;
};
