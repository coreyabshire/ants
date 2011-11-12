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

bool Bot::doMoveDirection(const Location &antLoc, int d)
{
    Location newLoc = state.getLocation(antLoc, d);
    Square& square = state.grid[newLoc.row][newLoc.col];
    if(!square.isWater && !orders.count(newLoc))
    {
	orders[newLoc] = antLoc;
	state.makeMove(antLoc, d);
	return true;
    }
    else
    {
	return false;
    }
}

//makes the bots moves for the turn
void Bot::makeMoves()
{
    orders.clear();
    
    state.bug << "turn " << state.turn << ":" << endl;
    state.bug << state << endl;

    //picks out moves for each ant
    for(vector<Location>::iterator antp = state.myAnts.begin();
	antp < state.myAnts.end(); antp++)
    {
        for(int d=0; d<TDIRECTIONS; d++)
        {
	    if(doMoveDirection(*antp, d))
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
    cout << "go" << endl;
};
