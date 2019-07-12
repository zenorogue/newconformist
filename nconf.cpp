/** New Conformist
    Copyright (C) 2018 Zeno Rogue

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CAP_GD
#define CAP_GD 1
#endif

#ifndef CAP_DRAW
#define CAP_DRAW 1
#endif

#ifndef CAP_BMP
#define CAP_BMP (CAP_GD | CAP_DRAW)
#endif

#include "graph2.h"
#include <unistd.h>
#include <unordered_map>
#include <array>
#include <vector>
#include <map>
#include <cstdio>
#include <cmath>
#include <string>
#include <algorithm>
#include <functional>
#include <queue>
#include <complex>

namespace nconf {
using namespace graph2;

bool chessmap = false;
ipoint chesspos;

int zoomout = 1;

int lined_out;

typedef std::complex<ld> cld;

using std::isnan;
using std::vector;
using std::printf;
using std::string;
using std::pair;
using std::array;
using std::sort;
using std::make_pair;
using std::tie;
using std::tuple;
using std::min;
using std::max;
using std::make_tuple;
using std::queue;

int elim_order = 3;

ld spinspeed;

bool use_childsides = true;

bool view_error = false;
}

#include "mat.cpp"
#include "zebra.cpp"

namespace nconf {
int SX, SY;

// Side types.
// stype::standard is homeomorphic to disk
// stype::ring is homeomorphic to ring
// stype::fixed_ring is used for rings if we want to make the period divisible by the tesselation period
// stpe::fake is used for found regions which have more than 1 hole

enum class stype : int { standard, ring, fixed_ring, fake };

// Point types.
// For stype::standard, iside points are of type 'inside', and the boundary is: top, right_inf, bottom, left_inf.
// For stype::ring, the boundary is top (external) and bottom (internal), and the ring is split into three inside_* subtypes.

enum class ptype : char { outside, inside, inside_left_up, inside_left_down, top, bottom, left_inf, right_inf, marked };

bool inner(ptype t) { return int(t) > 0 && int(t) < 4; }
bool infinitary(ptype t) { return int(t) >= 6; }

bool inner_border(ptype t) { return t == ptype::inside || t == ptype::top || t == ptype::bottom; }

// the four directions

ipoint dv[4] = { ipoint(1, 0), ipoint(0, -1), ipoint(-1, 0), ipoint(0, 1) };  

void resize_pt();

bool draw_progress = true;
bool text_progress = true;

#if CAP_GD
// the bitmap used for the shape (called 'heart' because heart was the first bitmap shape)
bitmap heart;
#endif

typedef pair<struct datapoint*, ld> equation;

// information about a pixel

struct datapoint {
  cpoint x;
  ptype type, baktype;
  char state;
  int side=0;
  int pointorder;
  ld bonus;
  vector<equation> eqs;
  };

template<class T> struct vector2 : vector<vector<T>> {
  T& operator [] (ipoint i) { return (*this)[i.y][i.x]; }
  vector<T>& operator [] (int i) { return (*(vector<vector<T>>*)this) [i]; }
  // const T& operator [] const (ipoint i) { return arr[i.y][i.x]; }  
  void resize2(int X, int Y) { this->resize(Y); for(auto& row: *this) row.resize(X); }
  };

typedef vector2<datapoint> pointmap;

pointmap pts;

// information about a side

struct sideinfo {
  #if CAP_BMP
  bitmap img;
  vector<bitmap> img_band;
  #endif
  cpoint cscale;     // cscale[0] says how we should rescale the X coordinates -- we should divide them by cscale[0]
  ipoint inner_point;
  ld xcenter;
  ld period;         // period in band units
  ld period_unit;    // period unit (changed e.g. with -zebra)
  ld animshift;      // add this to the X coordinates
  vector<transmatrix> period_matrices;
  stype type;
  int id;
  vector<int> childsides;
  bool need_btd;
  vector<transmatrix> matrixlist, rmatrixlist;
  ld zero_shift;
  pointmap* submap;
  ipoint join;
  int parentid, rootid;
  transmatrix parentrel_matrix;
  int parentrel_x;
  #if CAP_BMP
  vector<bitmap> img_line;
  #endif
  
  };

vector<sideinfo> sides;

sideinfo& rootof(const sideinfo& si) { return sides[si.rootid]; }
}

#include "btd.cpp"

namespace nconf {
ld scalex = 1, scaley = 1;
int marginx = 32, marginy = 32;

void resize_pt() {
  pts.resize(SY);
  for(int y=0; y<SY; y++) pts[y].resize(SX);
  }

int current_side;

sideinfo& new_side(stype type) {
  int N = size(sides);
  sides.emplace_back();
  auto& side = sides.back();
  side.id = N;
  side.type = type;
  side.submap = &pts;
  side.period_unit = 1;
  side.parentid = N;
  side.rootid = N;
  side.animshift = 0;
  return side;
  }

sideinfo& single_side(stype type) {
  sides.clear();
  auto& side = new_side(type);
  current_side = side.id;
  return side;
  }

sideinfo& create_side(stype type) {
  auto& side = new_side(type);
  current_side = side.id;
  return side;
  }

sideinfo& cside() { return sides[current_side]; }
sideinfo& csideroot() { return rootof(cside()); }

int sqr(int a) { return a*a; }

// create a rectangular shape

void createb_rectangle() {
  single_side(stype::standard);
  resize_pt();
  for(int y=0; y<SY; y++) 
  for(int x=0; x<SX; x++) {
    auto& p = pts[y][x];
    p.type = ptype::inside;
    p.side = 0;
    if(y == 0 || y == SY-1 || x == 0 || x == SX-1) {
      if(y < SY/2) p.type = ptype::top;
      else if(y > SY/2) p.type = ptype::bottom;
      else if(x == 0) p.type = ptype::left_inf;
      else p.type = ptype::right_inf;
      }
    }
  }

// given the points supposed to become left_inf and right_inf, and
// the starting direction, split the rest of the boundary to
// top and bottom.

void split_boundary(pointmap& ptmap, ipoint axy, ipoint bxy, int d) {

  ptmap[axy].type = ptype::left_inf;

  ptype phase = ptype::bottom;

  ptmap[bxy].type = ptype::right_inf;
  bxy -= dv[d];
  
  for(int iter=0; iter<100000000; iter++) {
    d &= 3;
    auto& pt2 = ptmap[bxy + dv[d]];
    ptype nphase = ptype(int(phase)+2); // ugly
    if(pt2.type == nphase || pt2.type == phase) d++;
    else if(pt2.type == ptype::outside) { pt2.type = phase; d++; }
    else if(infinitary(pt2.type)) { 
      if(phase == ptype::bottom) phase = ptype::top;
      else break; 
      }
    else if(pt2.type == ptype::inside) { bxy += dv[d]; d--; }
    }
  }

ld hypot(ipoint a) { return std::hypot(a.x, a.y); }

// find the point on the boundary nearest to cxy

tuple<ipoint, int> boundary_point_near(pointmap& ptmap, ipoint cxy) {
  ld bestdist = 1e8;
  int ad;
  ipoint axy (0, 0);
    
  for(int x=1; x<SX-1; x++) for(int y=1; y<SY-1; y++) {
    ipoint xy(x, y);
    for(int d=0; d<4; d++)
      if(ptmap[xy].type == ptype::outside && ptmap[xy + dv[d]].type == ptype::inside) {
        ld dist = hypot(xy-cxy);
        if(dist < bestdist) bestdist = dist, axy = xy, ad = d;
        }
    }
  
  printf("bestdist = %lf %d,%d .. %d.%d\n", double(bestdist), cxy.x, cxy.y, axy.x, axy.y);
  
  return make_tuple(axy, ad);
  }

#if CAP_BMP
// compute SX and SY based on heart
void set_SXY(bitmap& heart) {
  int newSX = heart.s->w / scalex + marginx + marginx;
  if(newSX < SX) 
    marginx = (SX - heart.s->w / scalex) / 2;
  else SX = newSX;

  int newSY = heart.s->h / scaley + marginy + marginy;
  if(newSY < SY) 
    marginy = (SY - heart.s->h / scaley) / 2;
  else SY = newSY;

  resize_pt();
  }  
#endif

#if CAP_GD
void load_image_for_mapping(const string& fname) {
  heart = readPng(fname);
  errpixel = heart[0][0];
  set_SXY(heart);
  }
#endif

// create a circular shape

void createb_circle() {
  single_side(stype::standard);
  resize_pt();
  for(int y=0; y<SY; y++) 
  for(int x=0; x<SX; x++) {
    auto& p = pts[y][x];
    p.type = ptype::inside;
    p.side = 0;
    int u = sqr(2 * x - SX + 1) + sqr(2 * y - SY + 1);
    if(u >= sqr(min(SY, SX) - 5))
      p.type = ptype::outside;
    }
  auto [axy1, ad] = boundary_point_near(pts, {0, SY/2});
  auto [bxy1, bd] = boundary_point_near(pts, {SX-1, SY/2});
  printf("%d %d %d %d %d %d\n", axy1.x, axy1.y, ad, bxy1.x, bxy1.y, bd);
  
  split_boundary(pts, axy1, bxy1, bd^2);
  }

// trimming (obsolete)

int trim_x1 = 0, trim_y1 = 0, trim_x2 = 99999, trim_y2 = 99999;

void trim(int x1, int y1, int x2, int y2) {
  trim_x1 = x1;
  trim_y1 = y1;
  trim_x2 = x2;
  trim_y2 = y2;
  }

// translate bitmap coordinates to internal coordinates (taking margin and scale into account)

ipoint unmargin(ipoint xy) {
  return ipoint((xy.x-marginx)*scalex, (xy.y-marginy)*scaley);
  }

ipoint addmargin(ipoint xy) {
  return ipoint(xy.x/scalex+marginx, xy.y/scaley+marginy);
  }

#if CAP_BMP
unsigned& get_heart(ipoint xy) {
  return heart[unmargin(xy)];
  }
#endif

// prepare a ring for mapping, including the given point

#if CAP_BMP
void createb_outer(ipoint cxy) {
  create_side(stype::ring);
  
  cside().inner_point = cxy;
  auto inpixel = get_heart(cxy);

  queue<ipoint> boundary;
  
  while(cxy.x < SX && get_heart(cxy) == inpixel) cxy.x++;
  if(cxy.x == SX) die("nothing on the line");
  
  for(int y=0; y<SY; y++) 
  for(int x=0; x<SX; x++) {
    ipoint xy(x, y);
    auto& p = pts[y][x];
    p.baktype = p.type;

    if(x == 0) {
      p.type = ptype::top;
      boundary.emplace(1, y);
      }
    else if(y == 0) {
      p.type = ptype::top;
      boundary.emplace(x, 1);
      }
    else if(x == SX-1) {
      p.type = ptype::top;
      boundary.emplace(x-1, 1);
      }
    else if(y == SY-1) {
      p.type = ptype::top;
      boundary.emplace(1, y-1);
      }
    else if(get_heart(xy) != inpixel)
      p.type = ptype::bottom;
    else {
      p.side = current_side;
      if(x > cxy.x)
        p.type = ptype::inside;
      else if(y < cxy.y)
        p.type = ptype::inside_left_up;
      else
        p.type = ptype::inside_left_down;
      }
    }
  
  while(!boundary.empty()) {
    auto xy = boundary.front();
    boundary.pop();
    auto& p = pts[xy];
    if(p.type != ptype::bottom) continue;
    p.type = ptype::top;
    for(int d=0; d<4; d++)
      boundary.emplace(xy + dv[d]);
    }
  }
#endif

// prepare a stype::standard for mapping, including the given point

#if CAP_BMP
void createb_inner(ipoint axy, ipoint bxy) {
  create_side(stype::standard);

  cside().inner_point = axy;
  auto inpixel = heart[axy];
  printf("%x %x err %x\n", inpixel, heart[bxy], errpixel);
  if(heart[bxy] != inpixel) die("both pixels should be in");
  
  for(int y=0; y<SY; y++) 
  for(int x=0; x<SX; x++) {
    auto xy = ipoint(x, y);
    auto& p = pts[xy];

    int ax = (x-marginx)/scalex;
    int ay = (y-marginy)/scaley;
    bool trimmed = ax < trim_x1 || ax >= trim_x2 || ay < trim_y1 || ay >= trim_y2;

    p.side = 0;
    p.baktype = p.type;
    if(get_heart(xy) == inpixel && x && y && x < SX-1 && y < SY-1 && !trimmed)
      p.type = ptype::inside;
    else
      p.type = ptype::outside;
    }
  
  auto [axy1, ad] = boundary_point_near(pts, addmargin(axy));
  auto [bxy1, bd] = boundary_point_near(pts, addmargin(bxy));
  
  split_boundary(pts, axy1, bxy1, bd^2);
  }
#endif

// create the Hilbert curve shape

void create_hilbert(int lev, int pix, int border) {
  single_side(stype::standard);

  SY = SX = pix << lev;
  resize_pt();
  for(int y=0; y<SY; y++) 
  for(int x=0; x<SX; x++) 
    pts[y][x].side = 0, 
    pts[y][x].type = ptype::outside;
  ipoint wxy(0, 0);
  auto connection = [&] (int dir) {
    for(int dy=(dir==1?-border:border); dy<(dir==3?pix+border:pix-border); dy++)
    for(int dx=(dir==2?-border:border); dx<(dir==0?pix+border:pix-border); dx++)
      pts[dy+wxy.y*pix][dx+wxy.x*pix].type = ptype::inside;
    if(dir<4) wxy += dv[dir];
    };
  std::function<void(int,int,int)> hilbert_recursive = [&] (int maindir, int subdir, int l) {
    if(l == 0) return;
    hilbert_recursive(subdir, maindir, l-1);
    connection(subdir);
    hilbert_recursive(maindir, subdir, l-1);
    connection(maindir);
    hilbert_recursive(maindir, subdir, l-1);
    connection(subdir^2);
    hilbert_recursive(subdir^2, maindir^2, l-1);
    };
  pts[border-1][pix/2].type = ptype::left_inf;
  hilbert_recursive(0, 3, lev);
  connection(4);
  split_boundary(pts, {pix/2, border-1}, {SX-1-pix/2, border-1}, 1);
  }

void saveb(const string& s) {  
  FILE *f = fopen(s.c_str(), "wt");
  printf("%d %d\n", SX, SY);
  for(int y=0; y<SY; y++) {
    for(int x=0; x<SX; x++)
      fprintf(f, "%c", "X.-+TDLR" [int(pts[y][x].type)]);
    fprintf(f, "\n");
    }
  fclose(f);
  }

int mousex, mousey;
}

// 'unofficial' experiments that newconformist has been used for
#include "triangle.cpp"
#include "quincunx.cpp"
#include "spiral.cpp"

namespace nconf {

void klawisze();

bool paused, zoomed;
int zx, zy;

int itc(int a) {
  return min(a, 255);
  // if(a < 16) return a * 8;
  // return min(128 + (a - 16) / 16, 255);
  }

ld find_equation(vector<equation>& v, datapoint& p) {
  auto seek = std::lower_bound(v.begin(), v.end(), equation{&p, -HUGE_VAL});
  if(seek != v.end() && seek->first == &p) return seek->second;
  return 0;
  }

// draw the state of computation during mapping

#if CAP_DRAW
void drawstates(pointmap& ptmap) {
  do {
    if(!draw_progress) return;
    initGraph(SX/zoomout, SY/zoomout, "conformist", false);
    int statecolors[4] = {
      0x000080, 0x00FF00, 0x000000, 0x00FFFF };
  
    auto& pt = zoomed ? ptmap[(mousey+zy)/4][(mousex+zx)/4] : ptmap[mousey][mousex];
    // printf("eqs = %d\n", isize(pt.eqs));
  
    for(int y=0; y<SY/zoomout; y++)
    for(int x=0; x<SX/zoomout; x++) {
      auto& p = zoomed ? ptmap[(y*zoomout+zy)/4][(x*zoomout+zx)/4] : ptmap[y*zoomout][x*zoomout];
      screen[y][x] = statecolors[int(p.state)];
      if(p.state == 0) switch(p.type) {
        case ptype::top: screen[y][x] = 0xFFFFFF; break;
        case ptype::bottom: screen[y][x] = 0xFF00FF; break;
        default: ;
        }
      part(screen[y][x], 2) = itc(isize(p.eqs));
      if(find_equation(pt.eqs, p)) part(screen[y][x], 2) = 0x80;
      if(p.type == ptype::inside_left_up) part(screen[y][x], 0) |= 0x20;
      if(p.type == ptype::inside_left_down) part(screen[y][x], 0) |= 0x40;
      }
    screen.draw();
    
    SDL_Event event;
    SDL_Delay(1);
    int ev;
    while((ev = SDL_PollEvent(&event))) switch (event.type) {
      case SDL_QUIT:
        exit(1);
        return;
  
      case SDL_MOUSEMOTION: {
        mousex = event.motion.x;
        mousey = event.motion.y;
        break;
        }
      
      case SDL_KEYDOWN: {
        int key = event.key.keysym.sym;
        // int uni = event.key.keysym.unicode;

        if(key == 'p') paused = !paused;
        if(key == 'z') zx = mousex*zoomout*3, zy = mousey*zoomout*3, zoomed = !zoomed;
        
        break;
        }
      
      }
  
  } while(paused);
  
  }
#endif

array<ipoint, 4> find_neighbors(pointmap& ptmap, ipoint xy) {
  array<ipoint, 4> res;
  for(int i=0; i<4; i++) res[i] = xy + dv[i];
  
  int ax = xy.x, ay = xy.y;
  
  if(elim_order == 3) {
    if((ax+ay) & 1) { ptmap[xy].pointorder = 1000; return res; }
    tie(ax, ay) = make_pair(ax+ay + (1<<16), ax-ay + (1<<16));
    }
  
  int axv = 0, ayv = 0;
  
  while(!(ax&1)) ax >>= 1, axv++;
  while(!(ay&1)) ay >>= 1, ayv++;
  
  if(elim_order == 0 || elim_order == 3)
    ptmap[xy].pointorder = 2000 + max(axv, ayv) * 1000 + (axv>ayv ? 500 : 0) + min(axv, ayv);

  if(elim_order == 1)
    ptmap[xy].pointorder = xy.x + xy.y;

  if(elim_order == 2)
    ptmap[xy].pointorder = xy.x + xy.y + ((xy.x ^ xy.y) & 1 ? 2000 : 0);

  return res;
  }

// compute the mapping, after every pixel/datapoint has been given a type

bool pretty_borders = false;

vector<ipoint> allpoints;

void build_equations(pointmap& ptmap, int i, bool fast) {
  printf("Building eqs, i=%d\n", i);
  
  for(int y=0; y<SY; y++) 
  for(int x=0; x<SX; x++) {
    auto& p = ptmap[y][x];
    p.state = 0;

    if(pretty_borders && i == 0 ? !inner_border(p.type) : !inner(p.type)) continue;
    p.state = 1;
    p.eqs.clear();
    p.bonus = 0;

    p.x[i] = 0;
    p.bonus = 0;
    
    ipoint xy(x, y);
    
    auto nei = find_neighbors(ptmap, xy);

    if(i == 0) {
      int xp = 0;
      for(auto np: nei) {
        auto t = ptmap[np].type;
        if(inner(t) || infinitary(t) || (pretty_borders && inner_border(t))) xp++;
        }
      for(auto np: nei) {
        auto& p2 = ptmap[np];
        if(p2.type == ptype::left_inf) p.bonus += 0;
        else if(p2.type == ptype::right_inf) p.bonus += 1./xp;
        else if(pretty_borders ? inner_border(p2.type) : inner(p2.type)) {
          p.eqs.emplace_back(&p2, 1./xp);
          if(p.type == ptype::inside_left_up && p2.type == ptype::inside_left_down) p.bonus += 1./xp;
          if(p.type == ptype::inside_left_down && p2.type == ptype::inside_left_up) p.bonus -= 1./xp;
          }
        }
      }
    if(i == 1) {
      for(auto np: nei) {
        auto& p2 = ptmap[np];
        if(p2.type == ptype::top) p.bonus += 0; 
        else if(p2.type == ptype::bottom) p.bonus += 1./4; 
        else if(infinitary(p2.type)) p.bonus += 1./8;
        else {
          p.eqs.emplace_back(&p2, 1./4);
          }
        }
      }
    sort(p.eqs.begin(), p.eqs.end());
    }
  
  allpoints.clear();
  for(int y=0; y<SY; y++) 
  for(int x=0; x<SX; x++) if(ptmap[y][x].state == 1) allpoints.push_back({x, y});
  if(fast) sort(allpoints.begin(), allpoints.end(), [&ptmap] (auto p1, auto p2) { return ptmap[p1].pointorder < ptmap[p2].pointorder; });
  }

void eliminate(pointmap& ptmap, ipoint co) {
  auto &p = ptmap[co];
  ld self = find_equation(p.eqs, p);
  
  if(self) {
    if(self == 1) {
      printf("Variable eliminated at (%d,%d)\n", co.x, co.y);
      p.state = 3;
      }
    else {
      ld fac = 1 / (1 - self);
      auto b = p.eqs.begin();
      for(auto &pa: p.eqs) if(pa.first != &p) *(b++) = {pa.first, pa.second * fac};
      p.eqs.resize(b - p.eqs.begin());
      p.bonus *= fac;
      }
    }
  
  for(auto& pa: p.eqs) {
    auto& p2 = *pa.first;

    ld mirror = find_equation(p2.eqs, p);
    if(!mirror) continue;

    p2.bonus += p.bonus * mirror;
    vector<equation> new_equations;
    new_equations.reserve(max(isize(p2.eqs), isize(p.eqs)) + 4);
    auto old = p2.eqs.begin();
    auto extra = p.eqs.begin();
    while(old != p2.eqs.end() && extra != p.eqs.end())
      if(old->first < extra->first) {
        if(old->first == &p) old++;
        else new_equations.push_back(*old), old++;
        }
      else if(old->first > extra->first)
        new_equations.emplace_back(extra->first, extra->second * mirror), extra++;
      else
        new_equations.emplace_back(old->first, old->second + extra->second * mirror), old++, extra++;
    while(old != p2.eqs.end()) {
      if(old->first == &p) old++;
      else new_equations.push_back(*old), old++;
      }
    while(extra != p.eqs.end())
      new_equations.emplace_back(extra->first, extra->second * mirror), extra++;
      
    p2.eqs = std::move(new_equations);
    }
  p.state = 2;
  }

void retrieve(pointmap& ptmap, int i) {
  printf("Solution retrieval\n");
  reverse(allpoints.begin(), allpoints.end());
  for(auto co: allpoints) {
    auto &p = ptmap[co];
    if(p.state != 2) continue;
    p.x[i] = p.bonus;
    for(auto& pa: p.eqs) p.x[i] += pa.second * pa.first->x[i];
    p.eqs = vector<equation> ();
    }
  allpoints.clear();
  }

void computemap(pointmap& ptmap) {

  for(int i=0; i<2; i++) {
    build_equations(ptmap, i, true);
    
    int lastt = SDL_GetTicks();
    printf("Gaussian elimination\n");
    int lastpct = -1, citer = 0;
    for(auto co: allpoints) {
      auto &p = ptmap[co];
      if(p.state != 1) continue;
      if(text_progress || draw_progress) {
        int cpct = citer * 1000 / size(allpoints);
        if(cpct != lastpct) {
          lastpct = cpct;
          if(text_progress) printf("  %d/1000 [%d]\n", cpct, isize(p.eqs));
          #if CAP_DRAW
          int nextt = SDL_GetTicks();
          if(nextt > lastt + 100) {
            drawstates(ptmap);
            lastt = SDL_GetTicks();
            }
          #endif
          }
        }
      citer++;
      
      eliminate(ptmap, co);
      }
    
    retrieve(ptmap, i);    
    printf("Done.\n");        
    }
  }

template<class T> void save(FILE *f, const T& x) {
  fwrite(&x, sizeof(T), 1, f);
  }

template<class T> void load(FILE *f, T& x) {
  fread(&x, sizeof(T), 1, f);
  }

void savemap(const string& fname) {
  FILE *f = fopen(fname.c_str(), "wb");
  if(!f) pdie("savemap");
  fwrite(&SX, sizeof(SX), 1, f);
  fwrite(&SY, sizeof(SY), 1, f);
  for(int y=0; y<SY; y++)
  for(int x=0; x<SX; x++) {
    auto& p = pts[y][x];
    save(f, p.x);
    int t = int(p.type);
    save(f, t);
    }
  fclose(f);
  }

void merge_sides() {
  if(current_side >= 0) {
    for(int y=0; y<SY; y++) 
    for(int x=0; x<SX; x++) {
      auto xy = ipoint(x, y);
      auto& p = pts[xy];
      if(!inner(p.type))
        p.type = p.baktype;
      }
    }
  }

void loadmap(const string& fname) {
  auto& side = single_side(stype::standard);

  FILE *f = fopen(fname.c_str(), "rb");
  if(!f) pdie("loadmap");
  fread(&SX, sizeof(SX), 1, f);
  fread(&SY, sizeof(SY), 1, f);
  resize_pt();
  for(int y=0; y<SY; y++)
  for(int x=0; x<SX; x++) {
    auto& p = pts[y][x];
    load(f, p.x);
    int t; load(f, t); p.type = ptype(t);
    if(p.type == ptype::inside_left_up) side.type = stype::ring;
    p.side = 0;
    }
  fclose(f);
  }

void loadmap2(const string& fname) {
  auto& side = new_side(stype::standard);
  FILE *f = fopen(fname.c_str(), "rb");
  if(!f) pdie("loadmap2");
  int iSX, iSY;
  fread(&iSX, sizeof(iSX), 1, f);
  fread(&iSY, sizeof(iSY), 1, f);
  if(iSX != SX || iSY != SY) die("map size mismatch\n");
  for(int y=0; y<SY; y++)
  for(int x=0; x<SX; x++) {
    datapoint dp; 
    load(f, dp.x);
    int t; load(f, t); dp.type = ptype(t);
    if(inner(dp.type)) {
      dp.side = side.id;
      pts[y][x] = dp;
      if(dp.type == ptype::inside_left_up) side.type = stype::ring;
      }
    }
  fclose(f);
  current_side = side.id; 
  }

void loadmap_join(const string& fname, ipoint xy) {
  FILE *f = fopen(fname.c_str(), "rb");
  if(!f) pdie("loadmap_join");
  
  auto& side = new_side(stype::standard);
  side.parentid = current_side;
  side.rootid = cside().rootid;
  cside().childsides.push_back(side.id);
  side.submap = new pointmap;
  side.join = addmargin(xy);
  auto &epts = *side.submap;
    
  int iSX, iSY;
  fread(&iSX, sizeof(iSX), 1, f);
  fread(&iSY, sizeof(iSY), 1, f);
  if(iSX != SX || iSY != SY) die("map size mismatch\n");

  epts.resize2(SX, SY);

  for(int y=0; y<SY; y++)
  for(int x=0; x<SX; x++) {
    auto& p = epts[y][x];
    load(f, p.x);
    int t; load(f, t); p.type = ptype(t);
    p.side = side.id;
    }

  fclose(f);
  current_side = side.id; 
  }

// how should be linearly transform the current harmonic mapping to make it conformal
// ([1] should be 0 and is ignored, we only have to scale the x coordinate by multiplying
// by get_conformity(...)[0]))
cpoint get_conformity(int x, int y, sideinfo& side) {

  auto& gpts = *side.submap;
  
  auto vzero = gpts[y][x].x;
  array<cpoint, 2> v = {gpts[y][x+1].x - vzero, gpts[y+1][x].x - vzero };
  
  ld det = v[0] ^ v[1];
  
  cpoint ba2 = cpoint{v[1][1], -v[0][1]} / det;
  cpoint ca2 = cpoint{-v[1][0], v[0][0]} / det;
  
  ld bad = (ca2|ba2) / (ba2|ba2);
  ld good = (ca2^ba2) / (ba2|ba2);
  
  return cpoint{good, bad};
  }  

bool mark_sides, no_images;

int notypeside = 0xFFD500;
int boundary_color = 0x000000;

int bbnd = 100;

ld intdif(ld z) {
  z = z - floor(z);
  if(z > .5) z = 1-z;
  return z;
  }

// the following routines are for -viewerror

ld am[2][2];

cpoint diskpoint(int x, int y) {
  cpoint c = pts[y][x].x;
  auto [vx,vy] = unband(c, sides[0], -sides[0].xcenter);
  hyperpoint p = equirectangular(vx, vy);
  cpoint pt = hyper_to_disk(p);
  return pt;
  }

ld max_error;

void compute_am() {

  cpoint sum_x = {0, 0};
  cpoint sum_y = {0, 0};
  cpoint sum_xx = {0, 0};
  cpoint sum_xy = {0, 0};
  cpoint sum_yy = {0, 0};
  
  int n = 0;
  
  for(int cy=0; cy<SY; cy++)
  for(int cx=0; cx<SX; cx++) {
    auto& p = pts[cy][cx];
    if(p.type != ptype::inside) continue;
    auto y = diskpoint(cx, cy);
    if(isnan(y[0]) || isnan(y[1]) || std::hypot(y[0], y[1]) > .9) continue;
    cpoint x = { ld(cx), ld(cy) };
    for(int i=0; i<2; i++) {
      sum_x[i] += x[i];
      sum_xx[i] += x[i] * x[i];
      sum_y[i] += y[i];
      sum_yy[i] += y[i] * y[i];
      sum_xy[i] += x[i] * y[i];
      }
    n++;
    }
  
  /*for(int a = 0; a < 5; a ++) {
    cpoint x = { a, a };
    cpoint y = { 2*a+3, 2*a+3 };
    for(int i=0; i<2; i++) {
      sum_x[i] += x[i];
      sum_xx[i] += x[i] * x[i];
      sum_y[i] += y[i];
      sum_yy[i] += y[i] * y[i];
      sum_xy[i] += x[i] * y[i];
      n++;
      }
    }*/

  for(int i=0; i<2; i++) {
    am[1][i] = (sum_xy[i] - sum_x[i] * sum_y[i] / n) / (sum_xx[i] - sum_x[i] * sum_x[i] / n);
    am[0][i] = (sum_y[i] - am[1][i] * sum_x[i]) / n;
    printf("X %lf Y %lf XX %lf XY %lf am = %lf %lf n = %d\n", double(sum_x[i]), double(sum_y[i]), double(sum_xx[i]), double(sum_xy[i]), double(am[0][i]), double(am[1][i]), n);
    }

  for(int cy=0; cy<SY; cy++)
  for(int cx=0; cx<SX; cx++) {
    auto& p = pts[cy][cx];
    if(p.type != ptype::inside) continue;
    auto y = diskpoint(cx, cy);
    if(isnan(y[0]) || isnan(y[1]) || std::hypot(y[0], y[1]) > .99999) continue;
    cpoint x = { ld(cx), ld(cy) };
    for(int i=0; i<2; i++) x[i] = x[i] * am[1][i] + am[0][i];
    cpoint dif = x-y;
    ld err = sqrt(dif|dif);
    if(err > max_error)
      max_error = err;
    }
  
  printf("max error = %lf (%lf pixels)\n", double(max_error), double(max_error * SX / 2));  
  }

