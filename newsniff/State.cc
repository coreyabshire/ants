#include "State.h"

using namespace std;

//constructor
State::State() {
  gameover = 0;
  turn = 0;
  bug.open("./debug.txt");
};

//deconstructor
State::~State() {
  bug.close();
};

bool operator<(const Offset &a, const Offset &b) {
  return a.d2 == b.d2
      ? (a.r == b.r ? a.c < b.c : a.r < b.r)
      : (a.d2 < b.d2);
}

ostream& operator<<(ostream& os, const Offset &o) {
  return os << o.d2 << " " << o.r << "," << o.c;
}

//sets the state up
void State::setup() {
  const int n = 100;

  grid = vector<vector<Square> >(rows, vector<Square>(cols, Square()));
  distanceSquaredLookup = vector< vector<int> >(n, vector<int>(n, 0));
  distanceLookup = vector< vector<double> >(n, vector<double>(n, 0.0));
                                                
  bug << "pre-calculating distances" << endl;
  for (int r = 0; r < n; r++) {
    for (int c = 0; c < n; c++) {
      double d = r * r + c * c;
      distanceSquaredLookup[r][c] = d;
      distanceLookup[r][c] = sqrt(d);
    }
  }

  bug << "pre-calculating offsets" << endl;
  for (int r = (1 - n); r < n; r++) {
    for (int c = (1 - n); c < n; c++) {
      int ar = abs(r), ac = abs(c);
      offsets.push_back(Offset(distanceLookup[ar][ac], distanceSquaredLookup[ar][ac], r, c));
    }
  }
  
  bug << "sorting offsets" << endl;
  sort(offsets.begin(), offsets.end());
  
  dumpInfluenceInformation();

  bug << "finished setup" << endl;
};

//resets all non-water squares to land and clears the bots ant vector
void State::reset() {
  myAnts.clear();
  enemyAnts.clear();
  myHills.clear();
  enemyHills.clear();
  food.clear();
  for(int row=0; row<rows; row++)
    for(int col=0; col<cols; col++)
      if(!grid[row][col].isWater)
        grid[row][col].reset();
};

//outputs move information to the engine
void State::makeMove(const Location &loc, int direction) {
  cout << "o " << loc.row << " " << loc.col << " " << CDIRECTIONS[direction] << endl;

  Location nLoc = getLocation(loc, direction);
  grid[nLoc.row][nLoc.col].ant = grid[loc.row][loc.col].ant;
  grid[loc.row][loc.col].ant = -1;
};

//returns the euclidean distance between two locations with the edges wrapped
double State::distance(const Location &loc1, const Location &loc2) {
  int d1 = abs(loc1.row-loc2.row),
      d2 = abs(loc1.col-loc2.col),
      dr = min(d1, rows-d1),
      dc = min(d2, cols-d2);
  return sqrt(dr*dr + dc*dc);
};

//returns the new location from moving in a given direction with the edges wrapped
Location State::getLocation(const Location &loc, int direction) {
  return Location( (loc.row + DIRECTIONS[direction][0] + rows) % rows,
                   (loc.col + DIRECTIONS[direction][1] + cols) % cols );
};

/*
  This function will update update the lastSeen value for any squares currently
  visible by one of your live ants.

  BE VERY CAREFUL IF YOU ARE GOING TO TRY AND MAKE THIS FUNCTION MORE EFFICIENT,
  THE OBVIOUS WAY OF TRYING TO IMPROVE IT BREAKS USING THE EUCLIDEAN METRIC, FOR
  A CORRECT MORE EFFICIENT IMPLEMENTATION, TAKE A LOOK AT THE GET_VISION FUNCTION
  IN ANTS.PY ON THE CONTESTS GITHUB PAGE.
*/
// void State::updateVisionInformation() {
//   queue<Location> locQueue;
//   Location sLoc, cLoc, nLoc;
//   for (int a=0; a<(int) myAnts.size(); a++) {
//     sLoc = myAnts[a];
//     locQueue.push(sLoc);
//     vector< vector<bool> > visited(rows, vector<bool>(cols, 0));
//     grid[sLoc.row][sLoc.col].isVisible = 1;
//     visited[sLoc.row][sLoc.col] = 1;
//     while(!locQueue.empty()) {
//       cLoc = locQueue.front();
//       locQueue.pop();
//       for(int d=0; d<TDIRECTIONS; d++) {
//         nLoc = getLocation(cLoc, d);
//         if(!visited[nLoc.row][nLoc.col] && distance(sLoc, nLoc) <= viewradius) {
//           grid[nLoc.row][nLoc.col].isVisible = 1;
//           grid[nLoc.row][nLoc.col].isKnown = 1;
//           locQueue.push(nLoc);
//         }
//         visited[nLoc.row][nLoc.col] = 1;
//       }
//     }
//   }
// };
inline int State::addWrap(int a, int b, int max) {
  return (a + b + max) % max;
}

inline Location State::addOffset(const Location &a, const Offset &o) {
  return Location(addWrap(a.row, o.r, rows), addWrap(a.col, o.c, cols));
}

ostream& operator<<(ostream& os, const Location &a) { return os << a.row << "," << a.col; }

