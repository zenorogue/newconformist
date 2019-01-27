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

bool chessmap = false;
ipoint chesspos;

int lined_out;

typedef std::complex<ld> cld;

using namespace std;

int elim_order = 3;

ld spinspeed;

bool use_childsides = true;

bool view_error = false;

#include "mat.cpp"
#include "zebra.cpp"

int SX, SY;

ipoint dv[4] = { ipoint(1, 0), ipoint(0, -1), ipoint(-1, 0), ipoint(0, 1) };  

void resize_pt();

bool draw_progress = true;
bool text_progress = true;

bitmap heart;

typedef pair<struct datapoint*, ld> equation;

struct datapoint {
  cpoint x;
  char type, baktype, state;
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

struct sideinfo {
  bitmap img;
  vector<bitmap> img_band;
  cpoint cscale;
  ipoint inner_point;
  ld xcenter;
  ld period;
  ld period_unit;
  ld animshift;
  vector<transmatrix> period_matrices;
  int type;
  int id;
  vector<int> childsides;
  bool need_btd;
  vector<transmatrix> matrixlist, rmatrixlist;
  ld zero_shift;
  pointmap* submap;
  ipoint join;
  int parentid, rootid;
  };

vector<sideinfo> sides;

sideinfo& rootof(const sideinfo& si) { return sides[si.rootid]; }

#include "btd.cpp"

ld scalex = 1, scaley = 1;
int marginx = 32, marginy = 32;

void resize_pt() {
  pts.resize(SY);
  for(int y=0; y<SY; y++) pts[y].resize(SX);
  }

int current_side;

sideinfo& new_side(int type) {
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

sideinfo& single_side(int type) {
  sides.clear();
  auto& side = new_side(type);
  current_side = side.id;
  return side;
  }

sideinfo& create_side(int type) {
  auto& side = new_side(type);
  current_side = side.id;
  return side;
  }

sideinfo& cside() { return sides[current_side]; }
sideinfo& csideroot() { return rootof(cside()); }

int sqr(int a) { return a*a; }

void createb_rectangle() {
  single_side(0);
  resize_pt();
  for(int y=0; y<SY; y++) 
  for(int x=0; x<SX; x++) {
    auto& p = pts[y][x];
    p.type = 1;
    p.side = 0;
    if(y == 0 || y == SY-1 || x == 0 || x == SX-1) {
      if(y < SY/2) p.type = 4;
      else if(y > SY/2) p.type = 5;
      else if(x == 0) p.type = 6;
      else p.type = 7;
      }
    }
  }

void split_boundary(pointmap& ptmap, ipoint axy, ipoint bxy, int d) {

  ptmap[axy].type = 6;

  int phase = 5;

  ptmap[bxy].type = 7;
  bxy -= dv[d];
  
  for(int iter=0; iter<100000; iter++) {
    d &= 3;
    auto& pt2 = ptmap[bxy + dv[d]];
    if(pt2.type == phase+2 || pt2.type == phase) d++;
    else if(pt2.type == 0) { pt2.type = phase; d++; }
    else if(pt2.type == 6 || pt2.type == 7) { phase--; if(phase == 3) break; }
    else if(pt2.type == 1) { bxy += dv[d]; d--; }
    }
  }

ld hypot(ipoint a) { return hypot(a.x, a.y); }

tuple<ipoint, int> boundary_point_near(pointmap& ptmap, ipoint cxy) {
  ld bestdist = 1e8;
  int ad;
  ipoint axy;
    
  for(int x=1; x<SX-1; x++) for(int y=1; y<SY-1; y++) {
    ipoint xy(x, y);
    for(int d=0; d<4; d++)
      if(ptmap[xy].type == 0 && ptmap[xy + dv[d]].type == 1) {
        ld dist = hypot(xy-cxy);
        if(dist < bestdist) bestdist = dist, axy = xy, ad = d;
        }
    }
  
  return make_tuple(axy, ad);
  }

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

void load_image_for_mapping(const string& fname) {
  heart = readPng(fname);
  errpixel = heart[0][0];
  set_SXY(heart);
  }

void createb_circle() {
  single_side(0);
  resize_pt();
  for(int y=0; y<SY; y++) 
  for(int x=0; x<SX; x++) {
    auto& p = pts[y][x];
    p.type = 1;
    p.side = 0;
    int u = sqr(2 * x - SX + 1) + sqr(2 * y - SY + 1);
    if(u >= sqr(min(SY, SX) - 5))
      p.type = 0;
    if(y == 0 || y == SY-1 || x == 0 || x == SX-1) {
      if(y < SY/2) p.type = 4;
      else if(y > SY/2) p.type = 5;
      else if(x == 0) p.type = 6;
      else p.type = 7;
      }
    }
  auto [axy1, ad] = boundary_point_near(pts, {0, SY/2});
  auto [bxy1, bd] = boundary_point_near(pts, {SX-1, SY/2});
  printf("%d %d %d %d %d %d\n", axy1.x, axy1.y, ad, bxy1.x, bxy1.y, bd);
  
  split_boundary(pts, axy1, bxy1, bd^2);
  }

int trim_x1 = 0, trim_y1 = 0, trim_x2 = 99999, trim_y2 = 99999;

void trim(int x1, int y1, int x2, int y2) {
  trim_x1 = x1;
  trim_y1 = y1;
  trim_x2 = x2;
  trim_y2 = y2;
  }

ipoint unmargin(ipoint xy) {
  return ipoint((xy.x-marginx)*scalex, (xy.y-marginy)*scaley);
  }

ipoint addmargin(ipoint xy) {
  return ipoint(xy.x/scalex+marginx, xy.y/scaley+marginy);
  }

unsigned& get_heart(ipoint xy) {
  return heart[unmargin(xy)];
  }

void createb_outer(ipoint cxy) {
  create_side(1);
  
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
      p.type = 4;
      boundary.emplace(1, y);
      }
    else if(y == 0) {
      p.type = 4;
      boundary.emplace(x, 1);
      }
    else if(x == SX-1) {
      p.type = 4;
      boundary.emplace(x-1, 1);
      }
    else if(y == SY-1) {
      p.type = 4;
      boundary.emplace(1, y-1);
      }
    else if(get_heart(xy) != inpixel)
      p.type = 5;
    else {
      p.side = current_side;
      if(x > cxy.x)
        p.type = 1;
      else if(y < cxy.y)
        p.type = 2;
      else
        p.type = 3;
      }
    }
  
