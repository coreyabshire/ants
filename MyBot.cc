#include "bot.h"

int main(int argc, char *argv[]) {
  cout.sync_with_stdio(0); // this line makes your bot faster
  Bot bot;
  // reads the game parameters and sets up
  cin >> bot.state;
  bot.state.setup();
  bot.endTurn();
  // continues making moves while the game is not over
  while (cin >> bot.state) {
    bot.makeMoves();
    bot.endTurn();
  }
  return 0;
}
