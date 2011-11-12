#include "Search.h"

const int GRAY = 1;
const int BLACK = 2;
Search::Search(State &state, const Location &s)
{
    std::map<Location,int> c;
    std::queue<Location> q;
    c[s] = GRAY;
    d[s] = 0;
    p[s] = s;
    q.push(s);
    while (!q.empty())
    {
	Location& u = q.front();
	for (int dir = 0; dir < TDIRECTIONS; dir++)
	{
	    Location v = state.getLocation(u, dir);
	    if (!state.grid[v.row][v.col].isWater) 
	    {
		if (!c.count(v))
		{
		    c[v] = GRAY;
		    d[v] = d[u] + 1;
		    p[v] = u;
		    q.push(v);
		}
	    }
	}
	c[u] = BLACK;
	q.pop();
    }
}

int Search::distance(const Location& dest)
{
    return d[dest];
}

Location Search::step(State& state, const Location& start, const Location& dest)
{
    Location loc = dest;
    int c = 0;
    while (!(p[loc] == start) && c < 200)
    {
	state.bug << "step - s: " << start << ", loc: " << loc << std::endl;
	loc = p[loc];
	c++;
    }
    state.bug << "next step from " << start << " is " << loc << std::endl;
    return loc;
}