  while(!boundary.empty()) {
    auto xy = boundary.front();
    boundary.pop();
    auto& p = pts[xy];
    if(p.type != 5) continue;
    p.type = 4;
    for(int d=0; d<4; d++)
      boundary.emplace(xy + dv[d]);
    }
  }

void createb_inner(ipoint axy, ipoint bxy) {
  create_side(0);

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
      p.type = 1;
    else
      p.type = 0;
    }
  
  auto [axy1, ad] = boundary_point_near(pts, addmargin(axy));
  auto [bxy1, bd] = boundary_point_near(pts, addmargin(bxy));
  
  split_boundary(pts, axy1, bxy1, bd^2);
  }

// Hilbert curve

void create_hilbert(int lev, int pix, int border) {
  single_side(0);

  SY = SX = pix << lev;
  resize_pt();
  for(int y=0; y<SY; y++) 
  for(int x=0; x<SX; x++) 
    pts[y][x].side = 0, 
    pts[y][x].type = 0;
  ipoint wxy(0, 0);
  auto connection = [&] (int dir) {
    for(int dy=(dir==1?-border:border); dy<(dir==3?pix+border:pix-border); dy++)
    for(int dx=(dir==2?-border:border); dx<(dir==0?pix+border:pix-border); dx++)
      pts[dy+wxy.y*pix][dx+wxy.x*pix].type = 1;
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
  pts[border-1][pix/2].type = 6;
  hilbert_recursive(0, 3, lev);
  connection(4);
  split_boundary(pts, {pix/2, border-1}, {SX-1-pix/2, border-1}, 1);
  }

void saveb(const string& s) {  
  FILE *f = fopen(s.c_str(), "wt");
  printf("%d %d\n", SX, SY);
  for(int y=0; y<SY; y++) {
    for(int x=0; x<SX; x++)
      fprintf(f, "%c", "X.-+TDLR" [pts[y][x].type]);
    fprintf(f, "\n");
    }
  fclose(f);
  }

int mousex, mousey;

#include "triangle.cpp"
#include "spiral.cpp"

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

void drawstates(pointmap& ptmap) {
  do {
    if(!draw_progress) return;
    initGraph(SX, SY, "conformist", false);
    int statecolors[4] = {
      0x000080, 0x00FF00, 0x000000, 0x00FFFF };
  
    auto& pt = zoomed ? ptmap[(mousey+zy)/4][(mousex+zx)/4] : ptmap[mousey][mousex];
    // printf("eqs = %d\n", isize(pt.eqs));
  
    for(int y=0; y<SY; y++)
    for(int x=0; x<SX; x++) {
      auto& p = zoomed ? ptmap[(y+zy)/4][(x+zx)/4] : ptmap[y][x];
      screen[y][x] = statecolors[p.state];
      if(p.state == 0) switch(p.type) {
        case 4: screen[y][x] = 0xFFFFFF; break;
        case 5: screen[y][x] = 0xFF00FF; break;
        }
      part(screen[y][x], 2) = itc(isize(p.eqs));
      if(find_equation(pt.eqs, p)) part(screen[y][x], 2) = 0x80;
      if(p.type == 2) part(screen[y][x], 0) |= 0x20;
      if(p.type == 3) part(screen[y][x], 0) |= 0x40;
      }
    screen.draw();
    
    SDL_Event event;
    SDL_Delay(1);
    int ev;
    while(ev = SDL_PollEvent(&event)) switch (event.type) {
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
        int uni = event.key.keysym.unicode;

        if(key == 'p') paused = !paused;
        if(key == 'z') zx = mousex*3, zy = mousey*3, zoomed = !zoomed;
        
        break;
        }
      
      }
  
  } while(paused);
  
  }

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

