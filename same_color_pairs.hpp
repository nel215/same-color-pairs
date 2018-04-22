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
#include <utility>

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

struct BIT {
  int H, W;
  vector<vector<int> > data;

 public:
  BIT(int _H, int _W): H(_H), W(_W) {
    data.assign(H+1, vector<int>(W+1, 0));
  }
  // [0, y)-[0, x)
  int sum(int _y, int _x) {
    int res = 0;
    for (int y=_y; y > 0; y-=y&(-y)) {
      for (int x=_x; x > 0; x-=x&(-x)) {
        res += data[y][x];
      }
    }
    return res;
  }
  int sum(int ymin, int xmin, int ymax, int xmax) {
    return sum(ymax, xmax) - sum(ymin, xmax) - sum(ymax, xmin) + sum(ymin, xmin);
  }
  void add(int _y, int _x, int v) {
    for (int y=_y+1; y <= H; y+=y&(-y)) {
      for (int x=_x+1; x <= W; x+=x&(-x)) {
        data[y][x] += v;
      }
    }
  }
};

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

class SameColorPairs {
  double startTime;
  int H, W, C;

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
  }

  double solveDiag(vector<BIT> bit,
                   vector<vector<pair<int, int> > > pos) {
    vector<int> colors(C);
    for (int c=0; c < C; c++) {
      colors[c] = c;
      random_shuffle(pos[c].begin(), pos[c].end());
    }

    double res = 0;
    Mask mask(H);
    while (true) {
      bool updated = false;
      for (int c=0; c < C; c++) {
        for (int i=0; i < pos[c].size(); i++) {
          int iy = pos[c][i].first;
          int ix = pos[c][i].second;
          if (mask.get(iy, ix)) {
            continue;
          }
          for (int j=i+1; j < pos[c].size(); j++) {
            int jy = pos[c][j].first;
            int jx = pos[c][j].second;
            if (mask.get(jy, jx)) {
              continue;
            }

            int ymax = max(iy, jy)+1;
            int xmax = max(ix, jx)+1;
            int ymin = min(iy, jy);
            int xmin = min(ix, jx);
            if (bit[c].sum(ymin, xmin, ymax, xmax) == bit[C].sum(ymin, xmin, ymax, xmax)) {
              mask.set(iy, ix);
              mask.set(jy, jx);
              bit[c].add(iy, ix, -1);
              bit[c].add(jy, jx, -1);
              bit[C].add(iy, ix, -1);
              bit[C].add(jy, jx, -1);
              updated = true;
              break;
            }
          }
        }
      }
      if (!updated) {
        for (int y=0; y < H; y++) {
          // cerr << board[y] << endl;
        }
        break;
      }
    }

    return H * W - res;
  }

  struct Mask {
    vector<uint64_t> data;
    explicit Mask(int H) {
      data.assign(2*H, 0);
    }
    bool get(int y, int x) const {
      return data[2*y+(x>>6)] & (1LL << (x&63));
    }
    void set(int y, int x) {
      data[2*y+(x>>6)] |= (1LL << (x&63));
    }
  };

 public:
  vector<string> removePairs(vector<string> board) {
    init(board);
    double best = H*W;
    double avg = 0;
    // int n = 10;
    int n = 1000000*10*10/H/W/C*10/H*10/W;
    cerr << n << endl;
    vector<BIT> bit(C+1, BIT(H, W));
    vector<vector<pair<int, int> > > pos(C, vector<pair<int, int> >());
    for (int y=0; y < H; y++) {
      for (int x=0; x < W; x++) {
        int c = board[y][x]-'0';
        pos[c].push_back(make_pair(y, x));
        bit[c].add(y, x, 1);
        bit[C].add(y, x, 1);
      }
    }
    for (int i=0; i < n; i++) {
      double tmp = solveDiag(bit, pos);
      // cerr << "s:" << tmp << endl;
      best = min(best, tmp);
      avg += tmp/n;
    }
    cerr << "best: " <<  best << " avg: " << avg << endl;
    // cerr << H << " " << W << " " << C << endl;
    vector<string> res;
    return res;
  }
};
#endif  // SAME_COLOR_PAIRS_HPP_
