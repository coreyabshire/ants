#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
#include <deque>
#include <vector>
#include <list>
#include <iostream>
#include <iomanip>
#include <sstream>
#include "bot.h"
#include "Timer.h"

using namespace std;

const int kMult = 5;
const unsigned int kDelay = 1000/50;
Bot *gbot;
Timer stopwatch;
int turn = 0;
bool showinfo = true;
vector< vector< vector<Square> > > grids;
vector<int> antCounts;
vector<int> foodCounts;
vector<double> turnTimes;
bool needEndTurn = false;
bool factor[kFactors] = { 1, 1, 1 };
int factorsOn = 0;
float weight = 0.0;
bool weightOn = false;
int mouseR, mouseC;
bool mouseOver;
string fnames[kFactors] = { "FOOD", "TARGET", "UNKNOWN" };
string snames[3] = { "SAFE", "KILL", "DIE" };

int antcolors[10][3] = {
  {   0,   0, 255 },
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
    bot.state.update();
    grids.push_back(bot.state.grid);
    bot.state.bug << "copied grid " << bot.state.turn << endl;
    antCounts.push_back(bot.state.ants.size());
    foodCounts.push_back(bot.state.food.size());
    turn = bot.state.turn;
    stopwatch.start();
    bot.makeMoves();
    turnTimes.push_back(stopwatch.getTime());
    needEndTurn = true;
    glutPostRedisplay();
  }
}

void refreshWeights() {
  factorsOn = 0;
  weight = 0.0;
  for (int fi = 0; fi < kFactors; fi++) {
    if (factor[fi]) {
      ++factorsOn;
      weight += gbot->state.weights[fi];
    }
  }
  weightOn = factorsOn > 1;
}

void keyboard(unsigned char k, int x, int y) {
  switch (k) {
    case 27:
      exit(0);
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
        cout << "keyboard " << k << " " << x << " " << y << endl;
      }
      break;
  }
  glutPostRedisplay();
}

void changeTurn(int n) {
  turn = n;
  if (turn < 0)
    turn = 0;
  if (turn >= (int) grids.size())
    turn = grids.size() - 1;
}

void special(int k, int x, int y) {
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

void motion(int x, int y) {
}

void passiveMotion(int x, int y) {
  int r = y / kMult, c = x / kMult;
  if (r != mouseR || c != mouseC) {
    mouseR = r;
    mouseC = c;
    glutPostRedisplay();
  }
}

void entry(int state) {
  mouseOver = state == GLUT_ENTERED;
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

void text(const int x, const int y, const char *s) {
  float fx = (float) x, fy = (float) y;
  glColor3f(0.0,0.0,0.0);
  glRasterPos2f(fx+0.2, fy+0.2);
  glutBitmapString(GLUT_BITMAP_8_BY_13, (const unsigned char*) s);
  glColor3f(1.0,1.0,1.0);
  glRasterPos2f(fx, fy);
  glutBitmapString(GLUT_BITMAP_8_BY_13, (const unsigned char*) s);
}

float influence(const Square& s) {
  float inf = 0.0;
  int c = 0;
  int onef = 0;
  for (int f = 0; f < kFactors; f++) {
    if (factor[f]) {
      inf += s.inf[f] * gbot->state.weights[f];
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

void info() {
  Bot &bot = *gbot;
  ostringstream os;
  os << "Turn: " << turn << endl;
  os << "Ants: " << antCounts[turn] << endl;
  os << "Food: " << foodCounts[turn] << endl;
  os << "Time: " << setiosflags(ios_base::fixed) << setprecision(7) << setw(9) << turnTimes[turn] << endl;
  for (int f = 0; f < kFactors; f++) {
    if (factor[f]) {
      os << f << ": " << fnames[f] << " x " << gbot->state.weights[f] << endl;
    }
  }
  if (mouseOver) {
    Loc a(mouseR, mouseC);
    Square &as = grids[turn][a.r][a.c];
    os << "At " << a << ": " << influence(as) << endl;
    for (int d = 0; d < TDIRECTIONS; d++) {
      Loc b = bot.state.getLoc(a, d);
      char dc = CDIRECTIONS[d];
      Square &bs = grids[turn][b.r][b.c];
      os << " " << dc << " " << b << ": " << setfill('0')
         << setiosflags(ios_base::fixed) << setprecision(7) << setw(9) << influence(bs) << endl;
    }
    os << "Weight " << (weightOn ? "on" : "off") << ": " << factorsOn << " " << weight << endl;
    os << "Ant:      " << as.ant << endl;
    os << "Sum:      " << as.sumAttacked << endl;
    os << "Attacked: " << as.attacked << endl;
    os << "Fighting: " << as.fighting << endl;
    os << "Best:     " << as.best << endl;
    os << "Status:   " << snames[as.status] << endl;
  }
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
      if (s.isWater)
        tile(c, r, 0.0, 0.0, 0.0);
      else if (s.ant >= 0)
        anttile(c, r, s.ant);
      else if (s.hill >= 0)
        hilltile(c, r, s.hill);
      else if (s.isFood)
        tile(c, r, 0.9, 0.9, 0.0);
      else {
        float inf = influence(s);
        float fg = (float) s.attacked * 0.05;
        float fv = (float) s.isVisible * 0.1;
        float fb = (float) s.fighting * 0.05;
        tile(c, r, fb, inf + 0.1, fg + fv);
        //tile(c, r, fb, inf + 0.1, fg + fv);
      }
    }
  }
  if (showinfo)
    info();
  glFlush();
  glutSwapBuffers();
  if (needEndTurn) {
    bot.endTurn();
    bot.state.bug << "ended turn " << bot.state.turn << endl;
    needEndTurn = false;
    glutTimerFunc(kDelay, timer, 0);
  }
}

int main(int argc, char *argv[]) {
  cout.sync_with_stdio(0); // this line makes your bot faster

  Bot bot;
  gbot = &bot;

  // reads the game parameters and sets up
  stopwatch.start();
  cin >> bot.state;
  bot.state.setup();
  grids.push_back(bot.state.grid);
  antCounts.push_back(bot.state.ants.size());
  foodCounts.push_back(bot.state.food.size());
  turn = bot.state.turn;
  turnTimes.push_back(stopwatch.getTime());
  bot.endTurn();

  int width = kMult * bot.state.cols;
  int height = kMult * bot.state.rows;

  refreshWeights();

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
  glutMotionFunc(motion);
  glutPassiveMotionFunc(passiveMotion);
  glutEntryFunc(entry);
  glutKeyboardFunc(keyboard);
  glutSpecialFunc(special);
  glutMainLoop();

  return 0;
}
