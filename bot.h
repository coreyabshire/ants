#ifndef BOT_H_
#define BOT_H_

#include <map>
#include <set>
#include <queue>
#include <deque>
#include <algorithm>
#include <list>
#include "state.h"

using namespace std;

class Search {
 public:
  Location start;
  std::map<Location,int> distances;
  std::map<Location,Location> predecessors;
  std::set<Location> expanded;
  std::queue<Location> remaining;
  int food, hills, unseen;

  Search() {};
  Search(const Location &start);
  int distance(const Location& dest);
  Location step(const Location& dest);
};

class Manhattan {
 public:
  int d;
  Location a, b;
  Manhattan(const Location &a, const Location &b, int d) : a(a), b(b), d(d) {};
};
bool operator<(const Manhattan &a, const Manhattan &b);

class Fcmp {
  map<Location,int> *f;
 public:
  Fcmp(map<Location,int> *f) : f(f) {};
  bool operator() (const Location& a, const Location& b) const {
    return (*f)[a] < (*f)[b];
  }
};

class Agent;
class Bot;

class AgentState {
 public:
  virtual ~AgentState() {};
  virtual void enter(Agent *) = 0;
  virtual bool execute(Agent *) = 0;
  virtual void exit(Agent *) = 0;
};

class Agent {
 public:
  int id;
  int fatigue;
  Location loc;
  AgentState *state;
  Agent(int id, const Location& loc, AgentState *state) : id(id), loc(loc), state(state) {};
  virtual ~Agent() {};
  bool update();
  void change(AgentState *s);
};

class Lazy : public AgentState {
 public:
  Bot *bot;
  Lazy() {};
  Lazy(Bot *bot) : bot(bot) {};
  virtual void enter(Agent *a);
  virtual bool execute(Agent *a);
  virtual void exit(Agent *a);
};

class Active : public AgentState {
 public:
  Bot *bot;
  Active() {};
  Active(Bot *bot) : bot(bot) {};
  virtual void enter(Agent *a);
  virtual bool execute(Agent *a);
  virtual void exit(Agent *a);
};

class Defend : public AgentState {
 public:
  Bot *bot;
  Defend() {};
  Defend(Bot *bot) : bot(bot) {};
  virtual void enter(Agent *a);
  virtual bool execute(Agent *a);
  virtual void exit(Agent *a);
};

class Bot {
 public:
  State state;
  Bot();
  Bot(Sim *sim) : state(sim) {};
  Bot(int rows, int cols);
  void playGame();   //plays a single game of Ants
  void setup();      //set up the bot on initial turn
  void makeMoves();  //makes moves for a single turn
  void makeMovesVector();  //makes moves for a single turn
  void oldMakeMoves();  //makes moves for a single turn
  void endTurn();    //indicates to the engine that it has made its moves

  set<Location> orders;
  set<Location> unseen;
  set<Location> myAnts, myHills, enemyAnts, enemyHills, food;
  list< Route > xroutes;
  int nextAntId;

  map<Location,int> straight;
  map<Location,int> lefty;
  map<Location,Route> routes;
  vector<Agent> agents;

  Lazy lazy;
  Active active;
  Defend defend;

  int bestDirection(const Location &a);

  void goLefty(set<Location> &antsUsed);  //makes moves for a single turn
  void goLefty2(set<Location> &antsUsed);  //makes moves for a single turn
  bool doMoveDirection(const Location &ant, int d);
  bool doMoveLocation(const Location &antLoc, const Location &destLoc);
  bool doMoveRoute(Route &route, set<Location> &antsUsed);
  bool doMoveRoutes(list< Route >& routes, set<Location> &antsUsed,
                    set<Location> targets);
  vector<Location> shortestPath(const Location &a, const Location &b);
  void insertAll(set<Location> &to, vector<Location> &from);
  void removeIf(set<Location> &locs, bool(*pred)(Square &));
  void updateMemory(set<Location> &memory, vector<Location> &seen, bool(*pred)(Square &));
  bool search(Location &start, set<Location> &ends, Route &route);
  void search(set<Location> &sources, set<Location> &targets, list< Route > &routes);
  void search(set<Location> &sources, list< Route > &routes);
  int search(set<Location> &sources, const Location &target, list< Route > &routes, int maxCount);
  bool search(Location &start, Location &goal, Route &route);
  int numAttackAnts(int totalAnts);
};


#endif //BOT_H_
