#include "Bot.h"

using namespace std;

//constructor
Bot::Bot()
{

};

//plays a single game of Ants.
void Bot::playGame()
{
    //reads the game parameters and sets up
    cin >> state;
    state.setup();
    endTurn();

    //continues making moves while the game is not over
    while(cin >> state)
    {
        state.updateVisionInformation();
        makeMoves();
        endTurn();
    }
};

bool Bot::doMoveDirection(int ant, int d)
{
    Location loc = state.getLocation(state.myAnts[ant], d);
    Square& square = state.grid[loc.row][loc.col];
    state.bug << "doMoveDirection: " << ant << ", " << d << ", "
	      << loc << ", [" << square << "], " << orders.count(loc) << endl;
    if(!square.isWater && !orders.count(loc))
    {
	orders[loc] = ant;
	state.makeMove(state.myAnts[ant], d);
	state.bug << "made move: " << loc << ", " << ant << endl;
	return true;
    }
    else
    {
	state.bug << "could not move " << ant << " to " << loc << endl;
	return false;
    }
}

//makes the bots moves for the turn
void Bot::makeMoves()
{
    state.bug << "turn " << state.turn << ":" << endl;
    state.bug << state << endl;
    state.bug << "orders: " << endl;
    for (map<Location,int>::iterator i = orders.begin(); i != orders.end(); i++) {
	state.bug << (*i).first << ": " << (*i).second << endl;
    }
    state.bug << "end orders" << endl;
    state.bug << "total orders at beginning: " << orders.size() << endl;

    //picks out moves for each ant
    for(int ant=0; ant<(int)state.myAnts.size(); ant++)
    {
        for(int d=0; d<TDIRECTIONS; d++)
        {
	    if(doMoveDirection(ant, d))
	    {
		break;
	    }
        }
    }

    state.bug << "time taken: " << state.timer.getTime() << "ms" << endl << endl;
};

//finishes the turn
void Bot::endTurn()
{
    if(state.turn > 0)
        state.reset();
    state.turn++;
    state.bug << "total orders: " << orders.size() << endl;
    orders.clear();
    state.bug << "orders erased" << endl;
    state.bug << "total orders after erased: " << orders.size() << endl;
    cout << "go" << endl;
};
