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

bool Bot::doMoveLocation(const Location &antLoc, const Location &destLoc)
{
    vector<int> directions = state.getDirections(antLoc, destLoc);
    for (vector<int>::iterator dp = directions.begin(); dp != directions.end(); dp++)
    {
	if (doMoveDirection(antLoc, *dp))
	{
	    return true;
	}
    }
    return false;
}

//makes the bots moves for the turn
void Bot::makeMoves()
{
    orders.clear();
    
    state.bug << "turn " << state.turn << ":" << endl;
    state.bug << state << endl;

    set<Location> foodTargets;
    set<Location> antsUsed;
    vector<Route> foodRoutes;
    set<Location> sortedFood(state.food.begin(), state.food.end());
    set<Location> sortedAnts(state.myAnts.begin(), state.myAnts.end());

    for (set<Location>::iterator foodp = sortedFood.begin();
	 foodp != sortedFood.end(); foodp++)
    {
	for (set<Location>::iterator antp = sortedAnts.begin();
	     antp != sortedAnts.end(); antp++)
	{
	    double distance = state.distance(*antp, *foodp);
	    foodRoutes.push_back(Route(*antp, *foodp, distance));
	}
    }

    sort(foodRoutes.begin(), foodRoutes.end());

    for (vector<Route>::iterator routep = foodRoutes.begin();
	 routep != foodRoutes.end(); routep++)
    {
	if (!foodTargets.count((*routep).end) &&
	    !antsUsed.count((*routep).start) &&
	    doMoveLocation((*routep).start, (*routep).end))
	{
	    foodTargets.insert((*routep).end);
	    antsUsed.insert((*routep).start);
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
