#include <iostream>
#include "bot.h"

using namespace std;



int main(int argc, char **argv) {
  Bot bot;
  vector<int> moves(3, -1);
  do cout << moves << endl;
  while (bot.nextPermutation(moves));
  cout << moves << endl;
}
