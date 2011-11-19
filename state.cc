#include "state.h"

// constructor
State::State() : gameover(0), turn(0) {
  bug.open("./debug.txt");
}

// deconstructor
State::~State() {
  bug.close();
}

State::State(int rows, int cols) : gameover(0), turn(0), rows(rows), cols(cols) {
  bug.open("./debug.txt");
  setup();
}    

// sets the state up
void State::setup() {
  nSquares = nUnknown = rows * cols;
  nSeen = nVisible = 0;
  grid = vector< vector<Square> >(rows, vector<Square>(cols, Square()));
  calcOffsets(viewradius2, viewOffsets);
  calcOffsets(attackradius2, attackOffsets);
  calcOffsets(spawnradius2, spawnOffsets);
}

// resets all non-water squares to land and clears the bots ant vector
void State::reset() {
  nVisible = 0;
  myAnts.clear();
  enemyAnts.clear();
  myHills.clear();
  enemyHills.clear();
  food.clear();
  for(int row=0; row<rows; row++)
    for(int col=0; col<cols; col++)
      grid[row][col].reset();
}

// outputs move information to the engine
void State::makeMove(const Location &loc, int d) {
  cout << "o " << (int) loc.row << " " << (int) loc.col << " " << CDIRECTIONS[d] << endl;

  Location nLoc = getLocation(loc, d);
  grid[nLoc.row][nLoc.col].ant = grid[loc.row][loc.col].ant;
  grid[loc.row][loc.col].ant = -1;
}

// returns the euclidean distance between two locations with the edges wrapped
double State::distance(const Location &loc1, const Location &loc2) {
  int d1 = abs(loc1.row-loc2.row),
      d2 = abs(loc1.col-loc2.col),
      dr = min(d1, rows-d1),
      dc = min(d2, cols-d2);
  return sqrt(dr*dr + dc*dc);
}

int State::distance2(const Location &a, const Location &b) {
  int d1 = abs(a.row - b.row),
      d2 = abs(a.col - b.col),
      dr = min(d1, rows - d1),
      dc = min(d2, cols - d2);
  return dr * dr + dc * dc;
}

// returns the euclidean distance between two locations with the edges wrapped
int State::manhattan(const Location &a, const Location &b) {
  int r = abs(a.row - b.row),
      c = abs(a.col - b.col),
      dr = min(r, rows - r),
      dc = min(c, cols - c);
  return dr + dc;
}

void State::calcOffsets(int radius2, vector<Location> &offsets) {
  queue<Location> Q;
  Location a(0,0);
  Q.push(a);
  set<Location> visited;
  visited.insert(a);
  while (!Q.empty()) {
    Location u = Q.front();
    Q.pop();
    for (int d = 0; d < TDIRECTIONS; d++) {
      Location v = getLocationNoWrap(u, d);
      if (!visited.count(v) && distance2(a, v) <= radius2) {
        offsets.push_back(v);
        Q.push(v);
      }
      visited.insert(v);
    }
  }
}

// returns the new location from moving in a given direction with the edges wrapped
Location State::getLocationNoWrap(const Location &loc, int direction) {
  return Location( (loc.row + DIRECTIONS[direction][0]),
                   (loc.col + DIRECTIONS[direction][1]));
}

// returns the new location from moving in a given direction with the edges wrapped
Location State::getLocation(const Location &loc, int direction) {
  return Location( (loc.row + DIRECTIONS[direction][0] + rows) % rows,
                   (loc.col + DIRECTIONS[direction][1] + cols) % cols );
}

Location State::getLocation(const Location &loc, const Location &off) {
  return Location( (loc.row + off.row + rows) % rows,
                   (loc.col + off.col + cols) % cols );
}

vector<int> State::getDirections(const Location &a, const Location &b) {
  vector<int> directions;
  if (a.row < b.row)
    if (b.row - a.row >= rows / 2)
      directions.push_back(NORTH);
    else
      directions.push_back(SOUTH);
  else if (a.row > b.row)
    if (a.row - b.row >= rows / 2)
      directions.push_back(SOUTH);
    else
      directions.push_back(NORTH);
  if (a.col < b.col)
    if (b.col - a.col >= cols / 2)
      directions.push_back(WEST);
    else
      directions.push_back(EAST);
  else if (a.col > b.col)
    if (a.col - b.col >= cols / 2)
      directions.push_back(EAST);
    else
      directions.push_back(WEST);
  return directions;
}

void State::markVisible(const Location& a) {
  if (!squareAt(a).isSeen) {
    --nUnknown;
    ++nSeen;
  }
  if (!squareAt(a).isVisible) {
    ++nVisible;
  }
  squareAt(a).markVisible(turn);
}

// This function will update the lastSeen value for any squares currently
// visible by one of your live ants.
void State::updateVisionInformation() {
  for (vector<Location>::iterator p = myAnts.begin(); p != myAnts.end(); p++) {
    markVisible(*p);
    for (vector<Location>::iterator o = viewOffsets.begin(); o != viewOffsets.end(); o++)
      markVisible(getLocation(*p, *o));
  }
}

