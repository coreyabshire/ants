/* botviz3: experiment with ant steering */

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <limits>
#include "visualizer.h"
#include "state.h"
#include "bot.h"

using namespace std;

const bool kFoodOn = false;
bool running = true;
const int kMult = 5;
const unsigned int kDelay = 1000/10;
void timer(int value);

string fnames[kFactors] = {
  "FOOD", "TARGET", "UNKNOWN"
};
bool factor[kFactors] = { 1, 1, 1 };


class Game;
Game *game;

class MySim : public Sim {
 public:
  Bot bot;
  Game *game;
  MySim(Game *game) : bot(this), game(game) {
    bot.state.bug << "bot constructed" << endl;
  };
  virtual void makeMove(const Loc &a, int d);
  virtual void go();
};

class GameMap {
 public:
  int rows, cols, players;
  vector<string> lines;
  int load(string filename);
};

class Ant {
 public:
  Bot *bot;
  void update();
};

class Game {
 public:
  int rows, cols, players;
  int turns, player_seed;
  int attackradius, spawnradius, viewradius;
  double loadtime, turntime;
  double time;
  int mouseR, mouseC;
  bool mouseOver;
  int factorsOn;
  int selectedFactor;
  float weight;
  bool weightOn;
  vector< vector<Square> > grid;
  MySim sim;
  Timer stopwatch;
  Game(const GameMap &gamemap);
  void setup(Bot& bot);
  void send(Bot& bot);
  void receive(Bot& bot);
  void update();
  void gather();
  void raze();
  void mouse(int button, int state, int r, int c);
  void special(int k, int r, int c);
  void keyboard(unsigned char k, int r, int c);
  void passiveMotion(int r, int c);
  void entry(int state);
  void info();
  void display();
  float influence(const Square& s);
  void refreshWeights();
  void chooseNewFactor(int d);
  void changeSelectedFactor(float f);
};

int factor1 = 0;
int factor2 = 0;
int factor3 = 0;

void MySim::makeMove(const Loc &a, int d) {
  vector< vector<Square> > &realgrid = game->grid;
  Loc b = bot.state.getLoc(a, d);
  realgrid[a.r][a.c].ant = -1;
  realgrid[b.r][b.c].ant = 0;
}

void Game::gather() {
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      if (grid[r][c].isFood) {
        Loc a(r, c);
        for (vector<Off>::iterator o = sim.bot.state.offsetFirst; o != sim.bot.state.spawnEnd; o++) {
          Loc b = sim.bot.state.addOff(a, *o);
          if (grid[b.r][b.c].ant == 0) {
            grid[r][c].isFood = 0;
          }
        }
      }
    }
  }
}

void Game::raze() {
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      if (grid[r][c].hill != -1) {
        if (grid[r][c].ant != -1) {
          if (grid[r][c].hill != grid[r][c].ant) {
            grid[r][c].hill = -1;
          }
        }
      }
    }
  }
}

void Game::receive(Bot &bot) {
  stopwatch.start();
  bot.state.update();
  if (running)
    bot.makeMoves();
  bot.endTurn();
  time = stopwatch.getTime();
}

void Game::update() {
  send(sim.bot);
  receive(sim.bot);
  gather();
  raze();
}

void MySim::go() {
}

void text(const int x, const int y, const char *s) {
  float fx = (float) x, fy = (float) y;
  glColor3f(0.0,0.0,0.0);
  glRasterPos2f(fx+0.2, fy+0.2);
  glutBitmapString(GLUT_BITMAP_8_BY_13, (const unsigned char*) s);
  glColor3f(1.0,1.0,1.0);
  glRasterPos2f(fx, fy);
  glutBitmapString(GLUT_BITMAP_8_BY_13, (const unsigned char*) s);
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

void line(const Loc &a, const Loc &b) {
  glColor3f(1.0, 0.0, 0.0);
  glBegin(GL_LINES);
  glVertex3f(a.c + 0.5, a.r + 0.5, 0.0);
  glVertex3f(b.c + 0.5, b.r + 0.5, 0.0);
  glEnd();
}

void Game::send(Bot &bot) {
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      if (grid[r][c].isWater) {
        bot.state.grid[r][c].isWater = 1;
      }
      else {
        if (kFoodOn)
          if (rand() % 100000 < 2)
            grid[r][c].isFood = 1;
      }
      if (grid[r][c].isFood) {
        bot.state.putFood(r, c);
      }
      if (grid[r][c].hill != -1) {
        bot.state.putHill(r, c, grid[r][c].hill);
      }
      if (grid[r][c].ant == 0) {
        bot.state.putAnt(r, c, 0);
      }
    }
  }
}

