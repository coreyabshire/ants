#include <iostream>
#include <iomanip>
#include "bot.h"
#include "state.h"

using namespace std;

class GridOut {
 public:
  State *state;
  GridOut(State *state) : state(state) {}
};

ostream &operator<<(ostream& os, const GridOut &go) {
  for (int r = 0; r < go.state->rows; r++) {
    for (int c = 0; c < go.state->cols; c++) {
      os << setiosflags(ios_base::fixed) << setprecision(2) << setw(5) << go.state->grid[r][c].inf[FOOD];
    }
    os << endl;
  }
  return os;
}

int main(int argc, char **argv) {
  Bot bot(20,20);
  State &state = bot.state;
  int iterations = atoi(argv[1]);
  state.putFood(4, 4);
  state.putFood(12, 12);
  state.putAnt(6,6,0);
  state.updateVisionInformation();
  state.updateInfluenceInformation(iterations);
  cout << GridOut(&state) << endl;
}
