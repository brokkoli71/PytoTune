#include <iostream>

#include "hwy/highway.h"

using namespace std;
using namespace hwy;
using namespace hwy::HWY_NAMESPACE;

// repo of highway library:
// https://github.com/google/highway
// for a quick reference, see here:
// https://github.com/google/highway/blob/master/g3doc/quick_reference.md
int main() {
  // data type of items (change this to int, float, short, double, ...)
  using DT = float;

  // create aligned arrays on stack
  HWY_ALIGN DT a[128];
  HWY_ALIGN DT b[128];
  HWY_ALIGN DT c[128];

  // fill arrays a and b with some values between 0 and 99
  for (int i = 0; i < 128; ++i) {
    a[i] = DT(rand() % 100);
    b[i] = DT(rand() % 100);
  }

  // tag for full vectors (as big as the hardware and compiler flags allow)
  auto d = ScalableTag<DT>{};
  cout << "number of values per vector: " << Lanes(d) << endl;

  // add values from array a to array b and store results to c
  for (size_t i = 0; i < 128; i += Lanes(d)) {
    auto va = Load(d, &a[i]); // aligned load from a
    auto vb = Load(d, &b[i]); // aligned load from b
    Store(va + vb, d, &c[i]); // aligned store of va + vb to c
  }

  // test correctness
  for (int i = 0; i < 128; ++i) {
    if(c[i] != a[i] + b[i]){
      cerr << "Wrong results!" << endl;
      return -1;
    }
  }

  return 0;
}
