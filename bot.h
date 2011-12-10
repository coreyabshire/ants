#ifndef BOT_H_
#define BOT_H_

#include <queue>
#include <deque>
#include <algorithm>
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


  void classifyAnts(v2i &battle, v1i &normal);
  void markMyHillsUsed();
  void updateAgents(v1i &ants, int mode);
  bool updateAgent(int i, int mode);
  int bestDirection(const Loc &a);
  int bestEvadeDirection(const Loc &a);
  void makeAttackMoves(v1i &ants);
  bool nextPermutation(v1i &moves);
  void payoffCell(v1i &ants, v1i &cell);
  void printPayoffMatrix(v3i &pm);
};

ostream& operator<<(ostream& os, const v1i &a);


#endif //BOT_H_
