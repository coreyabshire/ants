#include "bot.h"

int Bot::bestDirection(const Location &a) {
  Square &as = state.grid[a.row][a.col];
  int bestd = -1;
  float bestf = as.influence();
  for (int d = 1; d < TDIRECTIONS; d++) {
    Location b = state.getLocation(a, d);
    Square &bs = state.grid[b.row][b.col];
    if (!bs.isUsed && (bs.bad == 0 || bs.good > (bs.bad + 1))) {
      float f = bs.influence();
      if (f > bestf) {
        bestd = d;
        bestf = f;
      }
    }
  }
  return bestd;
}

int Bot::bestEvadeDirection(const Location &a) {
  int bestd = NOMOVE;
  float bestf = 999;//as.influence();
  for (int d = 1; d < TDIRECTIONS; d++) {
    Location b = state.getLocation(a, d);
    Square &bs = state.grid[b.row][b.col];
    if (!bs.isUsed && !bs.isWater) {
      float f = bs.inf[ENEMY];
      if (f < bestf) {
        bestd = d;
        bestf = f;
      }
    }
  }
  return bestd;
}

void Bot::markMyHillsUsed() {
  for (vector<Location>::iterator a = state.hills.begin(); a != state.hills.end(); a++) {
    Square &as = state.grid[(*a).row][(*a).col];
    as.isUsed = as.hillPlayer == 0;
  }
}

bool Bot::updateAgent(Agent &agent) {
  Location a = agent.loc;
  Square &as = state.grid[a.row][a.col];
  assert(as.isUsed == false);
  int i = as.index;
  assert(i != -1);
  switch (agent.mode) {
    case ATTACK:
      //      if (as.battle != -1 && as.inf[ENEMY] > 0.3) {
      //  agent.mode = EVADE;
      //  return false;
      //}
      break;
    case EVADE:
      if (as.battle == -1 || as.inf[ENEMY] < 0.3) {
        agent.mode = ATTACK;
        return false;
      }
      break;
    default:
      state.bug << "INVALID MODE: " << agent.mode << endl;
      break;
  }
  
  int bestd = NOMOVE;
  float bestf = as.influence();
  for (int d = 1; d < TDIRECTIONS; d++) {
    Location b = state.getLocation(a, d);
    Square &bs = state.grid[b.row][b.col];
    if (!bs.isWater && !bs.isFood && !bs.isUsed && (bs.bad == 0 || bs.good > (bs.bad + 1))) {
      float f = bs.influence();
      if (f > bestf) {
        bestd = d;
        bestf = f;
      }
    }
  }

  if (bestd != NOMOVE) {
    Location b = state.getLocation(a, bestd);
    Square &bs = state.grid[b.row][b.col];
    if (!bs.isUsed) {
      if (bs.ant != 0) {
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
void Bot::updateAgents(vector<int> &ants) {
  deque<int> q;
  for (size_t i = 0; i < ants.size(); i++) {
    Location &a = state.ants[ants[i]];
    if (state.grid[a.row][a.col].ant == 0)
      q.push_back(ants[i]);
  }
  int moved = 0, remaining = 0;
  do {
    moved = 0;
    remaining = q.size();
    while (!q.empty() && remaining-- > 0) {
      int i = q.front();
      Location &a = state.ants[i];
      Square &as = state.grid[a.row][a.col];
      Agent &agent = state.agents[as.id];
      q.pop_front();
      if (updateAgent(agent))
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
    if (as.battle != -1)
      battle[as.battle].push_back(i);
    else
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

bool Bot::payoffWin(vector<int> &ants) {
  vector<int> dead(state.players, 0);
  state.updateDeadInformation(ants, dead);
  int d = 0;
  for (int i = 0; i < state.players; i++) {
    d += dead[i];
  }
  int u = d - (2*dead[0]);
  for (int i = 1; i < state.players; i++) {
    if ((d - (2*dead[i])) >= u)
      return false;
  }
  return true;
}

void Bot::makeAttackMoves(vector<int> &ants) {
  vector<int> ma, ea;
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
  vector<int> mm(ma.size(), NOMOVE), em(ea.size(), NOMOVE);
  vector<int> best;
  int maxu = -1;
  do {
    if (state.tryMoves(ma, mm)) {
      //state.bug << "trying my moves " << mm << endl;
      //state.printAnts(ants);
      int u = 0;//utility(ants);
      do {
        //state.printAnts(ants);
        if (state.tryMoves(ea, em)) {
          //state.bug << "trying enemy moves " << em << endl;
          //state.printAnts(ants);
          if (payoffWin(ants)) {
            ++u;
          }
          state.undoMoves(ea);
        }
      } while (nextPermutation(em));
      if (u > maxu) {
        maxu = u;
        best = mm;
      }
      //state.bug << "undoing my moves " << mm << endl;
      //state.printAnts(ants);
      state.undoMoves(ma);
      //state.printAnts(ants);
    }
  } while (nextPermutation(mm));
  if (maxu > -1) {
    state.bug << "found best moves " << " " << maxu << " " << best << endl;
    assert(state.tryMoves(ma, best));
    // for (size_t i = 0; i < ma.size(); i++)
    //   state.makeMove(ma[i], best[i]);
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
