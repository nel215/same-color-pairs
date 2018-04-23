#include <cassert>
#include "same_color_pairs.hpp"

void testBIT() {
  int H = 100;
  int W = 100;
  BIT bit(H, W);
  bit.add(2, 1, 1);
  assert(bit.sum(2, 1) == 0);
  assert(bit.sum(2, 2) == 0);
  assert(bit.sum(3, 1) == 0);
  assert(bit.sum(3, 2) == 1);
  assert(bit.sum(2, 1, 2, 1) == 0);
  assert(bit.sum(2, 1, 3, 2) == 1);
  bit.add(99, 99, 1);
  assert(bit.sum(99, 99, 100, 100) == 1);
}

void testMask() {
  SameColorPairs::Mask mask(10);
  for (int y=0; y < 10; y++) {
    for (int x=0; x < 100; x++) {
      assert(!mask.get(y, x));
      mask.set(y, x);
      assert(mask.get(y, x));
    }
  }
}

int main() {
  testBIT();
  SameColorPairs scp;
  int H;
  cin >> H;
  vector<string> board(H);
  for (int i = 0; i < H; ++i) {
    cin >> board[i];
  }

  vector<string> ret = scp.removePairs(board);
  cout << ret.size() << endl;
  for (int i = 0; i < static_cast<int>(ret.size()); ++i) {
    cout << ret[i] << endl;
  }
  cout.flush();
}
