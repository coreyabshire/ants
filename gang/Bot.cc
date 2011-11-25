#include "Bot.h"

using namespace std;

//constructor
Bot::Bot() {};

//plays a single game of Ants.
void Bot::playGame() {
  //reads the game parameters and sets up
  cin >> state;
  state.setup();
  endTurn();

  //continues making moves while the game is not over
  while(cin >> state) {
    state.updateVisionInformation();
    makeMoves();
    endTurn();
  }
}

void Bot::old() {
  for (int ant = 0; ant < state.myAnts.size(); ant++) {
    for (int d = 0; d < TDIRECTIONS; d++) {
      Location loc = state.getLocation(state.myAnts[ant], d);
      if (!state.grid[loc.row][loc.col].isWater) {
        state.makeMove(state.myAnts[ant], d);
        break;
      }
    }
  }
}

ostream& operator<<(ostream& os, const Location& a) { return os << a.row << "," << a.col << endl; }
Location average(vector<Location>::iterator begin, vector<Location>::iterator end) {
  Location average;
  int count = 0;
  for (vector<Location>::iterator p = begin; p != end; ++p, ++count) {
    average.row += (*p).row;
    average.col += (*p).col;
  }
  average.row /= count;
  average.col /= count;
  return average;
}

bool operator<(const Location &a, const Location &b) {
  return (a.row == b.row) ? (a.col < b.col) : (a.row < b.row);
}

bool operator==(const Location &a, const Location &b) {
  return a.row == b.row && a.col == b.col;
}

bool operator!=(const Location &a, const Location &b) {
  return a.row != b.row || a.col != b.col;
}

char getDirection(const Location &a, const Location &b) {
  return (a.row == b.row)
      ?  ((a.col < b.col) ? 'E' : 'W')
      :  ((a.row < b.row) ? 'S' : 'N');
}

string Bot::search(const Location &a, const Location &b) {
  set<Location> X;
  queue<Location> Q;
  map<Location,Location> P;
  map<Location,int> D;
  D[a] = 0;
  Q.push(a);
  while (!Q.empty()) {
    Location u = Q.front();
    Q.pop();
    if (u == b) {
      string path = string();
      Location p = b;
      for (Location p = b; p != a; p = P[p]) {
        path.push_back(getDirection(P[p], p));
      }
      reverse(path.begin(), path.end());
      return path;
    }
    for (int d = 0; d < TDIRECTIONS; ++d) {
      Location v = state.getLocation(u, d);
      Square &vs = state.grid[v.row][v.col];
      if (!X.count(v) && !vs.isWater) {
        P[v] = u;
        D[v] = D[u] + 1;
        X.insert(v);
        Q.push(v);
      }
    }
  }
  return "";
}

void Ant::order(string newmoves) {
  moves = newmoves;
  moveid = 0;
}

//makes the bots moves for the turn
void Bot::makeMoves() {
  state.bug << "turn " << state.turn << ":" << endl;
  state.bug << state << endl;

  vector<string> commands;
  state.bug << "ants size " << ants.size() << " " << state.antid << endl;
  for (int i = ants.size(); i < state.antid; ++i) {
    ants.push_back(Ant());
    state.bug << "added new ant " << i << endl;
  }

  if (state.turn == 3) {
    string path = search(state.myAnts[2], state.food[0]);
    state.bug << "path " << path << endl;
    ants[2].order(path);
  }

  if (state.turn == 20) {
    Location a(18,11);
    state.bug << "objective " << a << endl;
    for (vector<Location>::iterator p = state.myAnts.begin(); p != state.myAnts.end(); ++p) {
      Location u = *p;
      Square &us = state.grid[u.row][u.col];
      int id = us.id;
      string path = search(u, a);
      ants[id].order(path);
    }
  }
  
  if (state.turn == 42) {
    Location a(13,10);
    state.bug << "objective " << a << endl;
    for (vector<Location>::iterator p = state.myAnts.begin(); p != state.myAnts.end(); ++p) {
      Location u = *p;
      Square &us = state.grid[u.row][u.col];
      int id = us.id;
      string path = search(u, a);
      ants[id].order(path);
    }
  }
  
  if (state.turn == 10) {
    Location a = average(state.myAnts.begin(), state.myAnts.end());
    state.bug << "average " << a << endl;
    for (vector<Location>::iterator p = state.myAnts.begin(); p != state.myAnts.end(); ++p) {
      Location u = *p;
      Square &us = state.grid[u.row][u.col];
      int id = us.id;
      string path = search(u, a);
      ants[id].order(path);
    }
  }
  
  //picks out moves for each ant
  state.bug << "average " << average(state.myAnts.begin(), state.myAnts.end()) << endl;
  for (vector<Location>::iterator p = state.myAnts.begin(); p != state.myAnts.end(); ++p) {
    int id = state.grid[(*p).row][(*p).col].id;
    Ant &ant = ants[id];
    if (ant.moveid < ant.moves.size()) {
      char d = ant.moves.at(ant.moveid);
      Location u = *p;
      Location v = state.getLocation(u, d);
      Square &vs = state.grid[v.row][v.col];
      if (!vs.isWater && vs.ant == -1) {
        state.makeMove(u, d);
        ant.moveid++;
      }
      else {
        state.bug << "unable to move " << vs.isWater << " " << vs.ant << endl;
      }
    }
  }

  state.bug << "time taken: " << state.timer.getTime() << "ms" << endl << endl;
}

//finishes the turn
void Bot::endTurn() {
  if(state.turn > 0)
    state.reset();
  state.turn++;

  cout << "go" << endl;
}