#if CAP_BMP
bitmap cheetah;
#endif

// draw the result image (on the given bitmap, which could be the screen)

#if CAP_BMP
void mark_outside(bitmap& b, int x, int y) {
  b[y][x] = notypeside;

  if(lined_out)
  for(int ax=-1; ax<=1; ax++)
  for(int ay=-1; ay<=1; ay++) {
    int x1 = x*zoomout + ax * lined_out;
    int y1 = y*zoomout + ay * lined_out;
    if(x1 >= 0 && y1 >= 0 && x1 < SX && y1 < SY && pts[y1][x1].type != ptype::outside)
      b[y][x] = boundary_color;
    }
  }
#endif

ld hypot(cpoint p) { return std::hypot(p[0], p[1]); }

ld factor = 1;

int icrad = 270;

bool use_back = false;

#if CAP_BMP
void draw_cheetah(bitmap &b) {

  ld xd = sides[0].xcenter;
  ld yd = 0;
  
  ld alpha = 0;
  
  if(true) {
    printf("mousex = %d mousey = %d\n", mousex, mousey);
    auto& p = pts[mousey*zoomout][mousex*zoomout];
    int siid = p.side;
    if(siid < isize(sides) && inner(p.type)) {
      xd = p.x[0];
      yd = unband(p.x, sides[siid], -xd).second;

      auto& p1 = pts[mousey*zoomout][mousex*zoomout+1];
      if(p1.side == siid) {
        auto [vx,vy] = unband(p1.x, sides[siid], -xd);
        hyperpoint p = mul(ypush(-yd), equirectangular(vx, vy));
        cpoint pt = hyper_to_disk(p);
        alpha = atan2(pt[1], pt[0]);
        }
      }
    }

/*
  int csize = 250;
  vector2<cpoint> cheetah_closest;
  vector2<color> cheetah_what;
  cpoint cerror {-100, -100};

  cheetah_closest.resize2(csize, csize);
  cheetah_what.resize2(csize, csize);
  for(int y=0; y<csize; y++)
  for(int x=0; x<csize; x++) {
    int r = hypot(y - csize/2, x - csize/2);
    if(r < icrad) cheetah_closest[y][x] = cerror, cheetah_what[y][x] = 0xFF00FF;
    else cheetah_closest[y][x] = cpoint{ld(x),ld(y)}, cheetah_what[y][x] = r < icrad+lined_out ? 0 : -1;
    }
  */

  for(int y=0; y<SY/zoomout; y++)
  for(int x=0; x<SX/zoomout; x++) 
    if(use_back) {
      for(int p=0; p<4; p++)
        part(b[y][x], p) = part(cheetah[y*zoomout-marginy][x*zoomout-marginx], p) / 4;
      }
    else b[y][x] = (y * 255 / (SY/zoomout)) << 8;

  // vector<ipoint> cheetah_queue;

  for(int y=0; y<SY; y++)
  for(int x=0; x<SX; x++) {
    auto& p = pts[y][x];

    if(p.type != ptype::outside) {
      int siid = p.side;
      if(siid >= isize(sides)) continue;
      auto& si = sides[siid];
      
      auto ixy = unmargin(ipoint{x,y});

      // auto& px = b[y/zoomout][x/zoomout];
      
      // px = cheetah[ixy.y][ixy.x];
      cpoint c = pts[y][x].x;
      auto [vx,vy] = unband(c, si, -xd);
      hyperpoint p = mul(spin(alpha), mul(ypush(-yd), equirectangular(vx, vy)));
      cpoint pt = hyper_to_disk(p);
      
      pt = (cpoint{ld(mousex),ld(mousey)} + pt * icrad) * (1-factor) + cpoint{ld(x)/zoomout,ld(y)/zoomout} * factor;
      ipoint ipt(pt[0], pt[1]);
      
      b[ipt] = cheetah[ixy];
      }
    }
  
  /*
  for(int i=0; i<isize(cheetah_queue); i++) {
    ipoint ipt = cheetah_queue[i];    
    if(ipt.x < 0 || ipt.y < 0) exit(1);
    cpoint cpt { ld(ipt.x), ld(ipt.y) };
    bool changed = false;
    for(auto k: dv) if(hypot(cpt-cheetah_closest[ipt+k]) < hypot(cpt-cheetah_closest[ipt]) - 1e-9) {
      // printf("%d %d :: %Lf %Lf -> %Lf %Lf\n", ipt.x, ipt.y, cheetah_closest[ipt][0], cheetah_closest[ipt][1], cheetah_closest[ipt+k][0], cheetah_closest[ipt+k][1]);
      changed = true; 
      cheetah_closest[ipt] = cheetah_closest[ipt+k];
      cheetah_what[ipt] = cheetah_what[ipt+k];
      }
    if(changed) for(auto k: dv) cheetah_queue.push_back(ipt+k);
    }

  for(int y=0; y<csize; y++)
  for(int x=0; x<csize; x++) if(cheetah_what[y][x] != -1)
    b[y][x] = cheetah_what[y][x];
  */
  }
