#ifndef BOT_H_
#define BOT_H_

#include "State.h"

struct Bot {
  State state;

  Bot();

  void playGame();
  void makeMoves();
  void endTurn();
};

struct Route {
  double d;
  Location a, b;
  Route(const Location &a, const Location &b, double d) : a(a), b(b), d(d) {};
};
bool operator<(const Route &a, const Route &b);

#endif //BOT_H_
