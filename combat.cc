#include <iostream>
#include <iomanip>
#include <vector>
using namespace std;

typedef unsigned char i8;
typedef vector<i8> vi8;

class AreaAnt {
 public:
  i8 r, c, p, e, d;
  AreaAnt(i8 r, i8 c, i8 p, i8 d=0, i8 e = 0) : r(r), c(c), p(p), d(d), e(e) {};
};

typedef vector<AreaAnt> vAreaAnt;
typedef vector<AreaAnt*> vAreaAntp;
typedef vector<vAreaAntp> vvAreaAntp;

class Area {
 public:
  int rows, cols, players;
  vAreaAnt ants;
  vvAreaAntp grid;
  vi8 dead;
  Area(int rows, int cols, int players, const vAreaAnt &ax)
      : rows(rows), cols(cols), players(players),
        grid(rows, vAreaAntp(cols, NULL)), ants(ax), dead(players,0) {
    static const int rco[5] = {1,2,2,2,1};
    for (vAreaAnt::iterator a = ants.begin(); a != ants.end(); a++) 
      grid[(*a).r][(*a).c] = &(*a);
    for (vAreaAnt::iterator a = ants.begin(); a != ants.end(); a++) {
      vvAreaAntp::iterator r = grid.begin() + (*a).r - 2;
      for (int i = 0; i < 5 && !(*a).d; i++, r++)
        for (vAreaAntp::iterator c = (*r).begin() + (*a).c - rco[i];
             !(*a).d && c <= (*r).begin() + (*a).c + rco[i]; c++)
          if (*c != NULL && (*a).p != (*c)->p)
            ++(*a).e;
    }
    for (vAreaAnt::iterator a = ants.begin(); a != ants.end(); a++) {
      vvAreaAntp::iterator r = grid.begin() + (*a).r - 2;
      for (int i = 0; i < 5 && !(*a).d; i++, r++)
        for (vAreaAntp::iterator c = (*r).begin() + (*a).c - rco[i];
             !(*a).d && c <= (*r).begin() + (*a).c + rco[i]; c++)
          if (*c != NULL && (*a).p != (*c)->p)
            dead[(*a).p] += ((*a).d = ((*a).e >= (*c)->e));
    }
  };
};

ostream& operator<<(ostream &os, const Area &a) {
  for (int r = 0; r < a.rows; r++) {
    for (int c = 0; c < a.cols; c++) {
      if (a.grid[r][c])
        cout << setw(2) << (int)a.grid[r][c]->d;
      else
        cout << setw(2) << ' ';
    }
    cout << endl;
  }
  for (vi8::const_iterator i = a.dead.begin(); i != a.dead.end(); i++)
    cout << (int)(*i) << " ";
  cout << endl;
}


int main(int argc, char **argv) {
  vAreaAnt ants;
  ants.push_back(AreaAnt(3,3,0));
  ants.push_back(AreaAnt(6,6,0));
  ants.push_back(AreaAnt(3,2,1));
  Area area(10, 10, 2, ants);
  cout << area << endl;
}
