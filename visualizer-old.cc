#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include "visualizer.h"

using namespace std;

const int kMaxHeight = 800;
int kMult = 5;
int rows, cols, players;
const int factors = 4;
vector< vector< vector<float> > > inf;
vector< string > lines;
const int dirs[4][2] = { {-1, 0}, {0, 1}, {1, 0}, {0, -1} };
unsigned int timeDelay = 100;

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

void timer(int value) {
  vector<string> temp(lines.begin(), lines.end());
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      if (lines[r][c] == 'a') {
        int nr = r, nc = c;
        float maxf = 0.0;
        cout << "computing " << r << "," << c << endl;
        for (int d = 0; d < 4; d++) {
          int dr = (r + dirs[d][0] + rows) % rows;
          int dc = (c + dirs[d][1] + cols) % cols;
          cout << "looking at " << dr << "," << dc << endl;
          if (lines[dr][dc] == '.' || lines[dr][dc] == '*') {
            if (inf[dr][dc][0] > maxf) {
              nr = dr;
              nc = dc;
              maxf = inf[dr][dc][0];
              cout << "chose new dir " << nr << "," << nc << endl;
              cout << "new maxf " << maxf << endl;
            }
          }
        }
        if ((nr != r) || (nc != c)) {
          cout << "moving to " << nr << "," << nc << endl;
          temp[nr][nc] = 'a';
          temp[r][c] = '.';
        }
      }
    }
  }
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      lines[r][c] = temp[r][c];
    }
  }
  redraw();
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

void mouse(int button, int state, int x, int y) {
  int r = (int)((float)y / (float)kMult);
  int c = (int)((float)x / (float)kMult);
  if (state != GLUT_DOWN) {
    cout << "mouse " << button << " " << state << " " << x << "," << y << " " << r << " " << c << endl;
    return;
  }
  cout << "mouse " << button << " " << x << "," << y << " " << r << " " << c << endl;
  if (r < rows && c < cols)
    switch (button) {
      case GLUT_LEFT_BUTTON:
        if (lines[r][c] == '*')
          lines[r][c] = '.';
        else if (lines[r][c] == '.')
          lines[r][c] = '*';
        break;
      case GLUT_MIDDLE_BUTTON:
        if (lines[r][c] == 'b')
          lines[r][c] = '.';
        else if (lines[r][c] == '.')
          lines[r][c] = 'b';
        break;
      case GLUT_RIGHT_BUTTON:
        if (lines[r][c] == 'a')
          lines[r][c] = '.';
        else if (lines[r][c] == '.')
          lines[r][c] = 'a';
        break;
    }
  redraw();
}

void display(void) {
  glClear(GL_COLOR_BUFFER_BIT);
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      float f0 = inf[r][c][0];
      float f1 = inf[r][c][1] == 1.0 ? 1.0 : 0.0;
      float f2 = inf[r][c][2];
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
  glutMainLoop();
  return 0;
}
