#ifndef SAME_COLOR_PAIRS_HPP_
#define SAME_COLOR_PAIRS_HPP_
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>
#include <set>
#include <string>

using namespace std;

class SameColorPairs {
 public:
  vector<string> removePairs(vector<string> board) {
    vector<string> ret;
    int H = board.size(), W = board[0].size();
    // find the first pair of horizontally adjacent tiles and remove them
    for (int i = 0; i < H; ++i)
      for (int j = 1; j < W; ++j)
        if (board[i][j] == board[i][j-1]) {
          ret.push_back(to_string(i) + " " + to_string(j) + " " + to_string(i) + " " + to_string(j-1));
          return ret;
        }
      return ret;
  }
};
#endif  // SAME_COLOR_PAIRS_HPP_