#endif

#if CAP_BMP
void draw_point(bitmap& b, int x, int y) {
  auto& p = pts[y][x];

  if(p.type == ptype::outside) {
    mark_outside(b, x, y);
    return;
    }
  
  int siid = p.side;
  int tsiid = siid;
  if(siid >= isize(sides)) return;
  auto& si = sides[siid];

  if(si.img_band.size() && !no_images) {
    ld by = p.x[1];
    ld bx = p.x[0] - si.xcenter;
    bx /= si.cscale[0];
    int sizy = si.img_band[0].s->h;
    by *= sizy;
    bx *= sizy;
    int totalx = 0;
    for(auto& bandimg: si.img_band) totalx += bandimg.s->w;
    bx += totalx/2.;
    bx -= int(bx) / totalx * totalx;
    if(bx < 0) bx += totalx;
    for(auto& bandimg: si.img_band) { 
      if(bx < bandimg.s->w) {
        b[y][x] = bandimg[by][bx];
        break;
        }
      else bx -= bandimg.s->w;
      }
    }
  else if(no_images || (!si.img.s && !isize(si.img_line))) {
    // int qsides = size(sides);
    auto& pix = b[y][x];
    auto& pt = pts[y][x];
    if(chessmap) {
      auto ch = pts[chesspos];
      static bool chess_shown;
      if(!chess_shown) {
        chess_shown = true;
        printf("chess = %lf %lf\n", double(ch.x[0]), double(ch.x[1]));
        printf("chess to band = %lf %lf\n", double(ch.x[0]), double(ch.x[1]));
        }

      ld y = pt.x[1];
      y *= 2; y -= 1; // -1 .. 1
      y *= M_PI / 2;
      y = -2 * atanh(tan(y/2));
      
      int sca = 1;
      
      ld coshy = cosh(y);
      
      ld xx = (pt.x[0] - ch.x[0]) * M_PI /  si.cscale[0];
      
      xx *= sca; y *= sca;
      
      if(abs(y) < .1 || intdif(xx) < .05 / coshy || (intdif(y) < .1 && intdif(xx) < .25 / coshy))
        pix = 0;
      else
        pix = 0xFFFFFF;

      // pix = ((int(100 + sca * (pt.x[0] - ch.x[0]) / si.cscale[0]) + int(100 + sca /M_PI * y /* pt.x[1] */)) & 1) ? 0xFFFFFF : 0x0;
      // pix = ((int(100 + M_PI * sca * (pt.x[0] - ch.x[0]) / si.cscale[0]) + int(100 + sca * y /* pt.x[1] */)) & 1) ? 0xFFFFFF : 0x0;
      }
    else if(view_error) {
      cpoint c = pts[y][x].x;
      auto [vx,vy] = unband(c, si, -si.xcenter);
      hyperpoint p = equirectangular(vx, vy);
      cpoint pt = hyper_to_disk(p);
      cpoint bycoord;
      bycoord[0] = am[1][0] * x + am[0][0];
      bycoord[1] = am[1][1] * y + am[0][1];
      
      part(pix, 0) = 128 + 100 * (pt[0] - bycoord[0])  / max_error;
      part(pix, 1) = 128 + 100 * (pt[1] - bycoord[1])  / max_error;
      part(pix, 2) = 128 + 100 * std::hypot(pt[0] - bycoord[0], pt[1] - bycoord[1])  / max_error;
      
      if(x == mousex && y == mousey)
        printf("is = %lf %lf should be = %lf %lf\n", double(pt[0]), double(pt[1]), double(bycoord[0]), double(bycoord[1]));
      }
    else {
      part(pix, 0) = int(255 & int(255 * pt.x[0]));
      part(pix, 1) = int(255 & int(255 * pt.x[1]));        
      }
    }
  else {
    ld xval = 0, yval = 0;
    auto dc = band_to_disk(x, y, si, tsiid, xval, yval);
    auto& tsi = sides[tsiid];
    if(yval >= bbnd || yval <= -bbnd)
      b[y][x] = boundary_color;
    else if(isize(tsi.img_line) > 0 && isize(tsi.img_line) > xval - tsi.zero_shift) {
      ld nxval = xval - tsi.zero_shift;
      int xi = int(nxval);
      hyperpoint p = equirectangular(nxval-xi, yval);
      cpoint pt = hyper_to_disk(p);  
      pt = (cpoint{1, 1} + pt) * (tsi.img_line[xi].s->h / 2);
      b[y][x] = tsi.img_line[xi][pt[1]][pt[0]];
      }
    else if(si.img.s)
      b[y][x] = si.img[dc[1]][dc[0]];
    else
      b[y][x] = boundary_color;
    }
  
  if(mark_sides) {
    part(b[y][x], 2) = part(b[y][x], 2) * 1 / 16 + tsiid * 240 / (isize(sides) - 1);
    }
  }