// This is the output function for a state. It will add a char map
// representation of the state to the output stream passed to it.
ostream& operator<<(ostream &os, const State &state) {
  for(int row=0; row<state.rows; row++) {
    for(int col=0; col<state.cols; col++) {
      if(state.grid[row][col].isWater)
        os << '%';
      else if(state.grid[row][col].isFood)
        os << '*';
      else if(state.grid[row][col].isHill)
        os << (char)('A' + state.grid[row][col].hillPlayer);
      else if(state.grid[row][col].ant >= 0)
        os << (char)('a' + state.grid[row][col].ant);
      else if(state.grid[row][col].isVisible)
        os << '.';
      else
        os << '?';
    }
    os << endl;
  }

  return os;
}

//input function
istream& operator>>(istream &is, State &state) {
  int row, col, player;
  string inputType, junk;

  //finds out which turn it is
  while(is >> inputType) {
    if(inputType == "end") {
      state.gameover = 1;
      break;
    }
    else if(inputType == "turn") {
      is >> state.turn;
      break;
    }
    else { //unknown line
      getline(is, junk);
    }
  }

  if(state.turn == 0) {
    //reads game parameters
    while(is >> inputType) {
      if(inputType == "loadtime")
        is >> state.loadtime;
      else if(inputType == "turntime")
        is >> state.turntime;
      else if(inputType == "rows")
        is >> state.rows;
      else if(inputType == "cols")
        is >> state.cols;
      else if(inputType == "turns")
        is >> state.turns;
      else if(inputType == "player_seed")
        is >> state.seed;
      else if(inputType == "viewradius2") {
        is >> state.viewradius2;
        state.viewradius = sqrt(state.viewradius2);
      } 
      else if(inputType == "attackradius2") {
        is >> state.attackradius2;
        state.attackradius = sqrt(state.attackradius2);
      } 
      else if(inputType == "spawnradius2") {
        is >> state.spawnradius2;
        state.spawnradius = sqrt(state.spawnradius2);
      }
      else if(inputType == "ready") {
        state.timer.start();
        break;
      }
      else    //unknown line
        getline(is, junk);
    }
  }
  else {
    //reads information about the current turn
    while(is >> inputType) {
      if(inputType == "w") { //water square
        is >> row >> col;
        state.grid[row][col].isWater = 1;
      }
      else if(inputType == "f") { //food square
        is >> row >> col;
        state.grid[row][col].isFood = 1;
        state.food.push_back(Location(row, col));
      }
      else if(inputType == "a") { //live ant square
        is >> row >> col >> player;
        state.grid[row][col].ant = player;
        if(player == 0)
          state.myAnts.push_back(Location(row, col));
        else
          state.enemyAnts.push_back(Location(row, col));
      }
      else if(inputType == "d") { //dead ant square
        is >> row >> col >> player;
        state.grid[row][col].deadAnts.push_back(player);
      }
      else if(inputType == "h") {
        is >> row >> col >> player;
        state.grid[row][col].isHill = 1;
        state.grid[row][col].hillPlayer = player;
        if(player == 0)
          state.myHills.push_back(Location(row, col));
        else
          state.enemyHills.push_back(Location(row, col));
      }
      else if(inputType == "players") { //player information
        is >> state.noPlayers;
      }
      else if(inputType == "scores") { //score information
        state.scores = vector<double>(state.noPlayers, 0.0);
        for(int p=0; p<state.noPlayers; p++)
          is >> state.scores[p];
      }
      else if(inputType == "go") { //end of turn input
        if(state.gameover)
          is.setstate(std::ios::failbit);
        else
          state.timer.start();
        break;
      }
      else { //unknown line
        getline(is, junk);
      }
    }
  }

  return is;
}

Route::Route(const Location& start, const Location& end, map<Location,Location> &p)
    : start(start), end(end), distance(0) {
  for (Location c = end; c != start; c = p[c]) {
    steps.push_front(c);
    distance++;
  }
  steps.push_front(start);
  distance++;
};

void Route::flip() {
  Location temp = start;
  start = end;
  end = temp;
  reverse(steps.begin(), steps.end());
}

bool operator<(const Route &a, const Route &b) {
  return a.distance < b.distance;
}

bool operator==(const Route &a, const Route &b) {
  return a.distance == b.distance
      && a.start == b.start
      && a.end == b.end;
}

ostream& operator<<(ostream& os, const Route &r) {
  os << r.start << " " << r.end << " " << r.steps.size();
}

std::ostream& operator<<(std::ostream& os, const Square &square) {
  if (square.isVisible)
    os << "V";
  if (square.isWater)
    os << "W";
  if (square.isHill)
    os << "H";
  if (square.isFood)
    os << "F";
  if (square.ant >= 0)
    os << "," << square.ant;
  if (square.hillPlayer >= 0)
    os << "," << square.hillPlayer;
}

bool operator<(const Location &a, const Location &b) {
  return a.col == b.col ? a.row < b.row : a.col < b.col;
}

bool operator==(const Location &a, const Location &b) {
  return a.row == b.row && a.col == b.col;
}

bool operator!=(const Location &a, const Location &b) {
  return a.row != b.row || a.col != b.col;
}

std::ostream& operator<<(std::ostream &os, const Location &loc) {
  return os << "(" << (int)loc.row << "," << (int)loc.col << ")";
}
