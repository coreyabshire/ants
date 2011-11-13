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
    for (int row = 0; row < state.rows; row += 5)
    {
	for (int col = 0; col < state.cols; col += 5)
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
    vector<Route> hillRoutes;
    map<Location,Location> foodAnts;
    set<Location> sortedFood(state.food.begin(), state.food.end());
    set<Location> sortedAnts(state.myAnts.begin(), state.myAnts.end());
    set<Location> sortedHills(state.myHills.begin(), state.myHills.end());
    map<Location, Search> searches;
    queue<Location> searchQueue;
    map<Location, Location> firstAnt;
    set<Location> remainingFood(state.food.begin(), state.food.end());
    set<Location> unassignedAnts(state.myAnts.begin(), state.myAnts.end());
    set<Location> assignedAnts;
    map<Location,int> distances;
    
    // add new hills to the set of all enemy hills
    for (vector<Location>::iterator hillp = state.enemyHills.begin();
	 hillp != state.enemyHills.end(); hillp++)
    {
	if (!enemyHills.count(*hillp))
	{
	    enemyHills.insert(*hillp);
	}
    }

    // remove destroyed hills from the list of hills
    for (set<Location>::iterator hillp = enemyHills.begin();
	 hillp != enemyHills.end(); hillp++)
    {
	if (sortedAnts.count(*hillp))
	{
	    enemyHills.erase(*hillp);
	}
    }

    // update set of unseen tiles
    for (set<Location>::iterator locp = unseen.begin(); locp != unseen.end(); locp++) 
    {
	Square& square = state.grid[(*locp).row][(*locp).col];
	if (square.isVisible)
	{
	    unseen.erase(locp);
	}
    }

    // initialize the search state
    for (vector<Location>::iterator antp = state.myAnts.begin();
	 antp != state.myAnts.end(); antp++)
    {
	searches[*antp] = Search(*antp);
	searchQueue.push(*antp);
	firstAnt[*antp] = *antp;
    }

    // search breadth first across all ants iteratively
    while (!searchQueue.empty())
    {
	Location& antLoc = searchQueue.front();
	Search &search = searches[antLoc];
	if (!search.remaining.empty())
	{
	    Location& u = search.remaining.front();

	    // If this location has one of the remaining food, then assign
	    // the current ant to collect it.  Ants may be assigned multiple
	    // food, but each food will only be assigned to one ant.
	    if (remainingFood.count(u))
	    {
		foodRoutes.push_back(Route(antLoc, u, search.distances[u]));
		remainingFood.erase(u);
		search.food++;
		unassignedAnts.erase(antLoc);
		assignedAnts.insert(antLoc);
	    }
	    
	    // If this location has an enemy hill, then assign the current
	    // ant to attack it unless it is already assigned to collect food.
	    if (enemyHills.count(u) && search.food == 0)
	    {
		hillRoutes.push_back(Route(antLoc, u, search.distances[u]));
		search.hills++;
	    }

	    // If there is already an ant at this location, then any hill
	    // or food we find will be closer to this ant, so it doesn't make
	    // sense to expand it any more. Instead, we may want to just
	    // move towards this ant to replace it if it moves.
	    if (sortedAnts.count(u))
	    {
	    }

	    for (int d = 0; d < TDIRECTIONS; d++)
	    {
		Location v = state.getLocation(u, d);
		if (!state.grid[v.row][v.col].isWater) 
		{
		    if (!search.expanded.count(v))
		    {
			int distance = search.distances[u] + 1;
			search.expanded.insert(v);
			search.distances[v] = distance;
			search.predecessors[v] = u;
			distances[u] = distance;
			search.remaining.push(v);
		    }
		}
	    }

	    search.remaining.pop();
	    if (!search.remaining.empty())
	    {
		if (!unassignedAnts.empty()) 
		{
		    if (!remainingFood.empty() || (search.food == 0 && search.hills == 0))
		    {
			searchQueue.push(antLoc);
		    }
		}
	    }
	    searchQueue.pop();
	}
    }

    // assign ants to the closest food
    sort(foodRoutes.begin(), foodRoutes.end());
    for (vector<Route>::iterator routep = foodRoutes.begin();
	 routep != foodRoutes.end(); routep++)
    {
	state.bug << "doing move from " << (*routep).start << " to " << (*routep).end << endl;
	if (!foodTargets.count((*routep).end) &&
	    !antsUsed.count((*routep).start) &&
	    doMoveLocation((*routep).start, searches[(*routep).start].step((*routep).end)))
	{
	    state.bug << "did move" << endl;
	    foodTargets.insert((*routep).end);
	    antsUsed.insert((*routep).start);
	}
    }

    // assign ants to destroy enemy hills
    sort(hillRoutes.begin(), hillRoutes.end());
    for (vector<Route>::iterator routep = hillRoutes.begin();
	 routep != hillRoutes.end(); routep++)
    {
	if (doMoveLocation((*routep).start, searches[(*routep).start].step((*routep).end)))
	{
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
		int distance = searches[*antp].distance(*locp);
		unseenRoutes.push_back(Route(*antp, *locp, distance));
	    }
	    sort(unseenRoutes.begin(), unseenRoutes.end());
	    for (vector<Route>::iterator routep = unseenRoutes.begin();
		 routep != unseenRoutes.end(); routep++)
	    {
		if (doMoveLocation((*routep).start, searches[(*routep).start].step((*routep).end)))
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

    state.bug << "time taken: " << state.timer.getTime() << endl << endl;
};

//finishes the turn
void Bot::endTurn()
{
    if(state.turn > 0)
        state.reset();
    state.turn++;
    cout << "go" << endl;
};