void Game::passiveMotion(int r, int c) {
  if (r != mouseR || c != mouseC) {
    mouseR = r;
    mouseC = c;
    glutPostRedisplay();
  }
}

void Game::entry(int state) {
  mouseOver = state == GLUT_ENTERED;
}

void Game::mouse(int button, int mstate, int r, int c) {
  mouseR = r;
  mouseC = c;
  if (mstate != GLUT_DOWN) {
    return;
  }
  if (r < rows && c < cols) {
    Square &s = grid[r][c];
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
}

void Game::special(int k, int x, int y) {
  switch (k) {
    case GLUT_KEY_F1:
      break;
    case GLUT_KEY_UP:        chooseNewFactor(-1); break;
    case GLUT_KEY_DOWN:      chooseNewFactor(1);  break;
    case GLUT_KEY_RIGHT:     changeSelectedFactor(0.1); break;
    case GLUT_KEY_LEFT:      changeSelectedFactor(-0.1); break;
    case GLUT_KEY_PAGE_UP:   changeSelectedFactor(1.0); break;
    case GLUT_KEY_PAGE_DOWN: changeSelectedFactor(-1.0); break;
    case GLUT_KEY_HOME:      break;
    case GLUT_KEY_END:       break;
    default:
      break;
  }
}

void Game::chooseNewFactor(int d) {
  selectedFactor += d;
  if (selectedFactor < 0)
    selectedFactor = 0;
  else if (selectedFactor >= kFactors)
    selectedFactor = kFactors - 1;
}

void Game::changeSelectedFactor(float f) {
  sim.bot.state.weights[selectedFactor] += f;
  refreshWeights();
}

void Game::keyboard(unsigned char k, int r, int c) {
  switch (k) {
    case 27:
      exit(0);
      break;
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
    case 'h':
      grid[r][c].hill = 1;
      if (factor3 > 0)
        --factor3;
      break;
    case '\t':
      update();
      glutPostRedisplay();
      break;
    case ' ':
      running = !running;
      break;
    default:
      if (k >= '0' && k <= '9') {
        int f = k - '0';
        if (f < kFactors) {
          factor[f] = !factor[f];
          refreshWeights();
        }
      }
      else {
        cout << "keyboard " << k << " " << r << " " << c << endl;
      }
      break;
  }
  refreshWeights();
  glutPostRedisplay();
}


void Game::refreshWeights() {
  factorsOn = 0;
  weight = 0.0;
  for (int fi = 0; fi < kFactors; fi++) {
    if (factor[fi]) {
      ++factorsOn;
      weight += sim.bot.state.weights[fi];
    }
  }
  weightOn = factorsOn > 1;
}

float Game::influence(const Square& s) {
  float inf = 0.0;
  int c = 0;
  int onef = 0;
  for (int f = 0; f < kFactors; f++) {
    if (factor[f]) {
      inf += s.inf[f] * sim.bot.state.weights[f];
      c++;
      onef = f;
    }
  }
  if (weight > 0.0) {
    inf /= weight;
  }
  if (c == 1)
    inf = s.inf[onef];
  inf *= 0.7;
  return inf;
}

void Game::info() {
  ostringstream os;
  os << "Time: " << setfill('0') << setiosflags(ios_base::fixed) << setprecision(5) << setw(7) << time << endl;
  for (int f = 0; f < kFactors; f++) {
    if (factor[f]) {
      os << (f == selectedFactor ? " > " : "   ")
         << f << ": " << fnames[f] << " x " << sim.bot.state.weights[f] << endl;
    }
  }
  if (mouseOver) {
    Loc a(mouseR, mouseC);
    Square &as = sim.bot.state.grid[mouseR][mouseC];
    os << "At " << a << ": " << setfill('0')
       << setiosflags(ios_base::fixed) << setprecision(11) << setw(13) << influence(as)  << endl;
    for (int d = 0; d < TDIRECTIONS; d++) {
      Loc b = sim.bot.state.getLoc(a, d);
      char dc = CDIRECTIONS[d];
      Square &bs = sim.bot.state.grid[b.r][b.c];
      os << " " << dc << " " << b << ": " << setfill('0')
         << setiosflags(ios_base::fixed) << setprecision(11) << setw(13) << influence(bs) << endl;
    }
  }
  text(1, 3, os.str().c_str());
}

void Game::display() {
  refreshWeights();
  State &state = sim.bot.state;
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      Square &s = grid[r][c];
      if (s.isWater)
        tile(c, r, 0.0, 0.0, 0.0);
      else if (s.ant == 0) {
        tile(c, r, 0.0, 1.0, 0.0);
      }
      else if (s.hill > 0) {
        tile(c, r, 1.0, 1.0, 0.0);
      }
      else if (s.isFood)
        tile(c, r, 0.0, 0.0, 1.0);
      else {
        float f = influence(state.grid[r][c]);
        tile(c, r, 0.0, f + 0.1, 0.0);
      }
    }
  }
  info();
}

