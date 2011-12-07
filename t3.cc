#include <iostream>
#include <vector>
#include <cassert>

using namespace std;

typedef int v0i;
typedef vector<int> v1i;
typedef vector<v1i> v2i;

typedef bool v0b;
typedef vector<v0b> v1b;
typedef vector<v1b> v2b;

enum { _, X, O };

struct t3m {
  size_t x, y;
  t3m() : x(9999), y(9999) {};
  t3m(size_t x, size_t y) : x(x), y(y) {};
};

struct t3s {
  int p, u;
  v2i board;
  v2b moves;
  t3s();
  t3s(int h, int v);
  t3s(int p, int u, const v2i &b, const v2b &m);
  char at(size_t x, size_t y) const;
};

typedef pair<t3m,t3s> t3ms;
typedef vector<t3ms> vt3ms;

class t3g {
 public:
  size_t h, v;
  int k;
  t3s initial;
  t3g(size_t, size_t, int);
  t3s make_move(const t3m &m, const t3s &s) const;
  int utility(const t3s &s, int p) const;
  int terminal(const t3s &s) const;
  void display(const t3s &s);
  bool k_in_row(const v2i &board, const t3m &m, int p, int dx, int dy) const;
  int compute_utility(const v2i &board, const t3m &m, int p) const;
  vt3ms successors(const t3s &s) const;
};

template <class S, class G, class A>
int min_value(const S &s, const G &g, int p);

template <class S, class G, class A>
int max_value(const S &s, const G &g, int p) {
  typedef pair<A,S> P;
  typedef vector<P> vP;
  if (g.terminal(s))
    return g.utility(s, p);
  int v = -99999999;
  vP vas = g.successors(s);
  for (typename vP::iterator as = vas.begin(); as != vas.end(); as++) {
    v = max(v, min_value<S,G,A>((*as).second, g, p));
  }
  return v;
}

template <class S, class G, class A>
int min_value(const S &s, const G &g, int p) {
  typedef pair<A,S> P;
  typedef vector<P> vP;
  if (g.terminal(s))
    return g.utility(s, p);
  int v = 99999999;
  vP vas = g.successors(s);
  for (typename vP::iterator as = vas.begin(); as != vas.end(); as++) {
    v = min(v, max_value<S,G,A>((*as).second, g, p));
  }
  return v;
}

template <class S, class G, class A>
A minimax_decision(const S &s, const G &g) {
  typedef pair<A,S> P;
  typedef vector<P> vP;
  int maxv = -9999;
  A maxa;
  int p = s.p;
  vP vas = g.successors(s);
  for (typename vP::iterator as = vas.begin(); as != vas.end(); as++) {
    int v = min_value<S,G,A>((*as).second, g, p);
    if (v > maxv) {
      maxv = v;
      maxa = (*as).first;
    }
  }
  return maxa;
}

t3s::t3s(int h, int v)
    : p(X), u(0), board(h, v1i(v, _)), moves(h, v1b(v, true)) {}

t3s::t3s(int p, int u, const v2i &b, const v2b &m)
    : p(p), u(u), board(b), moves(m) {}

t3s t3g::make_move(const t3m &m, const t3s &s) const {
  assert(s.moves[m.x][m.y]);
  int p = s.p == X ? O : X;
  v2i board(s.board); board[m.x][m.y] = s.p;
  v2b moves(s.moves); moves[m.x][m.y] = false;
  int u = compute_utility(board, m, s.p);
  t3s s2(p, u, board, moves);
  assert(!s2.moves[m.x][m.y]);
  return s2;
}

t3g::t3g(size_t h=3, size_t v=3, int k=3)
    : h(h), v(v), k(k), initial(h, v) {}

int t3g::utility(const t3s &s, int p) const {
  return s.u;
}

int t3g::terminal(const t3s &s) const {
  int n = 0;
  for (int x = 0; x < (int) h; x++)
    for (int y = 0; y < (int) v; y++)
      if (s.moves[x][y])
        ++n;
  //cout << "checking terminal " << s.u << " " << n << endl;
  return s.u != 0 || n == 0;
}

int t3g::compute_utility(const v2i &board, const t3m &m, int p) const {
  return (k_in_row(board, m, p, 0, 1) ||
          k_in_row(board, m, p, 1, 0) ||
          k_in_row(board, m, p, 1, -1) ||
          k_in_row(board, m, p, 1, 1))
      ? (p == X ? 1 : -1)
      : 0;
}

bool t3g::k_in_row(const v2i &board, const t3m &m, int p, int dx, int dy) const {
  int n = 0;
  int x, y;
  x = (int)m.x;
  y = (int)m.y;
  while (x < (int)h && y < (int)v && board[x][y] == p) {
    ++n;
    x += dx;
    y += dy;
  }
  x = (int)m.x;
  y = (int)m.y;
  while (x >= 0 && y >= 0 && board[x][y] == p) {
    ++n;
    x -= dx;
    y -= dy;
  }
  --n; // because we counted m itself twice
  //cout << "n is " << n << endl;
  return n >= k;
}

char t3s::at(size_t x, size_t y) const {
  switch (board[x][y]) {
    case X: return 'X';
    case O: return 'O';
    default: return ' ';
  }
}

void t3g::display(const t3s &s) {
  for (size_t y = 0; y < (size_t)v; ++y) {
    for (size_t x = 0; x < (size_t)h; ++x)
      cout << s.at(x,y) << '.';
    cout << endl;
  }
  cout << "utility: " << s.u
       << "; player: " << s.p << endl;
}

vt3ms t3g::successors(const t3s &s) const {
  vt3ms ms;
  for (int x = 0; x < (int)h; x++) {
    for (int y = 0; y < (int)v; y++) {
      if (s.moves[x][y]) {
        t3m m(x,y);
        ms.push_back(t3ms(m, make_move(m, s)));
      }
    }
  }
  return ms;
}

int main(int argc, char **argv) {
  t3g g;
  t3s s = g.initial;
  int x, y;
  do {
    t3m a = minimax_decision<t3s, t3g, t3m>(s, g);
    cout << "minimax: " << a.x << "," << a.y << endl;
    s = g.make_move(a, s);
    assert(!s.moves[a.x][a.y]);
    if (g.terminal(s))
      break;
    g.display(s);
    cout << "your move? ";
    if (!(cin >> x >> y))
      break;
    if (x < 0 || x >= 3 || y < 0 || y >= 3) {
      cout << "invalid move" << endl;
    }
    else if (!s.moves[x][y]) {
      cout << "move already taken" << endl;
    }
    else {
      s = g.make_move(t3m(x, y), s);
      assert(!s.moves[a.x][a.y]);
      assert(!s.moves[x][y]);
    }
  } while (!g.terminal(s));
  g.display(s);
  cout << "game over. "
       << (s.u == 1 ? "you lose." : s.u == 0 ? "tie game." : "you win.")
       << endl;
}
