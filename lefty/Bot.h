#ifndef BOT_H_
#define BOT_H_

#include "State.h"

typedef vector< vector<char> > vvc;
typedef vector< vector<bool> > vvb;

struct Bot {
  State state;
  vvc lefty, straight;
  Bot() {};
  void playGame();
  void makeMoves();
  void endTurn();
};

#endif //BOT_H_
