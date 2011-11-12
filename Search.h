#ifndef SEARCH_H_
#define SEARCH_H_

#include <map>
#include "Location.h"
#include "State.h"

struct Search
{
    std::map<Location,int> d;
    std::map<Location,Location> p;
    Search(State &state, const Location &s);
    int distance(const Location& dest);
    Location step(State &state, const Location& start, const Location& dest);
};


#endif //SEARCH_H_
