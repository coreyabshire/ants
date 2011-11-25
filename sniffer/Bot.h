#ifndef BOT_H_
#define BOT_H_

#include <vector>
#include <queue>

#include "State.h"
#include "Location.h"

using namespace std;

struct Bot {
  State state;
  vvc directions;
  Bot() {};
  void playGame();
  void makeMoves();
  void endTurn();
};

#endif //BOT_H_
