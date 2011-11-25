#ifndef _OFFSET_H
#define _OFFSET_H

struct Offset {
  double d;
  int d2, r, c;
  Offset(double d, int d2, int r, int c) : d(d), d2(d2), r(r), c(c) {};
};

#endif // _OFFSET_H