#endif

#if CAP_BMP
void draw(bitmap &b) {
  construct_btd();
  b.belocked();

  if(spiral_mode)
    draw_spiral(b, SDL_GetTicks() / 5000.);
  else if(triangle_mode)
    draw_triangle(b);
  else if(cheetah.s)
    draw_cheetah(b);
  else {
    for(int y=0; y<SY; y++)
    for(int x=0; x<SX; x++) {
      draw_point(b, x, y);
      }
    }

  if(mark_sides)
    for(auto& si: sides) {
      int c = rand();
      b[si.join] = c;
      for(auto k: dv) 
        b[si.join+k] = c;
      }
  b.draw();
  }
#endif

ld anim_speed;

bool break_loop = false;

// handle keys after drawing the result image

#if CAP_DRAW
void klawisze() {
  SDL_Event event;
  SDL_Delay(1);
  int ev;
  while((ev = SDL_PollEvent(&event))) switch (event.type) {
    case SDL_QUIT:
      break_loop = true;
      return;

    case SDL_MOUSEBUTTONDOWN: {
      break;
      }
    
    case SDL_MOUSEMOTION: {
      mousex = event.motion.x;
      mousey = event.motion.y;
      break;
      }
    
    case SDL_KEYDOWN: {
      int key = event.key.keysym.sym;
      // int uni = event.key.keysym.unicode;
      
      if(cheetah.s) {
        if(key == '0') factor = 0;
        if(key == '1') factor = 0.125;
        if(key == '2') factor = 0.25;
        if(key == '3') factor = 0.375;
        if(key == '4') factor = 0.5;
        if(key == '5') factor = 0.625;
        if(key == '6') factor = 0.75;
        if(key == '7') factor = 0.875;
        if(key == '8') factor = 1;
        }
      else {
        if(key == '1') anim_speed = .01;
        if(key == '2') anim_speed = .03;
        if(key == '3') anim_speed = .1;
        if(key == '4') anim_speed = .3;
        if(key == '5') anim_speed = 1;
        if(key == '6') anim_speed = 3;
        if(key == '7') anim_speed = 9;
        if(key == '8') anim_speed = 27;
        if(key == '9') anim_speed = 81;
        if(key == '0') anim_speed = 0;
        if(key == 'r') anim_speed = -anim_speed;
        }
      
      if(key == 'i') {
        auto& p = pts[mousey][mousex];
        printf("this point (%d,%d) is %lf %lf\n", mousex, mousey, double(p.x[0]), double(p.x[1]));
        }
      
      if(key == 'u') {
        use_childsides = !use_childsides;
        printf("use_childsides = %d\n", use_childsides);
        }
      
      if(key == 'm') {
        mark_sides = !mark_sides;
        printf("mark_sides = %d\n", mark_sides);
        }
      if(key == 'c') {
        no_images = !no_images;
        printf("no_images = %d\n", no_images);
        }
      if(key == 'q') 
        break_loop = true;
      
      break;
      }
    
    }
  }
