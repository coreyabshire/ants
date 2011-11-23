#include "state.h"

Square::Square() {
  isVisible = isWater = isHill = isFood = isKnown = 0;
  isLefty = 0;
  isFood2 = isHill2 = 0;
  foodScent = 0.0;
  for (int i = 0; i < kFactors; i++)
    inf[i] = 0.0;
  direction = -1;
  ant = hillPlayer = hillPlayer2 = -1;
}

//resets the information for the square except water information
void Square::reset() {
  isVisible = isHill2 = isFood2 = 0;
  foodScent = 0.0;
  ant = hillPlayer2 = -1;
  deadAnts.clear();
}

void Square::markVisible(int turn) {
  isVisible = isKnown = 1;
  isFood = isFood2;
  isHill = isHill2;
  hillPlayer = hillPlayer2;
  lastSeen = turn;
}

float Square::influence() {
  float sum = 0.0;
  for (int i = 0; i < kFactors; i++) {
    sum += inf[i] * weights[i];
  }
  return sum;
}

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

  int n = max(rows, cols);
  bug << "setup grid for size " << n << endl;

  distance2Grid = vector< vector<int> >(n, vector<int>(n, 0));
  distanceGrid = vector< vector<double> >(n, vector<double>(n, 0.0));
                                                
  bug << "pre-calculating distances" << endl;
  for (int r = 0; r < n; r++) {
    for (int c = 0; c < n; c++) {
      int d = r * r + c * c;
      distance2Grid[r][c] = d;
      distanceGrid[r][c] = sqrt(d);
    }
  }

  bug << "pre-calculating offsets" << endl;
  for (int r = (1 - n); r < n; r++) {
    for (int c = (1 - n); c < n; c++) {
      int ar = abs(r), ac = abs(c);
      offsets.push_back(Offset(distanceGrid[ar][ac], distance2Grid[ar][ac], r, c));
    }
  }
  
  bug << "sorting offsets" << endl;
  sort(offsets.begin(), offsets.end());

  bug << "initial influence dump" << endl;
  dumpInfluenceInformation();
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

inline int addWrap(int a, int b, int max) {
  return (a + b + max) % max;
}

inline int diffWrap(int a, int b, int w) {
  int d = abs(a - b);
  return min(d, w - d);
}

inline Location State::addOffset(const Location &a, const Offset &o) {
  return Location(addWrap(a.row, o.r, rows), addWrap(a.col, o.c, cols));
}

inline int absdiff(int a, int b) {
  return a > b ? a - b : b - a;
}

// returns the euclidean distance between two locations with the edges wrapped
double State::distance(const Location &a, const Location &b) {
  int r = diffWrap(a.row, b.row, rows);
  int c = diffWrap(a.col, b.col, cols);
  return distanceGrid[r][c];
}

int State::distance2(const Location &a, const Location &b) {
  int r = diffWrap(a.row, b.row, rows);
  int c = diffWrap(a.col, b.col, cols);
  return distance2Grid[r][c];
}

// returns the euclidean distance between two locations with the edges wrapped
int State::manhattan(const Location &a, const Location &b) {
  int r = abs(a.row - b.row),
      c = abs(a.col - b.col),
      dr = min(r, rows - r),
      dc = min(c, cols - c);
  return dr + dc;
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

Location State::randomLocation() {
  return Location(rand() % rows, rand() % cols);
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
  if (!squareAt(a).isKnown) {
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
  for (vector<Location>::iterator p = myAnts.begin(); p != myAnts.end(); p++)
    for (vector<Offset>::iterator o = offsets.begin(); (*o).d2 <= viewradius2; o++)
      markVisible(addOffset(*p, *o));
}

void State::updateInfluenceInformation() {
  for (int i = 0; i < 10; i++) {
    vector< vector< vector<float> > > temp(rows, vector< vector<float> >(cols, vector<float>(kFactors, 0.0)));
    for (int r = 0; r < rows; r++) {
      for (int c = 0; c < cols; c++) {
        Location a(r,c);
        Square &as = grid[a.row][a.col];
        if (as.isWater) {
          for (int f = 0; f < kFactors; f++) {
            temp[r][c][f] = 0.0;
          }
        }
        else {
          if (as.isFood) {
            temp[r][c][FOOD] = 1.0;
          }
          if (as.hillPlayer > 0) {
            temp[r][c][TARGET] = 1.0;
          }
          if (!as.isKnown) {
            temp[r][c][UNKNOWN] = 1.0;
          }
          if (as.ant == 0) {
            temp[r][c][FOOD] *= 0.5;
            temp[r][c][UNKNOWN] *= 0.7;
          }
          for (int d = 0; d < TDIRECTIONS; d++) {
            Location b = getLocation(a, d);
            Square &bs = grid[b.row][b.col];
            for (int f = 0; f < kFactors; f++) {
              float g = bs.inf[f] * 0.97;
              if (g > temp[r][c][f]) {
                temp[r][c][f] = g;
              }
            }
          }
        }
      }
    }
    for (int r = 0; r < rows; r++) {
      for (int c = 0; c < cols; c++) {
        for (int f = 0; f < kFactors; f++) {
          grid[r][c].inf[f] = temp[r][c][f];
        }
      }
    }
  }
  dumpInfluenceInformation();
}

void State::dumpInfluenceInformation() {
  if (turn == 0) {
    bug << "inf rows " << rows << endl;
    bug << "inf cols " << cols << endl;
    bug << "inf factors " << kFactors + 1 << endl;
  }
  bug << "inf turn " << turn << endl;
  for (int f = 0; f < kFactors; f++) {
    bug << "inf factor " << f << endl;
    for (int r = 0; r < rows; r++) {
      bug << "inf row " << r;
      for (int c = 0; c < cols; c++) {
        bug << " " << grid[r][c].inf[f];
      }
      bug << endl;
    }
  }
  bug << "inf factor " << kFactors << endl;
  float weight = 0.0;
  for (int f = 0; f < kFactors; f++) {
    weight += weights[f];
  }
  for (int r = 0; r < rows; r++) {
    bug << "inf row " << r;
    for (int c = 0; c < cols; c++) {
      bug << " " << grid[r][c].influence() / weight;
    }
    bug << endl;
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
        state.grid[row][col].isFood2 = 1;
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
        state.grid[row][col].isHill2 = 1;
        state.grid[row][col].hillPlayer2 = player;
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
          is.setstate(ios::failbit);
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

ostream& operator<<(ostream& os, const Square &square) {
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

ostream& operator<<(ostream &os, const Location &loc) {
  return os << "(" << (int)loc.row << "," << (int)loc.col << ")";
}

ostream& operator<<(ostream& os, const Offset &o) {
  return os << o.d2 << " " << o.r << "," << o.c;
}

bool operator<(const Offset &a, const Offset &b) {
  return a.d2 == b.d2
      ? (a.r == b.r ? a.c < b.c : a.r < b.r)
      : (a.d2 < b.d2);
}

