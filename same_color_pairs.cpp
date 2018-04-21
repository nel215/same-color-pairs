#include <cassert>
#include "same_color_pairs.hpp"

void testBIT() {
  int H = 5;
  int W = 2;
  BIT bit(H, W);
  bit.add(2, 1, 1);
  assert(bit.sum(2, 1) == 0);
  assert(bit.sum(2, 2) == 0);
  assert(bit.sum(3, 1) == 0);
  assert(bit.sum(3, 2) == 1);
  assert(bit.sum(2, 1, 2, 1) == 0);
  assert(bit.sum(2, 1, 3, 2) == 1);
}

int main() {
  testBIT();
  SameColorPairs scp;
  int H;
  cin >> H;
  vector<string> board(H);
  getVector(board);

  vector<string> ret = scp.removePairs(board);
  cout << ret.size() << endl;
  for (int i = 0; i < static_cast<int>(ret.size()); ++i) {
    cout << ret[i] << endl;
  }
  cout.flush();
}