#endif

#if CAP_GD
void load_image(const string& fname) {
  csideroot().img = readPng(fname); 
  }

void load_image_band(const string& fname) {
  csideroot().img_band.push_back(readPng(fname));
  }
#endif

bool need_measure = true;

// this mostly computes the cscale for the given side

void measure(sideinfo& si) {
  
  if(si.type == stype::fake) return;
  int sii = si.id;
  auto& gpts = *si.submap;  

  vector<ld> cscs[2];
  
  printf("side #%d (type %d), x %p\n", si.id, int(si.type), &gpts);
  
  int valid = 0, total = 0;

  for(int y=0; y<SY; y++)
  for(int x=0; x<SX; x++) if(gpts[y][x].side == sii) if(inner(gpts[y][x].type)) total++;
  
  for(int y=0; y<SY; y++)
  for(int x=0; x<SX; x++) if(gpts[y][x].side == sii) if(inner(gpts[y][x].type) && gpts[y+1][x].side == sii && inner(gpts[y+1][x].type) && gpts[y][x+1].side == sii && inner(gpts[y][x+1].type)) {
    valid++;
    auto c = get_conformity(x, y, si);
    if(isnan(c[0])) continue;
    if(isnan(c[1])) continue;
    for(int i: {0,1}) cscs[i].push_back(c[i]);
    }
  
  for(int i: {0,1}) sort(cscs[i].begin(), cscs[i].end());  
  int q = size(cscs[0]);
  
  printf("point counts: %d/%d/%d\n", q, valid, total);
  if(!q) die("no valid points");
  
  si.cscale = { cscs[0][q/2], cscs[1][q/2] };
  
  /* for(int i=0; i<=16; i++) {
    int id = (q * i - 1) / 16;
    printf("[%2d] %Lf %Lf\n", i, cscs[0][id], cscs[1][id]);
    } */

  printf("conformity: %Lf %Lf (%d points)\n", si.cscale[0], si.cscale[1], q);
  
  vector<ld> xes;
  for(int y=0; y<SY; y++)
  for(int x=0; x<SX; x++) if(gpts[y][x].side == sii) if(inner(gpts[y][x].type)) xes.push_back(gpts[y][x].x[0]);
  sort(xes.begin(), xes.end());
  si.xcenter = xes[size(xes) / 2] + si.animshift;
  printf("xcenter: %Lf\n", si.xcenter);

  if(si.type != stype::standard && si.period > 0) {
    printf("period multiple: %Lf\n", M_PI / si.cscale[0] / si.period);
  
    if(si.type == stype::fixed_ring) {
      ld pmul = M_PI / si.cscale[0] / si.period;
      pmul = int(pmul + .5);
      si.cscale[0] = M_PI / si.period / pmul;
      printf("fixed period multiple: %Lf\n", M_PI / si.cscale[0] / si.period);
      }
    }
  }