void State::updateVisionInformation() {
  for (vector<Location>::iterator a = myAnts.begin(); a < myAnts.end(); a++) {
    for (vector<Offset>::iterator o = offsets.begin(); (*o).d2 <= viewradius2; o++) {
      Location b = addOffset(*a, *o);
      Square &s = grid[b.row][b.col];
      s.isVisible = 1;
      s.isKnown = 1;
      if (s.isFood != s.isFood2)
        s.isFood = s.isFood2;
      if (s.isHill != s.isHill2)
        s.isHill = s.isHill2;
      if (s.hillPlayer != s.hillPlayer2)
        s.hillPlayer = s.hillPlayer2;
    }
  }
};

void State::dumpInfluenceInformation() {
  if (turn == 0) {
    bug << "inf rows " << rows << endl;
    bug << "inf cols " << cols << endl;
    bug << "inf factors " << kFactors << endl;
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
}

void State::updateInfluenceInformation() {
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      Location a(r,c);
      Square &as = grid[a.row][a.col];
      for (int f = 0; f < kFactors; f++) {
        //        as.inf[f] = 0.0;
      }
    }
  }
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
    float total_weight = 0.0;
    for (int f = 0; f < 3; f++) {
      total_weight += weights[f];
    }
    for (int r = 0; r < rows; r++) {
      for (int c = 0; c < cols; c++) {
        float t = 0.0;
        for (int f = 0; f < 3; f++) {
          grid[r][c].inf[f] = temp[r][c][f];
          t += temp[r][c][f] * weights[f];
        }
        grid[r][c].inf[3] = t / total_weight;
      }
    }
  }
  // bug.file.width(3);
  // for (int r = 0; r < rows; r++) {
  //   for (int c = 0; c < cols; c++) {
  //     Location a(r,c);
  //     Square &as = grid[a.row][a.col];
  //     bug << as.inf[FOOD] << " ";
  //   }
  //   bug << endl;
  // }
}

/*
  This is the output function for a state. It will add a char map
  representation of the state to the output stream passed to it.

  For example, you might call "cout << state << endl;"
*/
ostream& operator<<(ostream &os, const State &state)
{
  for(int row=0; row<state.rows; row++)
  {
    for(int col=0; col<state.cols; col++)
    {
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
};

//input function
istream& operator>>(istream &is, State &state)
{
  int row, col, player;
  string inputType, junk;

  //finds out which turn it is
  while(is >> inputType)
  {
    if(inputType == "end")
    {
      state.gameover = 1;
      break;
    }
    else if(inputType == "turn")
    {
      is >> state.turn;
      break;
    }
    else //unknown line
      getline(is, junk);
  }

  if(state.turn == 0)
  {
    //reads game parameters
    while(is >> inputType)
    {
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
      else if(inputType == "viewradius2")
      {
        is >> state.viewradius2;
        state.viewradius = sqrt(state.viewradius2);
      }
      else if(inputType == "attackradius2")
      {
        is >> state.attackradius2;
        state.attackradius = sqrt(state.attackradius2);
      }
      else if(inputType == "spawnradius2")
      {
        is >> state.spawnradius2;
        state.spawnradius = sqrt(state.spawnradius2);
      }
      else if(inputType == "ready") //end of parameter input
      {
        state.timer.start();
        break;
      }
      else    //unknown line
        getline(is, junk);
    }
  }
  else
  {
    //reads information about the current turn
    while(is >> inputType)
    {
      if(inputType == "w") //water square
      {
        is >> row >> col;
        state.grid[row][col].isWater = 1;
      }
      else if(inputType == "f") //food square
      {
        is >> row >> col;
        state.grid[row][col].isFood = 1;
        state.grid[row][col].isFood2 = 1;
        state.food.push_back(Location(row, col));
      }
      else if(inputType == "a") //live ant square
      {
        is >> row >> col >> player;
        state.grid[row][col].ant = player;
        if(player == 0)
          state.myAnts.push_back(Location(row, col));
        else
          state.enemyAnts.push_back(Location(row, col));
      }
      else if(inputType == "d") //dead ant square
      {
        is >> row >> col >> player;
        state.grid[row][col].deadAnts.push_back(player);
      }
      else if(inputType == "h")
      {
        is >> row >> col >> player;
        state.grid[row][col].isHill = 1;
        state.grid[row][col].isHill2 = 1;
        state.grid[row][col].hillPlayer = player;
        state.grid[row][col].hillPlayer2 = player;
        if(player == 0)
          state.myHills.push_back(Location(row, col));
        else
          state.enemyHills.push_back(Location(row, col));

      }
      else if(inputType == "players") //player information
        is >> state.noPlayers;
      else if(inputType == "scores") //score information
      {
        state.scores = vector<double>(state.noPlayers, 0.0);
        for(int p=0; p<state.noPlayers; p++)
          is >> state.scores[p];
      }
      else if(inputType == "go") //end of turn input
      {
        if(state.gameover)
          is.setstate(ios::failbit);
        else
          state.timer.start();
        break;
      }
      else //unknown line
        getline(is, junk);
    }
  }

  return is;
};
