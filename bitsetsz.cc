#include <iostream>
#include <bitset>
#include <vector>
using namespace std;
int main(int argc, char **argv) {
    long x = 1;
    bitset<8> y;
    bitset<32> u;
    bitset<64> v;
    bitset<128> z;
    vector<bool> b(8);
    cout << "long:      "    << sizeof(long) << endl;
    cout << "long(x):   "    << sizeof(x) << endl;
    cout << "bitset<8>: "    << sizeof(y) << endl;
    cout << "bitset<32>: "   << sizeof(u) << endl;
    cout << "bitset<64>: "   << sizeof(v) << endl;
    cout << "bitset<128>: "  << sizeof(z) << endl;
    cout << "vector<bool>: " << sizeof(b) << endl;
    return 0;
}
