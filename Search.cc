#include "Search.h"

Search::Search(const Location &start) : start(start), food(0), hills(0), unseen(0)
{
    expanded.insert(start);
    distances[start] = 0;
    predecessors[start] = start;
    remaining.push(start);
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

