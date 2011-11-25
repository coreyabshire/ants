#ifndef SQUARE_H_
#define SQUARE_H_

#include <vector>

/*
  struct for representing a square in the grid.
*/
struct Square
{
  bool isKnown, isVisible, isWater, isHill, isFood, isMovedTo, isFood2;
  double unknown, visible, water, home, target, food, enemy, buddy, aroma;
  int ant, hillPlayer;
  std::vector<int> deadAnts;

  Square()
  {
    isKnown = isVisible = isWater = isHill = isFood = isMovedTo = isFood2 = 0;
    unknown = visible = water = home = target = food = enemy = buddy = aroma = 0.0;
    ant = hillPlayer = -1;
  };

  //resets the information for the square except water information
  void reset()
  {
    isVisible = 0;
    isHill = 0;
    isFood2 = 0;
    isMovedTo = 0;
    unknown = visible = water = home = target = food = enemy = buddy = aroma = 0.0;
    ant = hillPlayer = -1;
    deadAnts.clear();
  };

};

#endif //SQUARE_H_
