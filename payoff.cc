#include <iostream>
#include "bot.h"
#include "state.h"

using namespace std;

char cd(int d) {
  return d == -1 ? '0' : CDIRECTIONS[d];
}

void prmoves(vector<int> m) {
  for (int i = 0; i < m.size(); i++) {
    cout << cd(m[i]);
  }
}

bool payoffWin(State &state, vector<int> &ants) {
  vector<int> dead(state.players, 0);
  state.updateDeadInformation(ants, dead);
  int d = 0;
  for (int i = 0; i < state.players; i++) {
    cout << dead[i];
    d += dead[i];
  }
  int u = d - (2*dead[0]);
  for (int i = 1; i < state.players; i++) {
    if ((d - (2*dead[i])) >= u)
      return false;
  }
  return true;
}

int main(int argc, char **argv) {
  Bot bot(100,100);
  State &state = bot.state;
  state.putAnt(2,1,1);
  state.putAnt(3,4,0);
  state.putAnt(4,3,0);
  vector<int> ants;
  for (int i = 0; i < state.ants.size(); i++)
    ants.push_back(i);
  vector<int> ma, ea;
  state.bug << "calculating attack moves " << ants << endl;
  state.printAnts(ants);
  for (int i = 0; i < ants.size(); i++) {
    Location &a = state.ants[ants[i]];
    Square &as = state.grid[a.row][a.col];
    if (as.ant == 0)
      ma.push_back(ants[i]);
    else
      ea.push_back(ants[i]);
  }
  cout << "my ants " << ma << endl;
  cout << "enemy ants " << ea << endl;
  vector<int> mm(ma.size(), -1), em(ea.size(), -1);
  vector<int> best;
  int maxu = -1;
  cout << "   ";
  do {
    prmoves(em);
    cout << "   ";
  } while(bot.nextPermutation(em));
  cout << endl;
  do {
    if (state.tryMoves(ma, mm)) {
      //cout << "trying my moves " << mm << endl;
      //state.printAnts(ants);
      int u = 0;//utility(ants);
      prmoves(mm);
      cout << " ";
      do {
        //state.printAnts(ants);
        if (state.tryMoves(ea, em)) {
          //cout << "trying enemy moves " << em << endl;
          //state.printAnts(ants);
          if (payoffWin(state, ants)) {
            cout << "T ";
            ++u;
          }
          else {
            cout << "F ";
          }
          state.undoMoves(ea);
        }
      } while (bot.nextPermutation(em));
      cout << " " << u;
      if (u > maxu) {
        cout << " T";
        maxu = u;
        best = mm;
      }
      else {
        cout << " F";
      }
      cout << endl;
      //cout << "undoing my moves " << mm << endl;
      //state.printAnts(ants);
      state.undoMoves(ma);
      //state.printAnts(ants);
    }
  } while (bot.nextPermutation(mm));
  if (maxu > -1) {
    cout << "found best moves " << " " << maxu << " " << best << endl;
    for (int i = 0; i < ma.size(); i++)
      state.makeMove(ma[i], best[i]);
  }
}
