#include "Route.h"

Route::Route(const Location& start, const Location& end, int distance) :
    start(start), end(end), distance(distance)
{
};

bool operator<(const Route &a, const Route &b)
{
    if (a.distance < b.distance)
	return true;
    else if (a.distance > b.distance)
	return false;
    else 
	if (a.start == b.start) 
	    return a.end < b.end;
	else
	    return a.start < b.start;
}

bool operator==(const Route &a, const Route &b)
{
    return a.distance == b.distance
	&& a.start == b.start
	&& a.end == b.end;
}
