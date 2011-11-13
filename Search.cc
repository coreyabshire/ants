#include "Search.h"

Search::Search(const Location &start) : start(start)
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

