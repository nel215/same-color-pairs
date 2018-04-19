#ifndef SAME_COLOR_PAIRS_HPP_
#define SAME_COLOR_PAIRS_HPP_
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>
#include <set>
#include <queue>
#include <stack>
#include <string>

using namespace std;

#ifdef LOCAL
const double ticks_per_sec = 3200000000;
#else
const double ticks_per_sec = 3000000000;
#endif  // LOCAL
inline double get_time() {
    uint32_t lo, hi;
    asm volatile ("rdtsc" : "=a" (lo), "=d" (hi));
    return (((uint64_t)hi << 32) | lo) / ticks_per_sec;
}

struct Board {
  int H, W, C;
  vector<vector<uint64_t> > data;
  Board(int _H, int _W, int _C) {
    H = _H;
    W = _W;
    C = _C;
    int size = (H*W/64) + ((H*W)%64 > 0 ? 1 : 0);
    data.assign(C, vector<uint64_t>(size, 0));
  }
  void set(int y, int x, int c) {
    int idx = y*W+x;
    data[c][idx>>5] |= 1 << (idx&63);
  }
};

class SameColorPairs {
  int H, W, C;

 public:
  void init(const vector<string> &board) {
    H = board.size();
    W = board[0].size();
    C = 0;
    for (int y=0; y < H; y++) {
      for (int x=0; x < W; x++) {
        C = max(C, board[y][x]-'0'+1);
      }
    }
  }
  struct State {
    int removed;
    int prev;
    int from;
    int to;
    State() {
      removed = 0;
      prev = -1;
      from = -1;
      to = -1;
    }
    bool operator<(const State &s)const{
      return removed < s.removed;
    }
  };
  void solveRow(const string &row) {
    const int n = row.size();
    const int m = n / 2;
    cerr << row << endl;
    vector<priority_queue<State> > queue(m, priority_queue<State>());
    vector<vector<State> > history(m, vector<State>());
    queue[0].push(State());
    for (int k=0; k < m-1; k++) {
      if (queue[k].empty()) {
        continue;
      }
      vector<char> removed(n, 0);  // TODO: speedup
      State s = queue[k].top();
      cerr << "s: " << s.removed << endl;
      int idx = k;
      while (idx > 0) {
        cerr << s.from << " " << s.to << endl;
        for (int i=s.from; i < s.to; i++) {
          removed[i] = 1;
        }
        idx--;
        s = history[idx][s.prev];
      }
      s = queue[k].top();
      queue[k].pop();
      int prev = history[k].size();
      history[k].push_back(s);
      for (int i=0; i < n-1; i++) {
        if (removed[i]) {
          continue;
        }
        stack<char> st;
        st.push(row[i]);
        int to = i;
        int alreadyRemoved = 0;
        int tmpRemoved = 0;
        for (int j=i+1; j < n; j++) {
          if (removed[j]) {
            tmpRemoved++;
            continue;
          }
          if ((!st.empty()) && st.top() == row[j]) {
            st.pop();
          } else {
            st.push(row[j]);
          }
          if (st.empty()) {
            to = j+1;
            alreadyRemoved = tmpRemoved;
          }
        }
        if (to > i) {
          State next;
          next.removed = s.removed - alreadyRemoved + (to - i);
          next.prev = prev;
          next.from = i;
          next.to = to;
          queue[k+1].push(next);
        }
      }
    }
  }
  vector<string> removePairs(vector<string> board) {
    init(board);
    vector<string> res;
    for (int y=0; y < H; y++) {
      solveRow(board[y]);
      break;
    }
    return res;
  }
};
#endif  // SAME_COLOR_PAIRS_HPP_
