#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include "visualizer.h"

using namespace std;

typedef vector<float> vf;
typedef vector<vf> vvf;
typedef vector<vvf> vvvf;
typedef vector<vvvf> vvvvf;

const int kMult = 5;
int rows, cols, factors;
int turn, factor;
vvvvf inf;
int dirs[4][2] = { {-1, 0}, {0, 1}, {1, 0}, {0, -1} };

void tile(float x, float y, float r, float g, float b) {
  glColor3f(r, g, b);
  glBegin(GL_POLYGON);
  glVertex3f(x, y, 0.0);
  glVertex3f(x + 1.0, y, 0.0);
  glVertex3f(x + 1.0, y + 1.0, 0.0);
  glVertex3f(x, y + 1.0, 0.0);
  glEnd();
}

void redraw(void) {
}

void idle(void) {
}

void keyboard(unsigned char k, int x, int y) {
  switch (k) {
    case 'a':
      if (turn < inf.size() - 1)
        ++turn;
      break;
    case 'z':
      if (turn > 0)
        --turn;
      break;
    case 'f':
      if (factor < factors - 1)
        ++factor;
      break;
    case 'v':
      if (factor > 0)
        --factor;
      break;
    default:
      cout << "keyboard " << k << " " << x << " " << y << endl;
      break;
  }
  cout << "turn " << turn << " factor " << factor << endl;
  glutPostRedisplay();
}

void display(void) {
  glClear(GL_COLOR_BUFFER_BIT);
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      float f0 = inf[turn][factor][r][c];
      float f1 = inf[turn][factor][r][c];
      float f2 = inf[turn][factor][r][c];
      tile(c, r, f2, f1, f0);
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

bool load(char *filename) {
  ifstream file(filename, ifstream::in);
  string code, type;
  if (file.fail()) {
    cout << "unable to open file: " << filename << endl;
    return false;
  }
  cout << "loading file" << endl;
  file >> code >> type >> rows;
  file >> code >> type >> cols;
  file >> code >> type >> factors;
  int turn, factor, line, row;
  float f;
  //cout << "map is " << rows << " x " << cols << " x " << factors << endl;
  while (file.good()) {
    file >> code >> type >> turn;
    if (type != "turn") {
      cout << "expected turn but got " << type << " at " << inf.size() << endl;
      return true;
    }
    inf.push_back(vvvf(factors, vvf(rows, vf(cols, 0.0))));
    //cout << code << type << turn << endl;
    for (int f = 0; f < factors; f++) {
      file >> code >> type >> factor;
      if (type != "factor") {
        cout << "expected factor but got " << type << " at " << inf.size() << endl;
        return true;
      }
      //cout << code << type << factor << endl;
      for (int r = 0; r < rows; r++) {
        file >> code >> type >> row;
        if (type != "row") {
          cout << "expected row but got " << type << endl;
          return true;
        }
        //cout << code << type << row << endl;
        string val;
        for (int c = 0; c < cols; c++) {
          file >> val;
          inf[turn][factor][r][c] = atof(val.c_str());
        }
      }
    }
  }
  file.close();
  return true;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    cout << "need filename" << endl;
    return 1;
  }
  if (!load(argv[1])) {
    return 1;
  }
  turn = 0;
  factor = 0;
  cout << "read map: " << inf.size() << " x " << factors << " x " << rows << " x " << cols << endl;
  redraw();
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(cols * kMult, rows * kMult);
  glutInitWindowPosition(100, 100);
  glutCreateWindow("Visualizer");
  init();
  glutDisplayFunc(display);
  glutIdleFunc(idle);
  glutKeyboardFunc(keyboard);
  //  glutMouseFunc(mouse);
  glutMainLoop();
  return 0;
}
