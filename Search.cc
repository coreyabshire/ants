#include "Search.h"

const int GRAY = 1;
const int BLACK = 2;
Search::Search(State &state, const Location &start) : start(start)
{
    std::map<Location,int> colors;
    std::queue<Location> q;
    colors[start] = GRAY;
    distances[start] = 0;
    predecessors[start] = start;
    q.push(start);
    while (!q.empty())
    {
	Location& u = q.front();
	for (int d = 0; d < TDIRECTIONS; d++)
	{
	    Location v = state.getLocation(u, d);
	    if (!state.grid[v.row][v.col].isWater) 
	    {
		if (!colors.count(v))
		{
		    colors[v] = GRAY;
		    distances[v] = distances[u] + 1;
		    predecessors[v] = u;
		    q.push(v);
		}
	    }
	}
	colors[u] = BLACK;
	q.pop();
    }
}

int Search::distance(const Location& dest)
{
    return distances[dest];
}

Location Search::step(const Location& dest)
{
    Location loc = dest;
    
    while (predecessors.count(loc) && predecessors[loc] != start)
    {
	loc = predecessors[loc];
    }

    return loc;
}

