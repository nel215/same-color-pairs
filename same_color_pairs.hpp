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

struct Interval {
  // [from, to)
  uint8_t from;
  uint8_t to;
  Interval(uint8_t f, uint8_t t) {
    from = f;
    to = t;
  }
};

struct RowAction {
  int removed;
  vector<Interval> intervals;
  RowAction(int _removed, vector<Interval> _intervals) {
    removed = _removed;
    intervals = _intervals;
  }
};

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
  RowAction solve(const string &row) {
    double start = get_time();
    const int n = row.size();
    const int m = n / 2;
    vector<priority_queue<State> > queue(m, priority_queue<State>());
    vector<State> history;
    set<uint32_t> searched;
    queue[0].push(State());
    while (get_time()-start < 0.1) {
      for (int k=0; k < m-1; k++) {
        // skip
        if (queue[k].empty()) {
          continue;
        }
        vector<char> removed(n, 0);  // TODO: speedup
        State s = queue[k].top();
        string d = row;
        while (s.prev >= 0) {
          for (int i=s.from; i < s.to; i++) {
            removed[i] = 1;
          }
          s = history[s.prev];
        }
        s = queue[k].top();
        queue[k].pop();
        int prev = history.size();
        history.push_back(s);
        for (int i=0; i < n-1; i++) {
          if (removed[i]) {
            continue;
          }
          stack<char> st;
          st.push(row[i]);
          int alreadyRemoved = 0;
          uint32_t nextHash = s.hash ^ hashSeed[i];
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
              if (searched.count(next.hash) == 0) {
                searched.insert(next.hash);
                queue[k+1].push(next);
              }
            }
          }
        }
      }
    }
    // TODO: speedup
    auto sortedHistory = history;
    sort(sortedHistory.begin(), sortedHistory.end());
    auto s = sortedHistory[static_cast<int>(sortedHistory.size()) - 1];
    int removed = s.removed;
    vector<Interval> intervals;
    while (s.prev >= 0) {
      intervals.push_back(Interval(s.from, s.to));
      s = history[s.prev];
    }
    reverse(intervals.begin(), intervals.end());
    return RowAction(removed, intervals);
  }
};

struct EntireState {
  int prev;
  int removed;
  EntireState() {
    prev = -1;
    removed = 0;
  }
  bool operator<(const EntireState &s)const{
    return removed < s.removed;
  }
};

class SameColorPairs {
  double startTime;
  int H, W, C;
  unique_ptr<RowSolver> rowSolver;

  inline bool mustFinish() {
    return get_time() - startTime > 9.5;
  }

  void init(const vector<string> &board) {
    startTime = get_time();
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

 public:
  vector<string> removePairs(vector<string> board) {
    init(board);
    const int num_queue = H * W / 2;
    vector<priority_queue<EntireState> > queue(num_queue);
    vector<EntireState> history;
    queue[0].push(EntireState());
    while (!mustFinish()) {
      for (int q=0; q < num_queue; q++) {
        if (queue[q].empty()) {
          continue;
        }
        EntireState s = queue[q].top();
        queue[q].pop();
        while (s.prev >= 0) {
          s = history[s.prev];
        }
        for (int y=0; y < H; y++) {
          RowAction act = rowSolver->solve(board[y]);
        }
      }
      break;
    }
    vector<string> res;
    return res;
  }
};
#endif  // SAME_COLOR_PAIRS_HPP_