int GameMap::load(string filename) {
  string type, line;
  ifstream file(filename.c_str(), ifstream::in);
  if (file.fail()) {
    cerr << "unable to open file: " << filename << endl;
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
    else if (type == "#") {
      file.ignore(numeric_limits<int>::max(), '\n');
    }
    else {
      cerr << "unrecognized: " << type << endl;
    }
  }
  file.close();
  return 0;
}

Game::Game(const GameMap &m) : sim(this) {
  rows = m.rows;
  cols = m.cols;
  players = m.players;
  mouseOver = false;
  weight = 0.0;
  weightOn = 0;
  selectedFactor = 0;
  factorsOn = false;
  viewradius = 77;
  attackradius = 5;
  spawnradius = 1;
  turntime = 1000;
  turns = 1000;
  loadtime = 3000;
  grid = vector< vector<Square> >(rows, vector<Square>(cols));
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      grid[r][c].isWater = m.lines[r][c] == '%';
    }
  }
  setup(sim.bot);
  refreshWeights();
}

void Game::setup(Bot &bot) {
  State &state = bot.state;
  state.rows = rows;
  state.cols = cols;
  state.viewradius = viewradius;
  state.attackradius = attackradius;
  state.spawnradius = spawnradius;
  state.turntime = turntime;
  state.turns = turns;
  state.loadtime = loadtime;
  state.turn = 0;
  state.setup();
}

void timer(int value) {
  game->update();
  glutPostRedisplay();
  glutTimerFunc(kDelay, timer, value);
}

void keyboard(unsigned char k, int x, int y) {
  game->keyboard(k, y/kMult, x/kMult);
}

void mouse(int button, int state, int x, int y) {
  game->mouse(button, state, y/kMult, x/kMult);
  glutPostRedisplay();
}

void special(int k, int x, int y) {
  game->special(k, y/kMult, x/kMult);
  glutPostRedisplay();
}

void passiveMotion(int x, int y) {
  game->passiveMotion(y/kMult, x/kMult);
}

void entry(int state) {
  game->entry(state);
}

void display(void) {
  glClear(GL_COLOR_BUFFER_BIT);
  game->display();
  glFlush();
  glutSwapBuffers();
}

int main(int argc, char **argv) {
  if (argc != 2) {
    cout << "need filename" << endl;
    return 1;
  }
  string filename(argv[1]);

  GameMap gamemap;
  gamemap.load(filename);
  Game xgame(gamemap);
  
  game = &xgame;

  int width = kMult * game->cols;
  int height = kMult * game->rows;

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(width, height);
  glutInitWindowPosition(100, 100);
  glutCreateWindow("Visualizer");
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, game->cols, game->rows, 0.0, -1.0, 1.0);
  glutDisplayFunc(display);
  glutTimerFunc(kDelay, timer, 0);
  glutMouseFunc(mouse);
  glutKeyboardFunc(keyboard);
  glutSpecialFunc(special);
  glutPassiveMotionFunc(passiveMotion);
  glutEntryFunc(entry);
  glutMainLoop();

  return 0;
}
