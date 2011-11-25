#include <iostream>
#include <vector>
#include <algorithm>
#include "State.h"
#include "Location.h"
#include "Offset.h"

using namespace std;

typedef vector<double> vd;
typedef vector<vd> vvd;

void dump(string name, vvd &d, int n) {
  cout << "double " << name << "[" << n << "][" << n << "] = {" << endl;
  for (int r = 0; r < d.size(); r++) {
    cout << "  {";
    for (int c = 0; c < d[r].size(); c++) {
      cout << (c ? "," : "") << d[r][c];
    }
    cout << "}," << endl;
  }
  cout << "};" << endl;
}

void dump(string name, vector<Offset> &offsets) {
  cout << "Offset " << name << "[] = {" << endl;
  for (int i = 0; i < offsets.size(); i++) {
    Offset &o = offsets[i];
    cout << "  {" << o.d << "," << o.d2 << "," << o.r << "," << o.c << "}," << endl;
  }
  cout << "};" << endl;
}

int main(int argc, char **argv) {
  string sep(" ");
  int n = 100;
  Location a(0,0);
  vector<Offset> offsets;

  vvd d(n, vd(n, 0.0));
  vvd d2(n, vd(n, 0.0));
  for (int r = 0; r < n; r++) {
    for (int c = 0; c < n; c++) {
      d2[r][c] = r * r + c * c;
      d[r][c] = sqrt(d2[r][c]);
    }
  }

  for (int r = (1 - n); r < n; r++) {
    for (int c = (1 - n); c < n; c++) {
      offsets.push_back(Offset(d[abs(r)][abs(c)], d2[abs(r)][abs(c)], r, c));
    }
  }

  cout << "#include \"Offset.h\"" << endl;
  dump("distanceLookup", d, n);
  dump("distance2Lookup", d2, n);
  sort(offsets.begin(), offsets.end());
  dump("sortedOffsets", offsets);
      
}
