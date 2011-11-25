#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
#include <vector>
#include <iostream>
#include <sstream>
#include "bot.h"

using namespace std;

const int kMult = 4;
const unsigned int kDelay = 1000/10;
Bot *gbot;
int turn = 0;
bool showinfo = false;
vector< vector< vector<Square> > > grids;
bool needEndTurn = false;

int antcolors[10][3] = {
  {   0, 255,   0 },
  {  49, 141, 233 },
  { 213,  18,  50 },
  { 233, 110,  49 },
  { 247, 247,  86 },
  { 100,  23, 207 },
  { 215,  92, 215 },
  { 166, 230, 153 },
  {  46, 184, 127 },
  { 213,  18,  50 },
};

void timer(int value) {
  Bot &bot = *gbot;
  if (cin >> bot.state) {
    bot.state.updateVisionInformation();
    bot.state.updateInfluenceInformation();
    grids.push_back(bot.state.grid);
    turn = bot.state.turn;
    bot.makeMoves();
    needEndTurn = true;
    glutPostRedisplay();
  }
}

void keyboard(unsigned char k, int x, int y) {
  switch (k) {
    case 27:
      exit(0);
      break;
    default:
      cout << "keyboard " << k << " " << x << " " << y << endl;
      break;
  }
  glutPostRedisplay();
}

void changeTurn(int n) {
  Bot &bot = *gbot;
  turn = n;
  if (turn < 0)
    turn = 0;
  if (turn >= grids.size())
    turn = grids.size() - 1;
}

void special(int k, int x, int y) {
  Bot &bot = *gbot;
  switch (k) {
    case GLUT_KEY_F1:
      showinfo = !showinfo;
      break;
    case GLUT_KEY_RIGHT:     changeTurn(turn + 1);       break;
    case GLUT_KEY_LEFT:      changeTurn(turn - 1);       break;
    case GLUT_KEY_PAGE_UP:   changeTurn(turn + 10);      break;
    case GLUT_KEY_PAGE_DOWN: changeTurn(turn - 10);      break;
    case GLUT_KEY_HOME:      changeTurn(  0);            break;
    case GLUT_KEY_END:       changeTurn(grids.size()); break;
    default:
      break;
  }
  glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
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

void text(const int x, const int y, const char *s) {
  glColor3f(1.0,1.0,1.0);
  glRasterPos2i(x, y);
  glutBitmapString(GLUT_BITMAP_8_BY_13, (const unsigned char*) s);
}

void info(void) {
  Bot &bot = *gbot;
  State &state = bot.state;
  ostringstream os;
  os << "Turn: " << turn << endl;
  os << "Ants: " << state.myAnts.size() << endl;
  os << "Food: " << state.food.size() << endl;
  text(1, 3, os.str().c_str());
}

inline float ac(int a, int c) { return (float) antcolors[a % sizeof antcolors][c] / 255.0; }
inline float hc(int a, int c) { return ac(a,c) * 0.75; }

void anttile(int x, int y, int a)  { tile(x, y, ac(a,0), ac(a,1), ac(a,2)); }
void hilltile(int x, int y, int a) { tile(x, y, hc(a,0), hc(a,1), hc(a,2)); }


void display(void) {
  glClear(GL_COLOR_BUFFER_BIT);
  Bot &bot = *gbot;
  State &state = bot.state;
  vector< vector<Square> > &grid = grids[turn];
  for (int r = 0; r < state.rows; r++) {
    for (int c = 0; c < state.cols; c++) {
      Square &s = grid[r][c];
      float f0 = 0.0, f1 = 0.0, f2 = 0.0;
      if (s.isWater)
        tile(c, r, 0.0, 0.0, 0.0);
      else if (s.ant >= 0)
        anttile(c, r, s.ant);
      else if (s.hillPlayer >= 0)
        hilltile(c, r, s.hillPlayer);
      else if (s.isFood)
        tile(c, r, 0.0, 0.0, 1.0);
      else {
        float f1 = s.inf[UNKNOWN];
        float f2 = s.inf[VISIBLE];
        float f3 = s.inf[FOOD];
        tile(c, r, f1 * 0.4, 0.2 *f2 + 0.1 , f3 * 1.0);
      }
    }
  }
  if (showinfo)
    info();
  glFlush();
  glutSwapBuffers();
  if (needEndTurn) {
    bot.endTurn();
    needEndTurn = false;
    glutTimerFunc(kDelay, timer, 0);
  }
}

int main(int argc, char *argv[]) {
  cout.sync_with_stdio(0); // this line makes your bot faster

  Bot bot;
  gbot = &bot;

  // reads the game parameters and sets up
  cin >> bot.state;
  bot.state.setup();
  bot.setup();
  grids.push_back(bot.state.grid);
  turn = bot.state.turn;
  bot.endTurn();

  int width = kMult * bot.state.cols;
  int height = kMult * bot.state.rows;

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(width, height);
  glutInitWindowPosition(100, 100);
  glutCreateWindow("Visualizer");
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, bot.state.cols, bot.state.rows, 0.0, -1.0, 1.0);
  glutDisplayFunc(display);
  glutTimerFunc(kDelay, timer, 0);
  glutMouseFunc(mouse);
  glutKeyboardFunc(keyboard);
  glutSpecialFunc(special);
  glutMainLoop();

  return 0;
}
