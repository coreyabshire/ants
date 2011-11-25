#ifndef SQUARE_H_
#define SQUARE_H_

#include <vector>

/*
  struct for representing a square in the grid.
*/
enum { FOOD=0, TARGET=1, UNKNOWN=2, ENEMY=3 };
const int kFactors = 4;
struct Square {
  bool isVisible, isWater, isHill, isFood, isKnown, isFood2, isHill2;
  int ant, hillPlayer, hillPlayer2;
  float inf[kFactors];
  
  std::vector<int> deadAnts;

  Square() {
    isVisible = isWater = isHill = isFood = isKnown = 0;
    isFood2 = isHill2 = 0;
    for (int i = 0; i < kFactors; i++)
      inf[i] = 0.0;
    ant = hillPlayer = hillPlayer2 = -1;
  };

  //resets the information for the square except water information
  void reset() {
    isVisible = 0;
    isHill2 = 0;
    isFood2 = 0;
    //    for (int i = 0; i < kFactors; i++)
    //      inf[i] = 0.0;
    ant = hillPlayer2 = -1;
    deadAnts.clear();
  };
};

#endif //SQUARE_H_
