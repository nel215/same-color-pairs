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
const double timeLimit = 6.0;
#else
const double ticks_per_sec = 3000000000;
const double timeLimit = 9.05;
#endif  // LOCAL
inline double getTime() {
    uint32_t lo, hi;
    asm volatile ("rdtsc" : "=a" (lo), "=d" (hi));
    return (((uint64_t)hi << 32) | lo) / ticks_per_sec;
}

inline uint8_t bitUp(uint8_t x) {
  return x - (x&(-x));
}

struct BIT {
  uint8_t H, W;
  vector<vector<int> > data;

 public:
  BIT(uint8_t _H, uint8_t _W): H(_H), W(_W) {
    data.assign(H+1, vector<int>(W+1, 0));
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
  void add(uint8_t _y, uint8_t _x, int v) {
    for (uint8_t y=_y+1; y <= H; y+=y&(-y)) {
      for (uint8_t x=_x+1; x <= W; x+=x&(-x)) {
        data[y][x] += v;
      }
    }
  }
  void copy(BIT &dst) const {
    for (int y=0; y < H+1; y++) {
      for (int x=0; x < W+1; x++) {
        dst.data[y][x] = data[y][x];
      }
    }
  }
};

struct Mask {
  vector<uint64_t> data;
  explicit Mask(int H) {
    data.assign(2*H+4, 0);
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
  void reset() {
    for (int i=0; i < data.size(); i++) {
      data[i] = 0;
    }
  }
};

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
  const int H;
  const int W;
  const int C;
  vector<Cell> data;
  explicit Board(const vector<string> &_board, const uint8_t _C):
    H(_board.size()), W(_board[0].size()), C(_C) {
      data.assign((H+2)*(W+2), Cell());
      for (int y=0; y < H; y++) {
        for (int x=0; x < W; x++) {
          _get(y, x).color = _board[y][x] - '0';
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
  Cell &_get(int y, int x) {
    return data[(y+1)*(W+2)+(x+1)];
  }
  const Cell &get(int y, int x) const {
    return data[(y+1)*(W+2)+(x+1)];
  }
};

class SameColorPairs {
  unique_ptr<Board> board;
  double startTime;
  int H, W, C;
  bool swapped;
  vector<BIT> srcBIT;
  vector<BIT> bit;

  inline bool mustFinish() {
    return getTime() - startTime > timeLimit;
  }

  void init(const vector<string> &_board) {
    startTime = getTime();
    swapped = false;
    H = _board.size();
    W = _board[0].size();
    if (W < H) {
      swapped = true;
      swap(H, W);
    }

    C = 0;
    vector<string> tmp(H, string(W, ' '));
    for (int y=0; y < _board.size(); y++) {
      for (int x=0; x < _board[y].size(); x++) {
        C = max(C, _board[y][x]-'0'+1);
        if (swapped) {
          tmp[x][y] = _board[y][x];
        } else {
          tmp[y][x] = _board[y][x];
        }
      }
    }
    board = unique_ptr<Board>(new Board(tmp, C));
    // construct BIT
    srcBIT.assign(C+1, BIT(H, W));
    for (int y=0; y < H; y++) {
      for (int x=0; x < W; x++) {
        int c = board->get(y, x).color;
        srcBIT[c].add(y, x, 1);
        srcBIT[C].add(y, x, 1);
      }
    }
    bit = srcBIT;
  }

  void solveDiag(Mask &mask,
                 vector<vector<int> > &positions,
                 const vector<uint8_t> &colors,
                 int &deleted,
                 vector<int> &history) {
    deleted = 0;
    int numPos[6];
    for (int c=0; c < C; c++) {
      srcBIT[c].copy(bit[c]);
      numPos[c] = positions[c].size();
    }
    srcBIT[C].copy(bit[C]);
    mask.reset();

    while (1) {
      bool updated = false;
      for (auto c : colors) {
        auto &pos = positions[c];
        for (int i=0;; i++) {
start:
          if (i >= numPos[c]) break;
          int iy = pos[i]>>7;
          int ix = pos[i]&127;
          const auto &icell = board->get(iy, ix);
          bool okU = mask.get(iy-1, ix) || icell.okU();
          bool okD = mask.get(iy+1, ix) || icell.okD();
          bool okL = mask.get(iy, ix-1) || icell.okL();
          bool okR = mask.get(iy, ix+1) || icell.okR();
          bool okUL = mask.get(iy-1, ix-1) || icell.okUL();
          bool okUR = mask.get(iy-1, ix+1) || icell.okUR();
          bool okDL = mask.get(iy+1, ix-1) || icell.okDL();
          bool okDR = mask.get(iy+1, ix+1) || icell.okDR();

          for (int j=i+1; j < numPos[c]; j++) {
            int jy = pos[j]>>7;
            int jx = pos[j]&127;
            if ((!okD && jy > iy) || (!okU && jy < iy)) {
              continue;
            }
            if ((!okR && jx > ix) || (!okL && jx < ix)) {
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
              history[2*deleted+0] = iy;
              history[2*deleted+1] = ix;
              mask.set(iy, ix);
              deleted++;
              history[2*deleted+0] = jy;
              history[2*deleted+1] = jx;
              deleted++;
              mask.set(jy, jx);
              swap(pos[j], pos[numPos[c]-1]);
              numPos[c]--;
              swap(pos[i], pos[numPos[c]-1]);
              numPos[c]--;
              bit[c].add(iy, ix, -1);
              bit[c].add(jy, jx, -1);
              bit[C].add(iy, ix, -1);
              bit[C].add(jy, jx, -1);
              updated = true;
              goto start;
            }
          }
        }
      }
      if (!updated) {
        break;
      }
    }
  }

 public:
  vector<string> removePairs(vector<string> _board) {
    init(_board);
    double avg = 0;
    cerr << "H:" << H << "\tW:" << W << "\tC:" << C << "\tswapped:" << swapped << endl;
    vector<uint8_t> colors(C);
    vector<vector<int> > positions(C, vector<int>());
    Mask mask(H);

    for (int c=0; c < C; c++) {
      colors[c] = c;
    }
    for (int y=0; y < H; y++) {
      for (int x=0; x < W; x++) {
        int c = board->get(y, x).color;
        positions[c].push_back(y*128+x);
      }
    }

    int bestDeleted = 0;
    vector<int> bestHistory;
    vector<int> history(4*H*W);
    int tried = 0;
    while (!mustFinish()) {
      if ((tried & 255) == 0) {
        random_shuffle(colors.begin(), colors.end());
      }
      int deleted;
      solveDiag(mask, positions, colors, deleted, history);
      if (deleted > bestDeleted) {
        bestDeleted = deleted;
        bestHistory = history;
      }
      avg += (H*W-deleted)*1.0;
      tried += 1;
      if (bestDeleted == H*W) break;
    }
    avg /= tried;
    cerr << "best:" <<  (H*W-bestDeleted) << "\tavg:" << avg << "\ttried:" << tried << endl;

    vector<string> res(bestDeleted/2);
    char buf[32];
    for (int i=0; i < bestDeleted/2; i++) {
      int idx = 0;
      if (swapped) {
        swap(bestHistory[4*i+0], bestHistory[4*i+1]);
        swap(bestHistory[4*i+2], bestHistory[4*i+3]);
      }
      for (int j=0; j < 4; j++) {
        const int h = bestHistory[4*i+j];
        if (h >= 10) buf[idx++] = '0' + h/10;
        buf[idx++] = '0' + h%10;
        buf[idx++] = ' ';
      }
      res[i] = string(buf, buf+idx-1);
    }
    return res;
  }
};
#endif  // SAME_COLOR_PAIRS_HPP_
