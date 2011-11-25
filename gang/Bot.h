#ifndef BOT_H_
#define BOT_H_

#include <string>
#include <set>
#include <map>
#include <queue>
#include <algorithm>
#include "State.h"

using namespace std;

struct Ant {
  string moves;
  int moveid;
  Ant() {};
  void order(string newmoves);
};

struct Unit {
  Ant a, b, c, d;
};

struct Bot {
  State state;
  vector<Ant> ants;

  Bot();

  void playGame();    //plays a single game of Ants
  void makeMoves();   //makes moves for a single turn
  void endTurn();     //indicates to the engine that it has made its moves

  void old();
  string search(const Location &a, const Location &b);

};

#endif //BOT_H_
