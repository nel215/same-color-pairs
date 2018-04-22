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

// TODO: precalculate
inline uint8_t bitUp(uint8_t x) {
  return x - (x&(-x));
}

struct BIT {
  int H, W;
  vector<vector<uint16_t> > data;

 public:
  BIT(int _H, int _W): H(_H), W(_W) {
    data.assign(H+1, vector<uint16_t>(W+1, 0));
  }
  // [0, y)-[0, x)
  int sum(uint8_t _y, uint8_t _x) const {
    const uint8_t x64 = _x;
    const uint8_t x32 = bitUp(x64);
    const uint8_t x16 = bitUp(x32);
    const uint8_t x08 = bitUp(x16);
    const uint8_t x04 = bitUp(x08);
    const uint8_t x02 = bitUp(x04);
    const uint8_t x01 = bitUp(x02);
    int res = 0;
    for (int y=_y; y > 0; y-=y&(-y)) {
      const auto &d = data[y];
      res += d[x64] + d[x32] + d[x16] + d[x08] + d[x04] + d[x02] + d[x01];
    }
    return res;
  }
  int sum(uint8_t ymin, uint8_t xmin, uint8_t ymax, uint8_t xmax) const {
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

struct Cell {
  int8_t color;
  uint8_t ok;
  Cell() {
    color = -1;
    ok = 0;
  }
  bool okU() const {
    return (ok>>0) & 1;
  }
  bool okD() const {
    return (ok>>1) & 1;
  }
  bool okL() const {
    return (ok>>2) & 1;
  }
  bool okR() const {
    return (ok>>3) & 1;
  }
  bool okUL() const {
    return (ok>>4) & 1;
  }
  bool okUR() const {
    return (ok>>5) & 1;
  }
  bool okDL() const {
    return (ok>>6) & 1;
  }
  bool okDR() const {
    return (ok>>7) & 1;
  }
};

struct Board {
  const uint8_t H;
  const uint8_t W;
  const uint8_t C;
  const vector<string> &board;
  vector<Cell> data;
  explicit Board(const vector<string> &_board, const uint8_t _C):
    H(_board.size()), W(_board[0].size()), C(_C), board(_board) {
      data.assign((H+2)*(W+2), Cell());
      for (int y=0; y < H; y++) {
        for (int x=0; x < W; x++) {
          _get(y, x).color = board[y][x] - '0';
        }
      }
      for (int y=0; y < H; y++) {
        for (int x=0; x < W; x++) {
          const auto c = get(y, x).color;
          _get(y, x).ok |= (c == get(y-1, x).color) << 0;
          _get(y, x).ok |= (c == get(y+1, x).color) << 1;
          _get(y, x).ok |= (c == get(y, x-1).color) << 2;
          _get(y, x).ok |= (c == get(y, x+1).color) << 3;
          _get(y, x).ok |= (c == get(y-1, x-1).color) << 4;
          _get(y, x).ok |= (c == get(y-1, x+1).color) << 5;
          _get(y, x).ok |= (c == get(y+1, x-1).color) << 6;
          _get(y, x).ok |= (c == get(y+1, x+1).color) << 7;
        }
      }
  }
  Cell &_get(uint8_t y, uint8_t x) {
    return data[(y+1)*(W+2)+(x+1)];
  }
  const Cell &get(uint8_t y, uint8_t x) const {
    return data[(y+1)*(W+2)+(x+1)];
  }
};

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

  // TODO: make positions to be const
  double solveDiag(vector<BIT> bit,
                   vector<vector<pair<int, int> > > positions,
                   const vector<string> &board,
                   const Board &myboard) {
    vector<int> colors(C);
    for (int c=0; c < C; c++) {
      colors[c] = c;
      random_shuffle(positions[c].begin(), positions[c].end());
    }

    double res = 0;
    Mask mask(H);
    while (true) {
      bool updated = false;
      random_shuffle(colors.begin(), colors.end());
      for (auto c : colors) {
        auto &pos = positions[c];
        // cerr << c << " " << num_pos << endl;
        for (int i=0;; i++) {
start:
          if (i >= pos.size()) break;
          int iy = pos[i].first;
          int ix = pos[i].second;
          const auto &icell = myboard.get(iy, ix);
          // TODO: guardian
          // bool upok = (iy - 1 >= 0 && (mask.get(iy-1, ix) || board[iy-1][ix] == '0'+c));
          // bool downok = (iy + 1 < H && (mask.get(iy+1, ix) || board[iy+1][ix] == '0'+c));
          // bool leftok = (ix - 1 >= 0 && (mask.get(iy, ix-1) || board[iy][ix-1] == '0'+c));
          // bool rightok = (ix + 1 < W && (mask.get(iy, ix+1) || board[iy][ix+1] == '0'+c));
          bool upok = mask.get(iy-1, ix) || icell.okU();
          bool downok = mask.get(iy+1, ix) || icell.okD();
          bool leftok = mask.get(iy, ix-1) || icell.okL();
          bool rightok = mask.get(iy, ix+1) || icell.okR();
          bool okUL = mask.get(iy-1, ix-1) || icell.okUL();
          bool okUR = mask.get(iy-1, ix+1) || icell.okUR();
          bool okDL = mask.get(iy+1, ix-1) || icell.okDL();
          bool okDR = mask.get(iy+1, ix+1) || icell.okDR();

          for (int j=i+1; j < pos.size(); j++) {
            int jy = pos[j].first;
            int jx = pos[j].second;
            if ((!downok && jy > iy) || (!upok && jy < iy)) {
              continue;
            }
            if ((!rightok && jx > ix) || (!leftok && jx < ix)) {
              continue;
            }
            if (!okUL && jy < iy && jx < ix) {
              continue;
            }
            if (!okUR && jy < iy && jx > ix) {
              continue;
            }
            if (!okDL && jy > iy && jx < ix) {
              continue;
            }
            if (!okDR && jy > iy && jx > ix) {
              continue;
            }

            int ymax = max(iy, jy)+1;
            int xmax = max(ix, jx)+1;
            int ymin = min(iy, jy);
            int xmin = min(ix, jx);
            if (bit[c].sum(ymin, xmin, ymax, xmax) == bit[C].sum(ymin, xmin, ymax, xmax)) {
              mask.set(iy, ix);
              mask.set(jy, jx);
              swap(pos[j], pos[pos.size()-1]);
              pos.pop_back();
              swap(pos[i], pos[pos.size()-1]);
              pos.pop_back();
              bit[c].add(iy, ix, -1);
              bit[c].add(jy, jx, -1);
              bit[C].add(iy, ix, -1);
              bit[C].add(jy, jx, -1);
              res += 2;
              updated = true;
              goto start;
            }
          }
        }
      }
      if (!updated) {
        // for (int y=0; y < H; y++) {
        //   cerr << board[y] << endl;
        // }
        // cerr << endl;
        break;
      }
    }

    return H * W - res;
  }

 public:
  struct Mask {
    vector<uint64_t> data;
    explicit Mask(int H) {
      data.assign(2*H+2, 0);
    }
    bool get(int y, int x) const {
      y++;
      x++;
      return data[2*y+(x>>6)] & (1LL << (x&63));
    }
    void set(int y, int x) {
      y++;
      x++;
      data[2*y+(x>>6)] |= (1LL << (x&63));
    }
  };

  vector<string> removePairs(vector<string> board) {
    init(board);
    Board myboard(board, C);
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
      double tmp = solveDiag(bit, pos, board, myboard);
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
