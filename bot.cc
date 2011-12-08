#include "bot.h"

void Bot::markMyHillsUsed() {
  for (vector<Location>::iterator a = state.hills.begin(); a != state.hills.end(); a++) {
    Square &as = state.grid[(*a).row][(*a).col];
    as.isUsed = as.hillPlayer == 0;
  }
}

bool Bot::updateAgent(int i, int mode) {
  assert(i != -1);
  Location a = state.ants[i];
  Square &as = state.grid[a.row][a.col];
  state.bug << "update agent " << i << " " << as.ant << " " << mode << endl;
  assert(as.isUsed == false);
  int bestd = NOMOVE;
  float bestf = -999.0;
  for (int d = 0; d < TDIRECTIONS; d++) {
    Location b = state.getLocation(a, d);
    Square &bs = state.grid[b.row][b.col];
    switch (mode) {
      case ATTACK:
        if (!bs.isWater && !bs.isFood && !bs.isUsed) {
          float f = bs.ant == 0 ? bs.inf[ENEMY] : bs.inf[FRIEND];
          if (f > bestf) {
            bestd = d;
            bestf = f;
          }
        }
        break;
      case EVADE:
        if (!bs.isWater && !bs.isFood && !bs.isUsed) {
          float f = bs.ant == 0 ? bs.inf[ENEMY] : bs.inf[FRIEND];
          f *= -1.0;
          if (f > bestf) {
            bestd = d;
            bestf = f;
          }
        }
        break;
      default:
        if (!bs.isWater && !bs.isFood && !bs.isUsed) {
          float f = bs.influence();
          if (f > bestf) {
            bestd = d;
            bestf = f;
          }
        }
        break;
    }
  }
  state.bug << "best move " << a << " " << CDIRECTIONS[bestd] << endl;
  if (bestd != NOMOVE) {
    Location b = state.getLocation(a, bestd);
    Square &bs = state.grid[b.row][b.col];
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
    Location &a = state.ants[i];
    Square &as = state.grid[a.row][a.col];
    if (as.battle != -1) {
      battle[as.battle].push_back(i);
    }
    else if (as.ant == 0) {
      normal.push_back(i);
    }
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

void Bot::payoffCell(v1i &ants, v1i &cell) {
  v1i dead(state.players, 0);
  state.updateDead(ants, dead);
  cell = dead;
}

void Bot::printPayoffMatrix(v3i &pm) {
  state.bug << "payoff matrix" << endl;
  for (size_t i = 0; i < (size_t) kModes; i++) {
    state.bug << "payoff ";
    for (size_t j = 0; j < (size_t) kModes; j++) {
      state.bug << ((j == 0) ? "" : " | ");
      for (size_t k = 0; k < (size_t) state.players; k++) {
        state.bug << ((k == 0) ? "" : ",") << pm[i][j][k];
      }
    }
    state.bug << endl;
  }
}

void Bot::makeAttackMoves(v1i &ants) {
  v1i ma, ea;
  state.bug << "calculating attack moves " << ants << endl;
  state.printAnts(ants);
  for (size_t i = 0; i < ants.size(); i++) {
    Location &a = state.ants[ants[i]];
    Square &as = state.grid[a.row][a.col];
    assert(as.isUsed == false);
    if (as.ant == 0)
      ma.push_back(ants[i]);
    else
      ea.push_back(ants[i]);
  }
  state.bug << "my ants " << ma << endl;
  state.bug << "enemy ants " << ea << endl;
  int best = 0;
  int maxu = -1;
  int u = 0;
  v3i pm(kModes, v2i(kModes, v1i(state.players, 0)));
  for (size_t i = 0; i < (size_t)kModes; i++) {
    updateAgents(ma, i);
    state.bug << "My Ants: " << i << endl;
    state.printAnts(ma);
    for (size_t j = 0; j < (size_t)kModes; j++) {
      state.bug << "Enemy Ants: " << j << endl;
      state.printAnts(ea);
      updateAgents(ea, j);
      v1i &cell = pm[i][j];
      payoffCell(ants, cell);
      int killed = 0;
      for (int i = 1; i < state.players; i++)
        killed += cell[i];
      int lost = cell[0];
      if (lost) {
        if (killed > lost) {
          u += 2;
        }
        else {
          u -= 1;
        }
      }
      state.undoMoves(ea);
    }
    if (u > maxu) {
      maxu = u;
      best = i;
    }
    state.undoMoves(ma);
  }
  printPayoffMatrix(pm);
  if (maxu > -1) {
    state.bug << "found best moves " << " " << maxu << " " << best << endl;
    updateAgents(ma, best);
  }
}

void Bot::makeMoves() {
  state.bug << "turn " << state.turn << ":" << endl;
  state.update();
  vector<int> allAnts;
  for (size_t i = 0; i < state.ants.size(); i++) {
    allAnts.push_back(i);
  }
  state.printAnts(allAnts);
  vector< vector<int> > battle(state.battle + 1);
  vector<int> normal;
  classifyAnts(battle, normal);
  //markMyHillsUsed();
  for (size_t i = 0; i < battle.size(); i++)
    makeAttackMoves(battle[i]);
  if (!normal.empty())
    updateAgents(normal);
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
