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
#include <memory>

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

class XorShift {
  uint32_t x;
  uint32_t y;
  uint32_t z;
  uint32_t w;
 public:
  explicit XorShift(int seed) {
    std::srand(seed);
    x = std::rand();
    y = std::rand();
    z = std::rand();
    w = std::rand();
  }
  uint32_t rand() {
    uint32_t t = x ^ (x << 11);
    x = y; y = z; z = w;
    return w = (w ^ (w >> 19)) ^ (t ^ (t >> 8));
  }
};
XorShift rng(215);

class RowSolver {
  struct State {
    int removed;
    int prev;
    int from;
    int to;
    uint32_t hash;
    State() {
      removed = 0;
      prev = -1;
      from = -1;
      to = -1;
      hash = 0;
    }
    bool operator<(const State &s)const{
      return removed < s.removed;
    }
  };
  vector<uint32_t> hashSeed;

 public:
  RowSolver() {
    hashSeed.resize(100);
    for (int i=0; i < 100; i++) {
      hashSeed[i] = rng.rand();
    }
  }
  void solve(const string &row) {
    double start = get_time();
    const int n = row.size();
    const int m = n / 2;
    vector<priority_queue<State> > queue(m, priority_queue<State>());
    vector<vector<State> > history(m, vector<State>());
    queue[0].push(State());
    while (get_time()-start < 0.1) {
      for (int k=0; k < m-1; k++) {
        // skip
        if (queue[k].empty()) {
          continue;
        }
        vector<char> removed(n, 0);  // TODO: speedup
        State s = queue[k].top();
        cerr << "k: " << k << ", s: " << s.removed << ", n: " << n << endl;
        int idx = k;
        while (idx > 0) {
          // cerr << s.from << " " << s.to << endl;
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
          int alreadyRemoved = 0;
          uint32_t nextHash = s.hash;
          for (int j=i+1; j < n; j++) {
            if (removed[j]) {
              alreadyRemoved++;
              continue;
            }
            nextHash = nextHash ^ hashSeed[j];
            if ((!st.empty()) && st.top() == row[j]) {
              st.pop();
            } else {
              st.push(row[j]);
            }
            if (st.empty()) {
              State next;
              next.removed = s.removed - alreadyRemoved + (j + 1 - i);
              next.prev = prev;
              next.from = i;
              next.to = j+1;
              next.hash = nextHash;
              queue[k+1].push(next);
            }
          }
        }
      }
    }
    cerr << row << endl;
  }
};

class SameColorPairs {
  int H, W, C;
  unique_ptr<RowSolver> rowSolver;

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
    rowSolver = unique_ptr<RowSolver>(new RowSolver());
  }


  vector<string> removePairs(vector<string> board) {
    init(board);
    vector<string> res;
    for (int y=0; y < H; y++) {
      rowSolver->solve(board[y]);
      break;
    }
    return res;
  }
};
#endif  // SAME_COLOR_PAIRS_HPP_