bool inner(int t) { return t > 0 && t < 4; }

void computemap(pointmap& ptmap) {

  for(int i=0; i<2; i++) {
    printf("Building eqs, i=%d\n", i);
    
    for(int y=0; y<SY; y++) 
    for(int x=0; x<SX; x++) {
      auto& p = ptmap[y][x];
      p.state = 0;
      if(!inner(p.type)) continue;
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
          if(t < 4 || t == 6 || t == 7) xp++;
          }
        for(auto np: nei) {
          auto& p2 = ptmap[np];
          if(p2.type == 6) p.bonus += 0;
          else if(p2.type == 7) p.bonus += 1./xp;
          else if(p2.type < 4) {
            p.eqs.emplace_back(&p2, 1./xp);
            if(p.type == 2 && p2.type == 3) p.bonus += 1./xp;
            if(p.type == 3 && p2.type == 2) p.bonus -= 1./xp;
            }
          }
        }
      if(i == 1) {
        for(auto np: nei) {
          auto& p2 = ptmap[np];
          if(p2.type == 4) p.bonus += 0; 
          else if(p2.type == 5) p.bonus += 1./4; 
          else if(p2.type == 6 || p2.type == 7) p.bonus += 1./8;
          else {
            p.eqs.emplace_back(&p2, 1./4);
            }
          }
        }
      sort(p.eqs.begin(), p.eqs.end());
      }
    
    vector<ipoint> allpoints;
    for(int y=0; y<SY; y++) 
    for(int x=0; x<SX; x++) if(ptmap[y][x].state == 1) allpoints.push_back({x, y});
    sort(allpoints.begin(), allpoints.end(), [&ptmap] (auto p1, auto p2) { return ptmap[p1].pointorder < ptmap[p2].pointorder; });
    
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
          if(text_progress) printf("  %d/1000 [%d]\n", cpct, size(p.eqs));
          int nextt = SDL_GetTicks();
          if(nextt > lastt + 100) {
            drawstates(ptmap);
            lastt = SDL_GetTicks();
            }
          }
        }
      citer++;
      
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
    
    printf("Solution retrieval\n");
    reverse(allpoints.begin(), allpoints.end());
    for(auto co: allpoints) {
      auto &p = ptmap[co];
      if(p.state != 2) continue;
      p.x[i] = p.bonus;
      for(auto& pa: p.eqs) p.x[i] += pa.second * pa.first->x[i];
      p.eqs = vector<equation> ();
      }
    
    printf("Done.\n");        
    }
  }

void savemap(const string& fname) {
  FILE *f = fopen(fname.c_str(), "wb");
  if(!f) pdie("savemap");
  fwrite(&SX, sizeof(SX), 1, f);
  fwrite(&SY, sizeof(SY), 1, f);
  for(int y=0; y<SY; y++)
  for(int x=0; x<SX; x++) {
    auto& p = pts[y][x];
    fwrite(&p.x, sizeof(p.x), 1, f);
    fwrite(&p.type, sizeof(p.type), 1, f);
    }
  fclose(f);
  }

void merge_sides() {
  if(current_side > 0) {
    for(int y=0; y<SY; y++) 
    for(int x=0; x<SX; x++) {
      auto xy = ipoint(x, y);
      auto& p = pts[xy];
      if(p.type == 0 || p.type >= 4)
        p.type = p.baktype;
      }
    }
  }