void measure_if_needed() {
  if(need_measure) {
    need_measure = false;
    for(auto& si: sides) measure(si);
    }
  }

#if CAP_DRAW
void ui() {
  measure_if_needed();
  initGraph(SX / zoomout, SY / zoomout, "conformist", false);
  int t = SDL_GetTicks();
  break_loop = false;
  while(!break_loop) {
    int t1 = SDL_GetTicks();
    if(spinspeed) cspin += (t1 - t) * spinspeed;
    for(auto& si: sides) {
      ld x = si.cscale[0] * anim_speed * (t1-t) / 1000.;
      si.xcenter += x;
      si.animshift += x;
      }
    t = t1;
    
    draw(screen);
    klawisze();
    }
  }
#endif

#if CAP_BMP
void export_image(const string& fname) {
  measure_if_needed();
  bitmap b = emptyBitmap(SX, SY);
  draw(b);
  for(int y=0; y<SY; y++)
  for(int x=0; x<SX; x++)
    b[y][x] |= 0xFF000000;
  writePng(fname.c_str(), b);
  }
#endif

#if CAP_BMP
void export_video(ld spd, int cnt, const string& fname) {
  measure_if_needed();
  bitmap b = emptyBitmap(SX, SY);
  for(int i=0; i<cnt; i++) {

    if(spiral_mode)
      draw_spiral(b, 2 * M_PI * (i+.5) / cnt);
    else
      draw(b);

    for(int y=0; y<SY; y++)
    for(int x=0; x<SX; x++)
      b[y][x] |= 0xFF000000;
    char buf[100000];
    snprintf(buf, 100000, fname.c_str(), i);
    writePng(buf, b);
    printf("Saving: %s\n", buf);
    for(auto& si: sides) {
      ld x = si.cscale[0] * spd;
      si.xcenter += x;
      si.animshift += x;
      }
    }
  }
