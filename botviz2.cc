#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include "visualizer.h"
#include "state.h"
#include "bot.h"

using namespace std;

class Game;

class MySim : public Sim {
 public:
  Bot bot;
  Game *game;
  MySim(Game *game) : game(game), bot(this) {};
  virtual void makeMove(const Location &a, int d);
  virtual void go();
};

class GameMap {
 public:
  int rows, cols, players;
  vector<string> lines;
  int load(string filename);
};

class Game {
 public:
  int rows, cols, players;
  int turns, player_seed;
  int attackradius2, spawnradius2, viewradius2;
  double loadtime, turntime;

  vector< vector<Square> > grid;
  MySim sim;

  Game(const GameMap &gamemap);

  void setup(Bot& bot);
  void send(Bot& bot);
  void receive(Bot& bot);

  void update();
  void gather();

  void mouse(int button, int state, int r, int c);
  void display();
};

int factor1 = 0;
int factor2 = 0;
int factor3 = 0;

void MySim::makeMove(const Location &a, int d) {
  vector< vector<Square> > &realgrid = game->grid;
  Location b = bot.state.getLocation(a, d);
  realgrid[a.row][a.col].ant = -1;
  realgrid[b.row][b.col].ant = 0;
}

void Game::gather() {
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      if (grid[r][c].isFood) {
        Location a(r, c);
        for (vector<Offset>::iterator o = sim.bot.state.offsetFirst; o != sim.bot.state.spawnEnd; o++) {
          Location b = sim.bot.state.addOffset(a, *o);
          if (grid[b.row][b.col].ant == 0) {
            grid[r][c].isFood = 0;
            //cout << "detected consumption" << endl;
          }
        }
      }
    }
  }
}

void Game::receive(Bot &bot) {
  bot.state.updateVisionInformation();
  bot.state.updateInfluenceInformation();
  //  cout << "making moves" << endl;
  bot.makeMoves();
  bot.endTurn();
}

void Game::update() {
  send(sim.bot);
  receive(sim.bot);
  gather();
}

void MySim::go() {
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

void Game::send(Bot &bot) {
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      if (grid[r][c].isWater) {
        bot.state.grid[r][c].isWater = 1;
      }
      else {
        //if (rand() % 100000 < 2) {
        //  grid[r][c].isFood = 1;
        //}
      }
      if (grid[r][c].isFood) {
        bot.state.grid[r][c].isFood = 1;
        bot.state.grid[r][c].isFood2 = 1;
        bot.state.food.push_back(Location(r, c));
      }
      if (grid[r][c].ant == 0) {
        //    cout << "ant at " << r << "," << c << endl;
        bot.state.grid[r][c].ant = 0;
        bot.state.myAnts.push_back(Location(r, c));
      }
    }
  }
}

void Game::mouse(int button, int mstate, int r, int c) {
  if (mstate != GLUT_DOWN) {
    return;
  }
  State& state = sim.bot.state;
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

void Game::display() {
  State &state = sim.bot.state;
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      Square &s = grid[r][c];
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
}

int GameMap::load(string filename) {
  string type, line;
  ifstream file(filename.c_str(), ifstream::in);
  if (file.fail()) {
    cout << "unable to open file: " << filename << endl;
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
  return 0;
}

Game::Game(const GameMap &m) : sim(this) {
  rows = m.rows;
  cols = m.cols;
  players = m.players;
  viewradius2 = 77;
  attackradius2 = 5;
  spawnradius2 = 1;
  turntime = 1000;
  turns = 1000;
  loadtime = 3000;
  grid = vector< vector<Square> >(rows, vector<Square>(cols, Square()));
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      grid[r][c].isWater = m.lines[r][c] == '%';
    }
  }
  setup(sim.bot);
}

void Game::setup(Bot &bot) {
  State &state = bot.state;
  state.rows = rows;
  state.cols = cols;
  state.viewradius2 = viewradius2;
  state.attackradius2 = attackradius2;
  state.spawnradius2 = spawnradius2;
  state.turntime = turntime;
  state.turns = turns;
  state.loadtime = loadtime;
  state.viewradius = sqrt(viewradius2);
  state.attackradius = sqrt(attackradius2);
  state.spawnradius = sqrt(spawnradius2);
  state.turn = 0;
  state.setup();
}

bool running = true;
const int kMult = 4;
const unsigned int kDelay = 1000/30;
Game *game;

void timer(int value) {
  game->update();
  glutPostRedisplay();
  if (running)
    glutTimerFunc(kDelay, timer, value);
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
    case '\t':
      game->update();
      glutPostRedisplay();
      break;
    case ' ':
      running = !running;
      if (running)
        glutTimerFunc(kDelay, timer, 0);
      break;
    default:
      cout << "keyboard " << k << " " << x << " " << y << endl;
      break;
  }
  cout << " factors " << factor1 << ", " << factor2 << ", " << factor3 << endl;
  glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
  game->mouse(button, state, y/kMult, x/kMult);
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
  glutMainLoop();

  return 0;
}