void loadmap(const string& fname) {
  auto& side = single_side(0);

  FILE *f = fopen(fname.c_str(), "rb");
  if(!f) pdie("loadmap");
  fread(&SX, sizeof(SX), 1, f);
  fread(&SY, sizeof(SY), 1, f);
  resize_pt();
  for(int y=0; y<SY; y++)
  for(int x=0; x<SX; x++) {
    auto& p = pts[y][x];
    fread(&p.x, sizeof(p.x), 1, f);
    fread(&p.type, sizeof(p.type), 1, f);
    if(p.type == 2) side.type = 1;
    p.side = 0;
    }
  fclose(f);
  }

void loadmap2(const string& fname) {
  auto& side = new_side(0);
  FILE *f = fopen(fname.c_str(), "rb");
  if(!f) pdie("loadmap2");
  int iSX, iSY;
  fread(&iSX, sizeof(iSX), 1, f);
  fread(&iSY, sizeof(iSY), 1, f);
  if(iSX != SX || iSY != SY) die("map size mismatch\n");
  for(int y=0; y<SY; y++)
  for(int x=0; x<SX; x++) {
    datapoint dp; 
    fread(&dp.x, sizeof(dp.x), 1, f);
    fread(&dp.type, sizeof(dp.type), 1, f);
    if(dp.type < 4 && dp.type > 0) {
      dp.side = side.id;
      pts[y][x] = dp;
      if(dp.type == 2) side.type = 1;
      }
    }
  fclose(f);
  current_side = side.id; 
  }

void loadmap_join(const string& fname, ipoint xy) {
  FILE *f = fopen(fname.c_str(), "rb");
  if(!f) pdie("loadmap_join");
  
  auto& side = new_side(0);
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
    fread(&p.x, sizeof(p.x), 1, f);
    fread(&p.type, sizeof(p.type), 1, f);
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
int boundary_color = 0xFFFFFF;

int bbnd = 100;

ld intdif(ld z) {
  z = z - floor(z);
  if(z > .5) z = 1-z;
  return z;
  }

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
    if(p.type != 1) continue;
    auto y = diskpoint(cx, cy);
    if(isnan(y[0]) || isnan(y[1]) || hypot(y[0], y[1]) > .9) continue;
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
    if(p.type != 1) continue;
    auto y = diskpoint(cx, cy);
    if(isnan(y[0]) || isnan(y[1]) || hypot(y[0], y[1]) > .99999) continue;
    cpoint x = { ld(cx), ld(cy) };
    for(int i=0; i<2; i++) x[i] = x[i] * am[1][i] + am[0][i];
    cpoint dif = x-y;
    ld err = sqrt(dif|dif);
    if(err > max_error)
      max_error = err;
    }
  
  printf("max error = %lf (%lf pixels)\n", double(max_error), double(max_error * SX / 2));  
  }

void draw(bitmap &b) {
  construct_btd();
  b.belocked();

  if(spiral_mode)
    draw_spiral(b, SDL_GetTicks() / 5000.);
  else

  if(triangle_mode)
    draw_triangle(b);
  else

  for(int y=0; y<SY; y++)
  for(int x=0; x<SX; x++) {
    auto& p = pts[y][x];

    if(p.type == 0) {
    
      b[y][x] = notypeside;

      if(lined_out)
      for(int ax=-1; ax<=1; ax++)
      for(int ay=-1; ay<=1; ay++) {
        int x1 = x + ax * lined_out;
        int y1 = y + ay * lined_out;
        if(x1 >= 0 && y1 >= 0 && x1 < SX && y1 < SY && pts[y1][x1].type)
          b[y][x] = 0;
        }

      continue;
      }
    
    int siid = p.side;
    int tsiid = siid;
    if(siid >= size(sides)) continue;
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
    else if(no_images || !si.img.s) {
      int qsides = size(sides);
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
        
        if(abs(y) < .1 || intdif(xx) < .05 / coshy || intdif(y) < .1 && intdif(xx) < .25 / coshy)
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
        part(pix, 2) = 128 + 100 * hypot(pt[0] - bycoord[0], pt[1] - bycoord[1])  / max_error;
        
        if(x == mousex && y == mousey)
          printf("is = %lf %lf should be = %lf %lf\n", double(pt[0]), double(pt[1]), double(bycoord[0]), double(bycoord[1]));
        }
      else {
        part(pix, 0) = int(255 & int(255 * pt.x[0]));
        part(pix, 1) = int(255 & int(255 * pt.x[1]));
        }
      }
    else {
      ld yval = 0;
      auto dc = band_to_disk(x, y, si, tsiid, yval);
      if(yval >= bbnd || yval <= -bbnd)
        b[y][x] = boundary_color;
      else
        b[y][x] = si.img[dc[1]][dc[0]];
      }
    
    if(mark_sides) {
      part(b[y][x], 2) = part(b[y][x], 2) * 14 / 16 + tsiid * 32 / isize(sides);
      }
    }

  if(mark_sides)
    for(auto& si: sides) b[si.join] = rand();
  b.draw();
  }

