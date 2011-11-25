#include <iostream>
#include <tr1/unordered_map>
#include <cstdlib>

using namespace std;
using namespace std::tr1;

struct loc {
  int r, c;
  loc(int r, int c) : r(r), c(c) {};
};
ostream& operator<<(ostream &out, const loc &a) {
  out << "(" << a.r << "," << a.c << ")";
}

struct lochash {
  int h,w;
  lochash(int h, int w) : h(h),w(w) {};
  size_t operator()(const loc &k) const {
    return k.r * w + k.c;
  }
};
struct loceq {
  bool operator()(const loc &a, const loc &b) const { return a.r == b.r && a.c == b.c; }
};
bool operator==(const loc &a, const loc &b) {
  return a.r == b.r && a.c == b.c;
}

int main(int argc, char **argv) {
  typedef unordered_map<loc,int,lochash,loceq> map_t;
  typedef map_t::iterator map_it;
  map_t dist(100,lochash(200,200));
  for (int i = 0; i < 100; i++) {
    dist[loc(rand()%200, rand()%200)] = rand()%1000;
  }
  for (map_it p = dist.begin(); p != dist.end(); p++) {
    cout << (*p).first << " " << (*p).second << endl;
  }
  return 0;
}
