#ifndef BOT_H_
#define BOT_H_

#include <map>
#include "State.h"

using namespace std;

/*
    This struct represents your bot in the game of Ants
*/
struct Bot
{
    State state;
    map<Location, int> orders;

    Bot();

    void playGame();    //plays a single game of Ants

    void makeMoves();   //makes moves for a single turn
    void endTurn();     //indicates to the engine that it has made its moves

    bool doMoveDirection(int ant, int d);
};

#endif //BOT_H_
