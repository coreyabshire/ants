#ifndef BOT_H_
#define BOT_H_

#include <map>
#include <set>
#include <queue>
#include <algorithm>
#include "State.h"
#include "Route.h"
#include "Search.h"

using namespace std;

/*
    This struct represents your bot in the game of Ants
*/
struct Bot
{
    State state;
    set<Location> orders;
    set<Location> unseen;
    set<Location> enemyHills;
    set<Location> food;

    Bot();

    void playGame();    //plays a single game of Ants

    void makeMoves();   //makes moves for a single turn
    void endTurn();     //indicates to the engine that it has made its moves

    bool doMoveDirection(const Location &ant, int d);
    bool doMoveLocation(const Location &antLoc, const Location &destLoc);
    vector<Location> shortestPath(const Location &a, const Location &b);
};

#endif //BOT_H_
