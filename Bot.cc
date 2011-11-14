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
    for (int row = 0; row < state.rows; row += 4)
    {
	for (int col = 0; col < state.cols; col += 4)
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
	orders.insert(newLoc);
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

bool Bot::doMoveRoutes(vector<Route>& routes, map<Location, Search> &searches, set<Location> &antsUsed)
{
    sort(routes.begin(), routes.end());
    for (vector<Route>::iterator r = routes.begin(); r != routes.end(); r++)
    {
	if (!antsUsed.count((*r).start) &&
	    doMoveLocation((*r).start, searches[(*r).start].step((*r).end)))
	{
	    antsUsed.insert((*r).start);
	}
    }
}

inline bool isVisibleAndNotHill(Square &s) { return s.isVisible && !s.isHill; }
inline bool isVisibleAndNotFood(Square &s) { return s.isVisible && !s.isFood; }
inline bool isVisible(Square &s) { return s.isVisible; }

void Bot::insertAll(set<Location> &to, vector<Location> &from)
{
    to.insert(from.begin(), from.end());
}

void Bot::removeIf(set<Location> &locs, bool(*pred)(Square &))
{
    for (set<Location>::iterator p = locs.begin(); p != locs.end();)
	if (pred(state.grid[(*p).row][(*p).col]))
	    locs.erase(*p++);
	else
	    ++p;
}

void Bot::updateMemory(set<Location> &memory, vector<Location> &seen, bool(*pred)(Square &))
{
    insertAll(memory, seen);
    removeIf(memory, pred);
}

//makes the bots moves for the turn
void Bot::makeMoves()
{
    orders.clear();

    // keep ants from moving onto our own hills and preventing spawning
    insertAll(orders, state.myHills);
    
    state.bug << "turn " << state.turn << ":" << endl;
    state.bug << state << endl;
    state.bug << "unseen " << unseen.size() << endl;

    updateMemory(enemyHills, state.enemyHills, isVisibleAndNotHill);
    updateMemory(food, state.food, isVisibleAndNotFood);
    removeIf(unseen, isVisible);

    set<Location> antsUsed;
    vector<Route> foodRoutes;
    vector<Route> hillRoutes;
    vector<Route> unseenRoutes;
    map<Location,Location> foodAnts;
    set<Location> sortedFood(food.begin(), food.end());
    set<Location> sortedAnts(state.myAnts.begin(), state.myAnts.end());
    set<Location> sortedHills(state.myHills.begin(), state.myHills.end());
    map<Location, Search> searches;
    set<Location> remainingFood(food.begin(), food.end());
    set<Location> remainingUnseen(unseen.begin(), unseen.end());
    set<Location> unassignedAnts(state.myAnts.begin(), state.myAnts.end());
    map<Location,int> distances;
    deque<Location> searchQueue(state.myAnts.begin(), state.myAnts.end());
    
    // initialize the search state
    for (vector<Location>::iterator p = state.myAnts.begin(); p != state.myAnts.end(); p++)
    {
	searches[*p] = Search(*p);
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
	    if (remainingFood.count(u) && search.hills == 0)
	    {
		foodRoutes.push_back(Route(antLoc, u, search.distances[u]));
		remainingFood.erase(u);
		search.food++;
		unassignedAnts.erase(antLoc);
	    }
	    
	    // If this location has an enemy hill, then assign the current
	    // ant to attack it unless it is already assigned to collect food.
	    if (enemyHills.count(u) && search.food == 0)
	    {
		hillRoutes.push_back(Route(antLoc, u, search.distances[u]));
		search.hills++;
	    }

	    // If this location has not been seen by any ant, then assign
	    // the current ant to try to see it.
	    if (remainingUnseen.count(u) && search.food == 0 && search.hills == 0)
	    {
		unseenRoutes.push_back(Route(antLoc, u, search.distances[u]));
		remainingUnseen.erase(u);
		search.unseen++;
	    }

	    // If there is already an ant at this location, then any hill
	    // or food we find will be closer to this ant, so it doesn't make
	    // sense to expand it any more. Instead, we may want to just
	    // move towards this ant to replace it if it moves.
	    if (sortedAnts.count(u))
	    {
	    }

	    // Expand this location for this ant, so that the neighboring
	    // locations are inspected the next time its this ants turn.
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
		    if (!remainingFood.empty() || (search.food == 0 && search.hills == 0 && search.unseen == 0))
		    {
			searchQueue.push_back(antLoc);
		    }
		}
	    }
	    searchQueue.pop_front();
	}
    }

    // assign ants to the closest food
    doMoveRoutes(foodRoutes, searches, antsUsed);

    // assign ants to destroy enemy hills
    doMoveRoutes(hillRoutes, searches, antsUsed);

    // explore unseen areas
    doMoveRoutes(unseenRoutes, searches, antsUsed);

    state.bug << "route counts: "
	      << foodRoutes.size() << " "
	      << hillRoutes.size() << " "
	      << unseenRoutes.size() << endl;

    state.bug << "ant count 1: " << antsUsed.size() << " of " << sortedAnts.size() << endl;
    
    // have remaining ants stay where they are, so hill unblocking doesn't destroy ants
    for (set<Location>::iterator p = sortedAnts.begin(); p != sortedAnts.end(); p++)
    {
	if (!antsUsed.count(*p))
	{
	    if (!sortedHills.count(*p)) 
	    {
		orders.insert(*p);
		antsUsed.insert(*p);
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
