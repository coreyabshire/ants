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

    // determine all currently unseen tiles
    for (int row = 0; row < state.rows; row++)
    {
	for (int col = 0; col < state.cols; col++)
	{
	    unseen.insert(Location(row, col));
	}
    }

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

    // keep ants from moving onto our own hills and preventing spawning
    for (vector<Location>::iterator hillp = state.myHills.begin();
	 hillp != state.myHills.end(); hillp++)
    {
	orders[*hillp] = *hillp;
    }
    
    state.bug << "turn " << state.turn << ":" << endl;
    state.bug << state << endl;

    set<Location> foodTargets;
    set<Location> antsUsed;
    vector<Route> foodRoutes;
    set<Location> sortedFood(state.food.begin(), state.food.end());
    set<Location> sortedAnts(state.myAnts.begin(), state.myAnts.end());
    set<Location> sortedHills(state.myHills.begin(), state.myHills.end());

    for (set<Location>::iterator locp = unseen.begin(); locp != unseen.end(); locp++) 
    {
	Square& square = state.grid[(*locp).row][(*locp).col];
	if (square.isVisible)
	{
	    unseen.erase(locp);
	}
    }

    // calculate distance from each ant to each food
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

    // assign ants to the closest food
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

    // explore unseen areas
    for (set<Location>::iterator antp = sortedAnts.begin();
	 antp != sortedAnts.end(); antp++)
    {
	if (!antsUsed.count(*antp))
	{
	    vector<Route> unseenRoutes;
	    for (set<Location>::iterator locp = unseen.begin();
		 locp != unseen.end(); locp++)
	    {
		double distance = state.distance(*antp, *locp);
		unseenRoutes.push_back(Route(*antp, *locp, distance));
	    }
	    sort(unseenRoutes.begin(), unseenRoutes.end());
	    for (vector<Route>::iterator routep = unseenRoutes.begin();
		 routep != unseenRoutes.end(); routep++)
	    {
		if (doMoveLocation((*routep).start, (*routep).end))
		{
		    antsUsed.insert((*routep).start);
		    break;
		}
	    }
	}
    }

    // have remaining ants stay where they are, so hill unblocking doesn't destroy ants
    for (set<Location>::iterator antp = sortedAnts.begin();
	 antp != sortedAnts.end(); antp++)
    {
	if (!antsUsed.count(*antp))
	{
	    if (!sortedHills.count(*antp)) 
	    {
		orders[*antp] = *antp;
		antsUsed.insert(*antp);
	    }
	}
    }

    // unblock hills
    for (set<Location>::iterator hillp = sortedHills.begin();
	 hillp != sortedHills.end(); hillp++)
    {
	if (sortedAnts.count(*hillp) && !antsUsed.count(*hillp))
	{
	    for (int d = 0; d < TDIRECTIONS; d++)
	    {
		if (doMoveDirection(*hillp, d))
		{
		    break;
		}
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
