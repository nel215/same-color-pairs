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
const double timeLimit = 8.9;
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
  const int H;
  const int W;
  const int C;
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
  Cell &_get(int y, int x) {
    return data[(y+1)*(W+2)+(x+1)];
  }
  const Cell &get(int y, int x) const {
    return data[(y+1)*(W+2)+(x+1)];
  }
};

class SameColorPairs {
  double startTime;
  int H, W, C;

  inline bool mustFinish() {
    return getTime() - startTime > timeLimit;
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

  void solveDiag(vector<BIT> bit,
                  Mask mask,
                   vector<vector<int> > &positions,
                   const Board &myboard,
                   int &deleted,
                   vector<int> &history,
                   const int target) {
    deleted = 0;
    vector<int> colors(C);
    vector<int> numPos(C, 0);
    for (int c=0; c < C; c++) {
      colors[c] = c;
      random_shuffle(positions[c].begin(), positions[c].end());
      numPos[c] = positions[c].size();
    }

    while (deleted < target) {
      bool updated = false;
      random_shuffle(colors.begin(), colors.end());
      for (auto c : colors) {
        auto &pos = positions[c];
        // cerr << c << " " << num_pos << endl;
        for (int i=0;; i++) {
start:
          if (i >= numPos[c]) break;
          int iy = pos[i]>>7;
          int ix = pos[i]&127;
          const auto &icell = myboard.get(iy, ix);
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
  vector<string> removePairs(vector<string> board) {
    init(board);
    Board myboard(board, C);
    double avg = 0;
    cerr << "H:" << H << "\tW:" << W << "\tC:" << C << endl;
    vector<BIT> bit(C+1, BIT(H, W));
    vector<vector<int> > pos(C, vector<int>());

    for (int y=0; y < H; y++) {
      for (int x=0; x < W; x++) {
        int c = board[y][x]-'0';
        pos[c].push_back(y*128+x);
        bit[c].add(y, x, 1);
        bit[C].add(y, x, 1);
      }
    }

    Mask mask(H);
    int preDeleted = 0;
    vector<int> preHistory(2*H*W);
    if (C >= 4 && H*W > 50*50) {
      int target = H*W-50*50;
      solveDiag(bit, mask, pos, myboard, preDeleted, preHistory, target);
    }
    cerr << "preDeleted:" << preDeleted << endl;

    vector<string> res;
    for (int i=0; i < preDeleted/2; i++) {
      int iy = preHistory[4*i+0];
      int ix = preHistory[4*i+1];
      int jy = preHistory[4*i+2];
      int jx = preHistory[4*i+3];
      mask.set(iy, ix);
      mask.set(jy, jx);
      board[iy][ix] = '.';
      board[jy][jx] = '.';
      bit[myboard.get(iy, ix).color].add(iy, ix, -1);
      bit[myboard.get(iy, ix).color].add(jy, jx, -1);
      bit[C].add(iy, ix, -1);
      bit[C].add(jy, jx, -1);
      stringstream ss;
      ss << iy << " ";
      ss << ix << " ";
      ss << jy << " ";
      ss << jx;
      res.push_back(ss.str());
    }

    pos.assign(C, vector<int>());
    for (int y=0; y < H; y++) {
      for (int x=0; x < W; x++) {
        if (board[y][x] == '.') {
          continue;
        }
        int c = board[y][x]-'0';
        pos[c].push_back(y*128+x);
      }
    }

    int bestDeleted = 0;
    vector<int> bestHistory(2*H*W);
    vector<int> history(2*H*W);
    double tried = 0;
    while (!mustFinish()) {
      int deleted;
      solveDiag(bit, mask, pos, myboard, deleted, history, H*W);
      if (deleted > bestDeleted) {
        bestDeleted = deleted;
        bestHistory = history;
      }
      avg += (H*W-deleted-preDeleted)*1.0;
      tried += 1.0;
    }
    avg /= tried;
    cerr << "best:" <<  (H*W-bestDeleted-preDeleted) << "\tavg:" << avg << "\ttried:" << tried << endl;
    for (int i=0; i < bestDeleted/2; i++) {
      stringstream ss;
      ss << bestHistory[4*i+0] << " ";
      ss << bestHistory[4*i+1] << " ";
      ss << bestHistory[4*i+2] << " ";
      ss << bestHistory[4*i+3];
      res.push_back(ss.str());
    }
    return res;
  }
};
#endif  // SAME_COLOR_PAIRS_HPP_
