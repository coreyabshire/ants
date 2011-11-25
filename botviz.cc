#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include "visualizer.h"
#include "state.h"
#include "bot.h"

using namespace std;

const int kMaxHeight = 800;
int kMult = 5;
int rows, cols, players;
const int factors = 4;
vector< vector< vector<float> > > inf;
vector< string > lines;
const int dirs[4][2] = { {-1, 0}, {0, 1}, {1, 0}, {0, -1} };
unsigned int timeDelay = 1000/30;
Bot *bot;
vector< vector<Square> > *grid;
bool needEndTurn = false;
int factor1 = 0;
int factor2 = 0;
int factor3 = 0;

class MySim : public Sim {
 public:
  MySim() {};
  virtual void makeMove(const Location &a, int d);
  virtual void go();
};

void MySim::makeMove(const Location &a, int d) {
  //  cout << "o " << (int) a.row << " " << (int) a.col << " " << CDIRECTIONS[d] << endl;
  vector< vector<Square> > &realgrid = *grid;
  Location b = bot->state.getLocation(a, d);
  realgrid[a.row][a.col].ant = -1;
  realgrid[b.row][b.col].ant = 0;
}

void MySim::go() {
  State& state = bot->state;
  vector< vector<Square> > &realgrid = *grid;
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      if (realgrid[r][c].isFood) {
        Location a(r, c);
        for (vector<Offset>::iterator o = state.offsetFirst; o != state.spawnEnd; o++) {
          Location b = state.addOffset(a, *o);
          if (realgrid[b.row][b.col].ant == 0) {
            realgrid[r][c].isFood = 0;
            cout << "detected consumption" << endl;
          }
        }
      }
    }
  }
  //  cout << "go" << endl;
}

void tile(float x, float y, float r, float g, float b) {
  glColor3f(r, g, b);
  glBegin(GL_POLYGON);
  glVertex3f(x, y, 0.0);
  glVertex3f(x + 1.0, y, 0.0);
  glVertex3f(x + 1.0, y + 1.0, 0.0);
  glVertex3f(x, y + 1.0, 0.0);
  glEnd();
}

float randf() {
  return (float)rand()/(float)RAND_MAX;
}

void blur(void) {
  for (int i = 0; i < 2; i++) {
    vector< vector< vector<float> > > temp(rows, vector< vector<float> >(cols, vector<float>(4, 0.0)));
    for (int r = 0; r < rows; r++) {
      for (int c = 0; c < cols; c++) {
        if (lines[r][c] == '%') {
          temp[r][c][0] = 0.0;
        }
        else {
          if (lines[r][c] == '*') {
            temp[r][c][0] = 1.0;
          }
          if (lines[r][c] == 'a' || lines[r][c] == 'A') {
            temp[r][c][1] = 1.0;
          }
          if (lines[r][c] == 'b' || lines[r][c] == 'B') {
            temp[r][c][2] = 1.0;
          }
          else {
            for (int f = 0; f < factors; f++) {
              for (int d = 0; d < 4; d++) {
                int dr = (r + dirs[d][0] + rows) % rows;
                int dc = (c + dirs[d][1] + cols) % cols;
                float h = inf[dr][dc][f] * 0.98;
                if (h > temp[r][c][f])
                  temp[r][c][f] = h;
              }
            }
          }
        }
      }
    }
    for (int r = 0; r < rows; r++) {
      for (int c = 0; c < cols; c++) {
        for (int f = 0; f < factors; f++) {
          inf[r][c][f] = temp[r][c][f];
        }
      }
    }
  }
}

void redraw(void) {
  blur();
}

void keyboard(unsigned char k, int x, int y) {
  switch (k) {
    case 's':
      if (factor1 < kFactors - 1)
        ++factor1;
      break;
    case 'x':
      if (factor1 > 0)
        --factor1;
      break;
    case 'd':
      if (factor2 < kFactors - 1)
        ++factor2;
      break;
    case 'c':
      if (factor2 > 0)
        --factor2;
      break;
    case 'f':
      if (factor3 < kFactors - 1)
        ++factor3;
      break;
    case 'v':
      if (factor3 > 0)
        --factor3;
      break;
    default:
      cout << "keyboard " << k << " " << x << " " << y << endl;
      break;
  }
  cout << " factors " << factor1 << ", " << factor2 << ", " << factor3 << endl;
  glutPostRedisplay();
}

void timer(int value) {
  State &state = bot->state;
  vector< vector<Square> > &realgrid = *grid;
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      if (realgrid[r][c].isWater) {
        state.grid[r][c].isWater = 1;
      }
      else {
        if (rand() % 100000 < 2) {
          realgrid[r][c].isFood = 1;
        }
      }
      if (realgrid[r][c].isFood) {
        state.grid[r][c].isFood = 1;
        state.grid[r][c].isFood2 = 1;
        state.food.push_back(Location(r, c));
      }
      if (realgrid[r][c].ant == 0) {
        //    cout << "ant at " << r << "," << c << endl;
        state.grid[r][c].ant = 0;
        state.myAnts.push_back(Location(r, c));
      }
    }
  }
  state.updateVisionInformation();
  state.updateInfluenceInformation();
  //  cout << "making moves" << endl;
  bot->makeMoves();
  bot->endTurn();
  //redraw();
  glutPostRedisplay();
  glutTimerFunc(timeDelay, timer, value);
}

