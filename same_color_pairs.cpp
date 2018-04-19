#include "same_color_pairs.hpp"


template<class T> void getVector(vector<T>& v) {
  for (int i = 0; i < v.size(); ++i)
  cin >> v[i];
}

int main() {
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
