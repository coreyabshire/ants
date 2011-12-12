#include "bot.h"

bool Bot::updateAgent(int i, int mode) {
  assert(i != -1);
  Loc a = state.ants[i];
  Square &as = state.grid[a.r][a.c];
  state.bug << "update agent " << i << " " << as.ant << " " << mode << endl;
  assert(as.isUsed == false);
  int bestd = NOMOVE;
  float bestf = -999.0;
  for (int d = 0; d < TDIRECTIONS; d++) {
    Loc b = state.getLoc(a, d);
    Square &bs = state.grid[b.r][b.c];
    switch (mode) {
      case ATTACK:
        break;
      case EVADE:
        break;
      default:
        if (!bs.isWater && !bs.isFood && !bs.isUsed && bs.status[as.ant] == SAFE) {
          float f = 0.0;
          for (int i = 0; i < kFactors; i++)
            f += bs.inf[i] * state.weights[i];
          if (f > bestf) {
            bestd = d;
            bestf = f;
          }
        }
        break;
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

// assign a move to each ant
void Bot::updateAgents(vector<int> &ants, int mode=NORMAL) {
  deque<int> q(ants.begin(), ants.end());
  int moved = 0, remaining = 0;
  do {
    moved = 0;
    remaining = q.size();
    while (!q.empty() && remaining-- > 0) {
      int i = q.front();
      q.pop_front();
      if (updateAgent(i, mode))
        moved++;
      else
        q.push_back(i);
    }
  } while (moved > 0);
}

void Bot::classifyAnts(vector< vector<int> > &battle, vector<int> &normal) {
  for (size_t i = 0; i < state.ants.size(); i++) {
    Loc &a = state.ants[i];
    Square &as = state.grid[a.r][a.c];
    // if (as.battle != -1) {
    //   battle[as.battle].push_back(i);
    // }
    // else if (as.ant == 0) {
    //   normal.push_back(i);
    // }
    if (as.ant == 0)
      normal.push_back(i);
  }
}

bool Bot::nextPermutation(vector<int> &moves) {
  size_t i = 0;
  bool carry = true;
  while (carry && i < moves.size()) {
    moves[i]++;
    if (moves[i] >= TDIRECTIONS) {
      moves[i] = NOMOVE;
      carry = true;
      i++;
    }
    else {
      return true;
    }
  }
  return false;
}

ostream& operator<<(ostream& os, const vector<int> &a) {
  for (vector<int>::const_iterator i = a.begin(); i != a.end(); i++)
    os << (*i) << " ";
  return os;
}

void Bot::makeMoves() {
  state.bug << "turn " << state.turn << ":" << endl;
  state.update();
  vector<int> allAnts;
  for (size_t i = 0; i < state.ants.size(); i++)
    allAnts.push_back(i);
  state.printAnts(allAnts);
  // for (size_t i = 0; i < battle.size(); i++)
  //   makeAttackMoves(battle[i]);
  updateAgents(allAnts);
  state.writeMoves();
  state.bug << "time taken: " << state.turn << " " << state.timer.getTime() << "ms" << endl << endl;
}

// void Bot::makeMoves() {
//   state.bug << "turn " << state.turn << ":" << endl;
//   state.update();

//   const int kRuns = 100;
//   for (int run = 0; run < kRuns; run++) {
//     // pick a random ant (either yours or an enemy ant)
//     int a = rand() % state.ants.size();
//     // move it in each direction (if you can, meaning no water, food, or other ant in the way)
//     // compute the scores for each potential move
//     // if this is an enemy ant, negate those scores so that the enemy ant is trying
//     //    to minimize your score by maximizing -score
//     // for every move that has the greatest possible score, increment the corresponding dirichlet parameter
//     // then sample a move from the new distribution
//   }

//   state.writeMoves();
//   state.bug << "time taken: " << state.turn << " " << state.timer.getTime() << "ms" << endl << endl;
// }

// finishes the turn
void Bot::endTurn() {
  if (state.turn > 0)
    state.reset();
  state.turn++;
  state.sim->go();
}
