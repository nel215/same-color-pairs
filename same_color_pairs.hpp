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
inline double getTime() {  // TODO: Fix function name
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

const uint8_t Horizontal = 0;
const uint8_t Vertical = 1;

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
  RowAction() {
    removed = 0;
    intervals.clear();
  }
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
    const int n = row.size();
    const int m = n / 2;
    vector<priority_queue<State> > queue(m, priority_queue<State>());
    vector<State> history;
    set<uint32_t> searched;
    vector<char> initialRemoved(n, 0);
    for (int i=0; i < n; i++) {
      if (row[i] == '.') {
        initialRemoved[i] = 1;
      }
    }
    queue[0].push(State());
    for (int loop=0; loop < 20; loop++) {
      // TODO: the case of k == m-1
      for (int k=0; k < m-1; k++) {
        // skip
        if (queue[k].empty()) {
          continue;
        }
        vector<char> removed = initialRemoved;  // TODO: speedup
        State s = queue[k].top();

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
            } else if (row[j] != '.') {
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

struct EntireAction {
  uint8_t direction;
  uint8_t index;
  RowAction action;
  EntireAction() {
  }
  EntireAction(uint8_t d, uint8_t i, RowAction a) {
    direction = d;
    index = i;
    action = a;
  }
};

struct EntireState {
  int prev;
  int removed;
  vector<EntireAction> actions;
  EntireState() {
    prev = -1;
    removed = 0;
    actions.clear();
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
    return getTime() - startTime > 9.5;
  }

  void init(const vector<string> &board) {
    startTime = getTime();
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
        auto b = board;
        if (queue[q].empty()) {
          continue;
        }
        EntireState s = queue[q].top();
        while (s.prev >= 0) {
          cerr << s.prev << " " << s.removed << endl;
          for (auto &action : s.actions) {
            if (action.direction == Horizontal) {
              for (auto &interval : action.action.intervals) {
                for (int x=interval.from; x < interval.to; x++) {
                  b[action.index][x] = '.';
                }
              }
            } else if (action.direction == Vertical) {
              for (auto &interval : action.action.intervals) {
                for (int y=interval.from; y < interval.to; y++) {
                  b[y][action.index] = '.';
                }
              }
            }
          }
          s = history[s.prev];
        }
        s = queue[q].top();
        queue[q].pop();
        int prev = history.size();
        history.push_back(s);
        if (q == num_queue-1) {
          continue;
        }
        // Horizontal
        vector<EntireAction> actions;
        int removed = s.removed;
        for (int y=0; y < H; y++) {
          if (q > 0) cerr << b[y] << endl;
          RowAction act = rowSolver->solve(b[y]);
          if (act.removed == 0) {
            continue;
          }
          removed += act.removed;
          actions.push_back(EntireAction(Horizontal, y, act));
        }
        if (removed > s.removed) {
          EntireState nextState;
          nextState.prev = prev;
          nextState.removed = removed;
          nextState.actions = actions;
          queue[q+1].push(nextState);
        }
        // Vertical
        actions.clear();
        removed = s.removed;
        for (int x=0; x < W; x++) {
          string col = "";
          for (int y=0; y < H; y++) {
            col += b[y][x];
          }
          RowAction act = rowSolver->solve(col);
          if (act.removed == 0) {
            continue;
          }
          removed += act.removed;
          actions.push_back(EntireAction(Vertical, x, act));
        }
        if (removed > s.removed) {
          EntireState nextState;
          nextState.prev = prev;
          nextState.removed = removed;
          nextState.actions = actions;
          queue[q+1].push(nextState);
        }
        cerr << q << endl;
      }
      break;
    }
    cerr << H << " " << W << endl;
    vector<string> res;
    return res;
  }
};
#endif  // SAME_COLOR_PAIRS_HPP_