ld anim_speed;

bool break_loop = false;

void klawisze() {
  SDL_Event event;
  SDL_Delay(1);
  int ev;
  while(ev = SDL_PollEvent(&event)) switch (event.type) {
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
      int uni = event.key.keysym.unicode;
      
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
      
      if(key == 'i') {
        auto& p = pts[mousey][mousex];
        printf("this point (%d,%d) is %lf %lf\n", mousex, mousey, double(p.x[0]), double(p.x[1]));
        }
      
      if(key == 'u') use_childsides = !use_childsides;
      
      if(key == 'm') mark_sides = !mark_sides;
      if(key == 'c') no_images = !no_images;
      if(key == 'q') 
        break_loop = true;
      
      break;
      }
    
    }
  }

void load_image(const string& fname) {
  csideroot().img = readPng(fname); 
  }

void load_image_band(const string& fname) {
  csideroot().img_band.push_back(readPng(fname));
  }

bool need_measure = true;

void measure(sideinfo& si) {
  
  if(si.type == 3) return;
  int sii = si.id;
  auto& gpts = *si.submap;  

  vector<ld> cscs[2];
  
  printf("side #%d (type %d), x %p\n", si.id, si.type, &gpts);
  
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

  if(si.type && si.period > 0) {
    printf("period multiple: %Lf\n", M_PI / si.cscale[0] / si.period);
  
    if(si.type == 2) {
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

void ui() {
  measure_if_needed();
  initGraph(SX, SY, "conformist", false);
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

void export_image(const string& fname) {
  measure_if_needed();
  bitmap b = emptyBitmap(SX, SY);
  draw(b);
  for(int y=0; y<SY; y++)
  for(int x=0; x<SX; x++)
    b[y][x] |= 0xFF000000;
  writePng(fname.c_str(), b);
  }

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

#include "automapper.cpp"

int main(int argc, char **argv) {
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
    else if(s == "-mim") load_image_for_mapping(next_arg());
    else if(s == "-cbo") {
      createb_outer(next_arg_ipoint());
      }
    else if(s == "-trim") {
      int x1 = atoi(next_arg());
      int y1 = atoi(next_arg());
      int x2 = atoi(next_arg());
      int y2 = atoi(next_arg());
      trim(x1, y1, x2, y2);
      }
    else if(s == "-cbi") {
      ipoint axy = next_arg_ipoint();
      ipoint bxy = next_arg_ipoint();
      createb_inner(axy, bxy);
      }
    else if(s == "-mapat") {
      auto_map_at(addmargin(next_arg_ipoint()));
      }
    else if(s == "-mapall") {
      auto_map_all();
      }
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
    else if(s == "-zebra") csideroot().period_unit = zebra_period, csideroot().period_matrices = zebra_matrices;
    else if(s == "-period") csideroot().period = csideroot().period_unit * atoi(next_arg());
    else if(s == "-fix") csideroot().type = 2;
    else if(s == "-ash") csideroot().animshift += atof(next_arg());
    else if(s == "-draw") ui();
    else if(s == "-export") export_image(next_arg());
    else if(s == "-spinspeed") spinspeed = atof(next_arg());
    else if(s == "-marksides") mark_sides = true;
    else if(s == "-bandlen") {
      auto& si = csideroot();
      if(si.img_band.empty()) die("no bands to measure in -bandlen");
      int totalx = 0;
      for(auto& bandimg: si.img_band) totalx += bandimg.s->w;
      int y = si.img_band[0].s->h;
      printf("x = %d y = %d\n", totalx, y);
      printf("To make a loop, speed times count should be %lf\n", totalx * 1. / y);
      }
    else if(s == "-exportv") {
      ld speed = atof(next_arg());
      int cnt = atoi(next_arg());
      export_video(speed, cnt, next_arg());
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
    else if(s == "-spiral") {
      single_side(0);
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
    else die("unrecognized argument: " + s);
    }

  return 0;
  }
