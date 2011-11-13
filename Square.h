#ifndef SQUARE_H_
#define SQUARE_H_

#include <vector>
#include <iostream>

/*
    struct for representing a square in the grid.
*/
struct Square
{
    bool isVisible, isWater, isHill, isFood, isSeen;
    int ant, hillPlayer, lastSeen;
    std::vector<int> deadAnts;

    Square()
    {
        isVisible = isWater = isHill = isFood = isSeen = 0;
        ant = hillPlayer = -1;
    };

    //resets the information for the square except water information
    void reset()
    {
        isVisible = 0;
        isHill = 0;
        isFood = 0;
        ant = hillPlayer = -1;
        deadAnts.clear();
    };
};

std::ostream& operator<<(std::ostream& os, const Square &square);

#endif //SQUARE_H_
