#ifndef BOT_H_
#define BOT_H_

#include "state.h"

using namespace std;

class Bot {
 public:
  State state;
  Bot() {};
  Bot(Sim *sim) : state(sim) {};
  Bot(int rows, int cols) : state(rows, cols) {};

  void makeMoves();  //makes moves for a single turn
  void endTurn();    //indicates to the engine that it has made its moves
  bool updateAgent(int i);
};

ostream& operator<<(ostream& os, const v1i &a);

#endif //BOT_H_
