#include <cassert>
#include "bot.h"

bool Bot::updateAgent(int i) {
  assert(i != -1);
  Loc a = state.ants[i];
  Square &as = state.grid[a.r][a.c];
  state.bug << "update agent " << i << " " << as.ant << " " << endl;
  assert(as.isUsed == false);
  int bestd = NOMOVE;
  float bestf = -999.0;
  for (int d = 0; d < TDIRECTIONS; d++) {
    Loc b = state.getLoc(a, d);
    Square &bs = state.grid[b.r][b.c];
    if (!bs.isWater && !bs.isFood && !bs.isUsed && bs.status == SAFE) {
      float f = 0.0;
      for (int i = 0; i < kFactors; i++)
        f += bs.inf[i] * state.weights[i];
      if (f > bestf) {
        bestd = d;
        bestf = f;
      }
    }
  }
  if (bestd != NOMOVE) {
    Loc b = state.getLoc(a, bestd);
    Square &bs = state.grid[b.r][b.c];
    if (!bs.isUsed) {
      if (bs.ant == -1) {
        assert(bs.isCleared());
        state.makeMove(i, bestd);
        return true;
      }
      else {
        return false;
      }
    }
    else {
      state.makeMove(i, NOMOVE);
      return true;
    }
  }
  else {
    state.makeMove(i, NOMOVE);
    return true;
  }
}

ostream& operator<<(ostream& os, const v1i &a) {
  for (vector<int>::const_iterator i = a.begin(); i != a.end(); i++)
    os << (*i) << " ";
  return os;
}

void Bot::makeMoves() {
  state.bug << "turn " << state.turn << ":" << endl;
  v1i ants;
  for (size_t i = 0; i < state.ants.size(); i++)
    ants.push_back(i);
  deque<int> q(ants.begin(), ants.end());
  int moved = 0, remaining = 0;
  do {
    moved = 0;
    remaining = q.size();
    while (!q.empty() && remaining-- > 0) {
      int i = q.front();
      q.pop_front();
      if (updateAgent(i))
        moved++;
      else
        q.push_back(i);
    }
  } while (moved > 0);
  state.writeMoves();
  state.bug << "time taken: " << state.turn << " " << state.timer.getTime() << "ms" << endl << endl;
}

// finishes the turn
void Bot::endTurn() {
  if (state.turn > 0)
    state.reset();
  state.turn++;
  state.sim->go();
}
