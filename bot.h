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


  void classifyAnts(vector< vector<int> > &battle, vector<int> &normal);
  void markMyHillsUsed();
  void updateAgents(vector<int> &ants);
  bool updateAgent(Agent &agent);
  int bestDirection(const Location &a);
  int bestEvadeDirection(const Location &a);
  void makeAttackMoves(vector<int> &ants);
  bool nextPermutation(vector<int> &moves);
  bool payoffWin(vector<int> &ants);
};

ostream& operator<<(ostream& os, const vector<int> &a);


#endif //BOT_H_