#endif

#if CAP_BMP
void export_cheetah(int cnt, const string& fname) {
  measure_if_needed();
  mousex = 790; mousey = 332;
  bitmap b = emptyBitmap(SX/zoomout, SY/zoomout);
  for(int i=0; i<cnt; i++) {
    factor = i * 1.0 / (cnt-1);
    draw(b);

    for(int y=0; y<SY; y++)
    for(int x=0; x<SX; x++)
      b[y][x] |= 0xFF000000;
    char buf[100000];
    snprintf(buf, 100000, fname.c_str(), i);
    writePng(buf, b);
    printf("Saving: %s\n", buf);
    }
  }
#endif
}

#include "automapper.cpp"

int main(int argc, char **argv) {
  using namespace nconf;
  int i = 1;
  auto next_arg = [&] () { if(i == argc) die("not enough arguments"); return argv[i++]; };
  auto next_arg_ipoint = [&] () { int x = atoi(next_arg()); int y = atoi(next_arg()); return ipoint(x, y); };
  while(i < argc) {
    string s = next_arg();
    if(s == "-scale") scalex = scaley = atof(next_arg());
    else if(s == "-margin") marginx = marginy = atoi(next_arg());
    else if(s == "-rectangle") {
      SX = atoi(next_arg());
      SY = atoi(next_arg());
      createb_rectangle();
      }
    else if(s == "-circle") {
      SX = SY = atoi(next_arg());
      createb_circle();
      }
    #if CAP_GD
    else if(s == "-mim") load_image_for_mapping(next_arg());
    #endif
    #if CAP_BMP
    else if(s == "-cbo") {
      createb_outer(next_arg_ipoint());
      }
    #endif
    else if(s == "-trim") {
      int x1 = atoi(next_arg());
      int y1 = atoi(next_arg());
      int x2 = atoi(next_arg());
      int y2 = atoi(next_arg());
      trim(x1, y1, x2, y2);
      }
    #if CAP_BMP
    else if(s == "-cbi") {
      ipoint axy = next_arg_ipoint();
      ipoint bxy = next_arg_ipoint();
      createb_inner(axy, bxy);
      }
    #endif
    #if CAP_BMP
    else if(s == "-mapat") {
      auto_map_at(addmargin(next_arg_ipoint()));
      }
    else if(s == "-mapall") {
      auto_map_all();
      }
    #endif
    else if(s == "-sb") saveb(next_arg());
    else if(s == "-q") draw_progress = false, text_progress = false;
    else if(s == "-qt") text_progress = false;
    else if(s == "-qd") draw_progress = false;
    else if(s == "-cm") computemap(pts);
    else if(s == "-sm") savemap(next_arg());
    else if(s == "-sma") save_all_maps(next_arg());
    else if(s == "-lm") loadmap(next_arg());
    else if(s == "-lm2") loadmap2(next_arg());
    else if(s == "-lma") load_all_maps(next_arg());
    else if(s == "-lmj") {
      string s = next_arg();
      loadmap_join(s, next_arg_ipoint());
      }
    else if(s == "-back") {
      current_side = cside().parentid;
      }
    else if(s == "-side") {
      current_side = atoi(next_arg());
      if(current_side < 0 || current_side >= isize(sides))
        pdie("error: wrong side number");
      }
    #if CAP_BMP
    else if(s == "-li") load_image(next_arg());
    else if(s == "-lband") load_image_band(next_arg());
    else if(s == "-lbands") {
      int i0 = atoi(next_arg());
      int i1 = atoi(next_arg());
      string s = next_arg();
      for(int i=i0; i<=i1; i++) {
        char buf[100000];
        snprintf(buf, 100000, s.c_str(), i);
        printf("Loading image #%d: %s\n", i, buf);
        load_image_band(buf);
        }
      }
    #endif
    else if(s == "-zebra") csideroot().period_unit = zebra_period, csideroot().period_matrices = zebra_matrices;
    else if(s == "-p45") csideroot().period_unit = period_45, csideroot().period_matrices = matrices_45;
    else if(s == "-p46") csideroot().period_unit = period_46, csideroot().period_matrices = matrices_46;
    else if(s == "-period") csideroot().period = csideroot().period_unit * atof(next_arg());
    else if(s == "-fix") csideroot().type = stype::fixed_ring;
    else if(s == "-ash") csideroot().animshift += atof(next_arg());
    #if CAP_DRAW
    else if(s == "-draw") ui();
    #endif
    #if CAP_GD
    else if(s == "-export") export_image(next_arg());
    #endif
    else if(s == "-spinspeed") spinspeed = atof(next_arg());
    else if(s == "-marksides") mark_sides = true;
    #if CAP_BMP
    else if(s == "-bandlen") {
      auto& si = csideroot();
      if(si.img_band.empty()) die("no bands to measure in -bandlen");
      int totalx = 0;
      for(auto& bandimg: si.img_band) totalx += bandimg.s->w;
      int y = si.img_band[0].s->h;
      printf("x = %d y = %d\n", totalx, y);
      printf("To make a loop, speed times count should be %lf\n", totalx * 1. / y);
      }
    #endif
    #if CAP_GD
    else if(s == "-exportv") {
      ld speed = atof(next_arg());
      int cnt = atoi(next_arg());
      export_video(speed, cnt, next_arg());
      }
    #endif
    else if(s == "-killside") {
      for(int y=0; y<SY; y++)
      for(int x=0; x<SX; x++) {
        auto& p = pts[y][x];
        if(p.side == current_side)
          p.type = ptype::outside;
        }
      cside().type = stype::fake;
      }
    else if(s == "-ntsblack") {
      notypeside = 0;
      }
    else if(s == "-ntswhite") {
      notypeside = 0xFFFFFF;
      }
    else if(s == "-btblack") {
      boundary_color = 0;
      }
    else if(s == "-btwhite") {
      boundary_color = 0xFFFFFF;
      }
    else if(s == "-btbnd") {
      bbnd = atoi(next_arg());
      }
    else if(s == "-hilbert") {
      int lev = atoi(next_arg());
      int pix = atoi(next_arg());
      int border = atoi(next_arg());
      create_hilbert(lev, pix, border);
      }
    else if(s == "-chessmap") {
      chessmap = true;
      chesspos = next_arg_ipoint();
      }
    else if(s == "-eo") {
      elim_order = atoi(next_arg());
      }
    else if(s == "-triangle") {
      create_triangle(atoi(next_arg()));
      }
    else if(s == "-diamond") {
      create_diamond(atoi(next_arg()));
      }
    else if(s == "-spiral") {
      single_side(stype::standard);
      spiral_mode = true;
      need_measure = false;
      SX = atoi(next_arg());
      SY = atoi(next_arg());
      }
    else if(s == "-mergesides")
      merge_sides();
    else if(s == "-lineout")
      lined_out = atoi(next_arg());
    else if(s == "-notype")
      notypeside = strtol(next_arg(), NULL, 16);
    else if(s == "-boundcolor")
      boundary_color = strtol(next_arg(), NULL, 16);
    else if(s == "-viewerror") {
      view_error = true;
      measure_if_needed();
      compute_am();
      }
    else if(s == "-joinparams") {
      join_epsilon = atof(next_arg());
      join_distance = atoi(next_arg());
      join_y = atof(next_arg());
      }
    else if(s == "-joinoff") {
      join_epsilon = -1;
      join_distance = 9999;
      }
    else if(s == "-tm")
      triangle_mode = true;
    else if(s == "-quincunx")
      do_quincunx();
    #if CAP_GD
    else if(s == "-lquincunx") {
      ld scale = atof(next_arg());
      load_image_for_quincunx(next_arg(), scale);
      }
    #endif
    else if(s == "-cvlgen")
      create_viewlist(current_side, next_arg());
    #if CAP_GD
    else if(s == "-cvlimg")
      read_viewlist(current_side, next_arg());
    #endif
    #if CAP_GD
    else if(s == "-cheetah")
      cheetah = readPng(next_arg());
    #endif
    else if(s == "-zo")
      zoomout = atoi(next_arg());
    #if CAP_GD
    else if(s == "-excheetah") {
      int cnt = atoi(next_arg());
      export_cheetah(cnt, next_arg());
      }
    #endif
    else die("unrecognized argument: " + s);
    }

  return 0;
  }
