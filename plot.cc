#include <iostream>
#include <iomanip>
using namespace std;
int main(int argc, char **argv) {
  int kf = 3;
  float f[] = {1.0, 1.0, 1.0};
  float decay[] = {0.97, 0.9, 0.5};
  for (int i = 0; i < 255; i++) {
    for (int j = 0; j < kf; j++) {
      cout << setiosflags(ios_base::fixed) << setprecision(7) << setw(11) << f[j];
    }
    cout << endl;
    for (int j = 0; j < kf; j++) {
      f[j] *= decay[j];
    }
  }
}