void idle(void) {
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      for (int f = 0; f < factors; f++) {
        inf[r][c][f] -= 0.0001;
      }
    }
  }
  redraw();
  glutPostRedisplay();
}

void mouse(int button, int mstate, int x, int y) {
  int r = (int)((float)y / (float)kMult);
  int c = (int)((float)x / (float)kMult);
  if (mstate != GLUT_DOWN) {
    //cout << "mouse " << button << " " << mstate << " " << x << "," << y << " " << r << " " << c << endl;
    return;
  }
  //cout << "mouse " << button << " " << x << "," << y << " " << r << " " << c << endl;
  State& state = bot->state;
  vector< vector<Square> > &realgrid = *grid;
  if (r < rows && c < cols) {
    Square &s = realgrid[r][c];
    switch (button) {
      case GLUT_LEFT_BUTTON:
        s.isFood = !s.isFood;
        break;
      case GLUT_MIDDLE_BUTTON:
        s.ant = s.ant == 1 ? -1 : s.ant == -1 ? 1 : s.ant;
        break;
      case GLUT_RIGHT_BUTTON:
        s.ant = s.ant == 0 ? -1 : s.ant == -1 ? 0 : s.ant;
        break;
    }
  }
  redraw();
}

void display(void) {
  vector< vector<Square> > &realgrid = *grid;
  State &state = bot->state;
  glClear(GL_COLOR_BUFFER_BIT);
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      Square &s = realgrid[r][c];
      float f0 = 0.0, f1 = 0.0, f2 = 0.0;
      if (s.isWater)
        tile(c, r, 0.0, 0.0, 0.0);
      else if (s.ant == 0)
        tile(c, r, 0.0, 1.0, 0.0);
      else if (s.isFood)
        tile(c, r, 0.0, 0.0, 1.0);
      else {
        float f1 = state.grid[r][c].inf[UNKNOWN];
        float f2 = state.grid[r][c].inf[VISIBLE];
        float f3 = state.grid[r][c].inf[FOOD];
        tile(c, r, f1 * 0.4, 0.2 *f2 + 0.1 , f3 * 1.0);
      }
    }
  }
  glFlush();
  glutSwapBuffers();

}

void init() {
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, cols, rows, 0.0, -1.0, 1.0);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    cout << "need filename" << endl;
    return 1;
  }
  ifstream file(argv[1], ifstream::in);
  string type, line;
  if (file.fail()) {
    cout << "unable to open file: " << argv[1] << endl;
    return 1;
  }
  while (file.good()) {
    file >> type;
    if (type == "rows") {
      file >> rows;
    }
    else if (type == "cols") {
      file >> cols;
    }
    else if (type == "players") {
      file >> players;
    }
    else if (type == "m") {
      file >> line;
      lines.push_back(line);
    }
    else {
      cout << "unrecognized: " << type << endl;
    }
  }
  file.close();
  cout << "read map: " << rows << " x " << cols << endl;
  inf = vector< vector< vector<float> > >(rows, vector< vector<float> >(cols, vector<float>(4, 0.0)));
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      for (int f = 0; f < factors; f++) {
        inf[r][c][f] = 0.0;
      }
    }
  }
  redraw();
  kMult = kMaxHeight / rows;
  int width = kMult * cols;
  int height = kMult * rows;
  MySim sim;
  Bot realbot(&sim);
  State &state = realbot.state;
  state.rows = rows;
  state.cols = cols;
  state.viewradius2 = 77;
  state.attackradius2 = 5;
  state.spawnradius2 = 1;
  state.viewradius = sqrt(state.viewradius2);
  state.attackradius = sqrt(state.attackradius2);
  state.spawnradius = sqrt(state.spawnradius2);
  state.turntime = 1000;
  state.turn = 0;
  state.turns = 1000;
  state.loadtime = 3000;
  state.setup();
  bot = &realbot;
  vector< vector<Square> > realgrid(rows, vector<Square>(cols, Square()));
  grid = &realgrid;
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      realgrid[r][c].isWater = lines[r][c] == '%';
    }
  }
  const int kScreenWidth = 1920;
  const int xOffset = (kScreenWidth / 2) - (width / 2);
  cout << "xOffset " << xOffset << endl;
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(width, height);
  glutInitWindowPosition(xOffset, 100);
  glutCreateWindow("Visualizer");
  init();
  glutDisplayFunc(display);
  glutIdleFunc(idle);
  glutTimerFunc(timeDelay, timer, 0);
  glutMouseFunc(mouse);
  glutKeyboardFunc(keyboard);
  glutMainLoop();
  return 0;
}
