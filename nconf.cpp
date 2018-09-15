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

using namespace std;

ld spinspeed;

#include "mat.cpp"
#include "zebra.cpp"

int SX, SY;

int dx[4] = {1, 0, -1, 0};
int dy[4] = {0, -1, 0, 1};

void resize_pt();

bool draw_progress = true;

bitmap heart;

struct datapoint {
  cpoint x;
  int type;
  int state;
  int side=0;
  ld bonus;
  unordered_map<datapoint*, ld> eqs;
  };

typedef vector<vector<datapoint>> pointmap;

pointmap pts;

struct sideinfo {
  bitmap img;
  vector<bitmap> img_band;
  cpoint cscale;
  ld xcenter;
  ld period;
  ld period_unit;
  vector<transmatrix> period_matrices;
  int type;
  int id;
  int code;
  vector<int> childsides;
  // for child sides
  pointmap* submap;
  int join_x, join_y, parentid;
  transmatrix matrix;
  ld shift;
  bool need_btd;
  };

vector<sideinfo> sides;

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
  side.code = N;
  side.type = type;
  side.submap = &pts;
  side.period_unit = 1;
  side.parentid = N;
  return side;
  }

sideinfo& single_side(int type) {
  sides.clear();
  auto& side = new_side(type);
  current_side = side.id;
  return side;
  }

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

void split_boundary(int ax, int ay, int bx, int by, int d) {

  pts[ay][ax].type = 6;

  int phase = 5;

  pts[by][bx].type = 7;
  bx -= dx[d], by -= dy[d];
  
  for(int iter=0; iter<100000; iter++) {
    printf("%d %d %d %d\n", bx, by, d, phase);
    d &= 3;
    auto& pt2 = pts[by+dy[d]][bx+dx[d]];
    if(pt2.type == phase+2 || pt2.type == phase) d++;
    else if(pt2.type == 0) { pt2.type = phase; d++; }
    else if(pt2.type == 6 || pt2.type == 7) { phase--; if(phase == 3) break; }
    else if(pt2.type == 1) { by += dy[d]; bx += dx[d]; d--; }
    }
  }

tuple<int, int, int> boundary_point_near(int cx, int cy) {
  ld bestdist = 1e8;
  int ax, ay, ad;
    
  for(int x=1; x<SX-1; x++) for(int y=1; y<SY-1; y++)
  for(int d=0; d<4; d++)
    if(pts[y][x].type == 0 && pts[y+dy[d]][x+dx[d]].type == 1) {
      ld dist = hypot(x-cx, y-cy);
      if(dist < bestdist) bestdist = dist, ax = x, ay = y, ad = d;
      }
  
  return make_tuple(ax, ay, ad);
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
  set_SXY(heart);
  }

unsigned& get_heart(int x, int y) {
  return heart[(y-marginy)*scaley][(x-marginx)*scalex];
  }

void createb_outer(int cx, int cy) {
  single_side(1);
  
  auto inpixel = get_heart(cx, cy);

  queue<pair<int, int> > boundary;
  
  while(cx < SX && get_heart(cx, cy) == inpixel) cx++;
  if(cx == SX) die("nothing on the line");
  
  for(int y=0; y<SY; y++) 
  for(int x=0; x<SX; x++) {
    auto& p = pts[y][x];
    p.side = 0;

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
    else if(get_heart(x,y) != inpixel)
      p.type = 5;
    else if(x > cx)
      p.type = 1;
    else if(y < cy)
      p.type = 2;
    else
      p.type = 3;
    }
  
  while(!boundary.empty()) {
    auto [x, y] = boundary.front();
    boundary.pop();
    auto& p = pts[y][x];
    if(p.type != 5) continue;
    p.type = 4;
    boundary.emplace(x+1, y);
    boundary.emplace(x-1, y);
    boundary.emplace(x, y+1);
    boundary.emplace(x, y-1);
    }
  }

void createb_inner(int x1, int y1, int x2, int y2) {
  single_side(0);

  auto inpixel = get_heart(x1, y1);
  printf("%x %x\n", inpixel, get_heart(x2, y2));
  if(get_heart(x2, y2) != inpixel) die("both pixels should be in");
  
  for(int y=0; y<SY; y++) 
  for(int x=0; x<SX; x++) {
    auto& p = pts[y][x];
    p.side = 0;
    if(get_heart(x, y) == inpixel)
      p.type = 1;
    else
      p.type = 0;
    }
  
  auto [ax, ay, ad] = boundary_point_near(x1/scalex+marginx, y1/scaley+marginy);
  auto [bx, by, bd] = boundary_point_near(x2/scalex+marginx, y2/scaley+marginy);
  
  printf("%d %d %d\n", ax, ay, ad);
  printf("%d %d %d\n", bx, by, bd);
  
  split_boundary(ax, ay, bx, by, bd^2);
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
  int wx=0, wy=0;
  auto connection = [&] (int dir) {
    printf("%d %d %d\n", wx, wy, dir);
    for(int dy=(dir==1?-border:border); dy<(dir==3?pix+border:pix-border); dy++)
    for(int dx=(dir==2?-border:border); dx<(dir==0?pix+border:pix-border); dx++)
      pts[dy+wy*pix][dx+wx*pix].type = 1;
    if(dir<4) 
      wx += dx[dir], wy += dy[dir];      
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
  split_boundary(pix/2, border-1, SX-1-pix/2, border-1, 1);
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

#include "triangle.cpp"

int pointorder(pair<int, int> xy) {
  int x = xy.first, y = xy.second;
  if(x==0 && y==0) return -1;
  if(x%2 == 0 || y%2 == 0)
    return pointorder({y, (x&1) ? 1 : (x >> 1)}) + 1;
  else
    return 0;
  }

void drawstates() {
  if(!draw_progress) return;
  initGraph(SX, SY, "conformist", false);
  int statecolors[4] = {
    0, 0xFF0000, 0x00FF00, 0x0000FF };

  for(int y=0; y<SY; y++)
  for(int x=0; x<SX; x++) {
    auto& p = pts[y][x];
    screen[y][x] = statecolors[p.state];
    }
  screen.draw();
  
  }

void computemap() {

  for(int i=0; i<2; i++) {
    printf("Building eqs, i=%d\n", i);

    for(int y=0; y<SY; y++) 
    for(int x=0; x<SX; x++) {
      auto& p = pts[y][x];
      p.state = 0;
      if(p.type >= 4 || p.type == 0) continue;
      p.state = 1;
      p.eqs.clear();
      p.bonus = 0;

      p.x[i] = 0;
      p.bonus = 0;

      if(i == 0) {
        int xp = 0;
        for(int d=0; d<4; d++) {
          auto t = pts[y+dy[d]][x+dx[d]].type;
          if(t < 4 || t == 6 || t == 7) xp++;
          }
        for(int d=0; d<4; d++) {
          auto& p2 = pts[y+dy[d]][x+dx[d]];
          if(p2.type == 6) p.bonus += 0;
          else if(p2.type == 7) p.bonus += 1./xp;
          else if(p2.type < 4) {
            p.eqs[&p2] = 1./xp;
            if(p.type == 2 && p2.type == 3) p.bonus += 1./xp;
            if(p.type == 3 && p2.type == 2) p.bonus -= 1./xp;
            }
          }
        }
      if(i == 1) {
        for(int d=0; d<4; d++) {
          auto& p2 = pts[y+dy[d]][x+dx[d]];
          if(p2.type == 4) p.bonus += 0; 
          else if(p2.type == 5) p.bonus += 1./4; 
          else if(p2.type == 6 || p2.type == 7) p.bonus += 1./8;
          else p.eqs[&p2] = 1./4;
          }
        }
      }
    
    vector<pair<int, int> > allpoints;
    for(int y=0; y<SY; y++) 
    for(int x=0; x<SX; x++) if(pts[y][x].state == 1) allpoints.push_back({x, y});
    sort(allpoints.begin(), allpoints.end(), [] (auto p1, auto p2) { return pointorder(p1) < pointorder(p2); });

    printf("Gaussian elimination\n");
    int lastpct = -1, citer = 0;
    for(auto co: allpoints) {
      auto &p = pts[co.second][co.first];
      if(p.state != 1) continue;
      int cpct = citer * 100 / size(allpoints);
      if(cpct != lastpct) {
        lastpct = cpct;
        printf("  %d%% [%d]\n", cpct, size(p.eqs));
        drawstates();
        }
      citer++;
      if(p.eqs.count(&p)) {
        if(p.eqs[&p] == 1) {
          printf("Variable eliminated at (%d,%d)\n", co.first, co.second);
          p.state = 3;
          }
        else {
          ld fac = 1 / (1 - p.eqs[&p]);
          p.eqs.erase(&p);
          for(auto& pa: p.eqs) pa.second *= fac;
          p.bonus *= fac;
          }
        }
      for(auto& pa: p.eqs) {
        auto& p2 = *pa.first;
        ld mirror = p2.eqs[&p];
        p2.eqs.erase(&p);
        p2.bonus += p.bonus * mirror;
        for(auto& pa: p.eqs) p2.eqs[pa.first] += pa.second * mirror;
        }
      p.state = 2;
      }
    
    printf("Solution retrieval\n");
    reverse(allpoints.begin(), allpoints.end());
    for(auto co: allpoints) {
      auto &p = pts[co.second][co.first];
      if(p.state != 2) continue;
      p.x[i] = p.bonus;
      for(auto& pa: p.eqs) p.x[i] += pa.second * pa.first->x[i];
      }
    
    printf("Done.\n");        
    }
  }

int mousex, mousey;

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
      dp.side = side.code;
      pts[y][x] = dp;
      if(dp.type == 2) side.type = 1;
      }
    }
  fclose(f);
  current_side = side.id; 
  }

void loadmap_join(const string& fname, int x, int y) {
  FILE *f = fopen(fname.c_str(), "rb");
  if(!f) pdie("loadmap_join");
  
  auto& side = new_side(0);
  side.parentid = current_side;
  sides[current_side].childsides.push_back(side.id);
  side.submap = new pointmap;
  side.join_x = x/scalex + marginx;
  side.join_y = y/scaley + marginy;
  side.code = 0;
  auto &epts = *side.submap;
    
  int iSX, iSY;
  fread(&iSX, sizeof(iSX), 1, f);
  fread(&iSY, sizeof(iSY), 1, f);
  if(iSX != SX || iSY != SY) die("map size mismatch\n");

  epts.resize(SY);
  for(int y=0; y<SY; y++) epts[y].resize(SX);

  for(int y=0; y<SY; y++)
  for(int x=0; x<SX; x++) {
    auto& p = epts[y][x];
    fread(&p.x, sizeof(p.x), 1, f);
    fread(&p.type, sizeof(p.type), 1, f);
    p.side = 0;
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

void draw(bitmap &b) {
  construct_btd();
  b.belocked();

  if(triangle_mode)
    draw_triangle(b);
  else

  for(int y=0; y<SY; y++)
  for(int x=0; x<SX; x++) {
    auto& p = pts[y][x];

    if(y == SY/3 && x == SX/8) { b[y][x] = 0xFFD500; continue; }

    if(p.type == 0) {
      b[y][x] = 0xFFD500;
      continue;
      }
    
    int siid = p.side;
    if(siid >= size(sides)) continue;
    auto& si = sides[siid];

    if(si.img_band.size()) {
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
    else if(!si.img.s) {
      int qsides = size(sides);
      b[y][x] = int(255 & int(256 * pts[y][x].x[0])) + ((255 & int(255 * pts[y][x].x[1])) << 8) + (qsides>1 ? (((siid * 255) / (qsides-1)) << 16) : 0);
      }
    else {
      auto dc = band_to_disk(x, y, si);
      b[y][x] = si.img[dc[1]][dc[0]];
      }
    }
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
      
      if(key == 'q') 
        break_loop = true;
      
      break;
      }
    
    }
  }

bool inner(int t) { return t > 0 && t< 4; }

void load_image(const string& fname) {
  sides[current_side].img = readPng(fname); 
  }

void load_image_band(const string& fname) {
  sides[current_side].img_band.push_back(readPng(fname));
  }

bool need_measure = true;

void measure(sideinfo& si) {
  
  int sii = si.code;
  auto& gpts = *si.submap;  

  vector<ld> cscs[2];
  
  printf("side #%d (%d), code %d, %x\n", si.id, si.type, sii, &gpts);
  
  printf("%lf %lf\n", (double)gpts[SY/3][SX/8].x[0], (double)gpts[SY/3][SX/8].x[1]);
  
  for(int y=0; y<SY; y++)
  for(int x=0; x<SX; x++) if(gpts[y][x].side == sii) if(inner(gpts[y][x].type) && gpts[y+1][x].side == sii && inner(gpts[y+1][x].type) && gpts[y][x+1].side == sii && inner(gpts[y][x+1].type)) {
    auto c = get_conformity(x, y, si);
    for(int i: {0,1}) cscs[i].push_back(c[i]);
    }
  
  for(int i: {0,1}) sort(cscs[i].begin(), cscs[i].end());  
  int q = size(cscs[0]);
  
  si.cscale = { cscs[0][q/2], cscs[1][q/2] };
  
  for(int i=0; i<=16; i++) {
    int id = (q * i) / 16;
    printf("%Lf %Lf\n", cscs[0][id], cscs[1][id]);
    }

  printf("conformity: %Lf %Lf (%d points)\n", si.cscale[0], si.cscale[1], q);
  
  vector<ld> xes;
  for(int y=0; y<SY; y++)
  for(int x=0; x<SX; x++) if(gpts[y][x].side == sii) if(inner(gpts[y][x].type)) xes.push_back(gpts[y][x].x[0]);
  sort(xes.begin(), xes.end());
  si.xcenter = xes[size(xes) / 2];
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
    for(auto& si: sides) 
      si.xcenter += si.cscale[0] * anim_speed * (t1-t) / 1000.;
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
    draw(b);
    for(int y=0; y<SY; y++)
    for(int x=0; x<SX; x++)
      b[y][x] |= 0xFF000000;
    char buf[100000];
    snprintf(buf, 100000, fname.c_str(), i);
    writePng(buf, b);
    printf("Saving: %s\n", buf);
    for(auto& si: sides) 
      si.xcenter += si.cscale[0] * spd;
    }
  }

int main(int argc, char **argv) {
  int i = 1;
  auto next_arg = [&] () { if(i == argc) die("not enough arguments"); return argv[i++]; };
  while(i < argc) {
    string s = next_arg();
    if(s == "-scale") scalex = scaley = atoi(next_arg());
    else if(s == "-margin") marginx = marginy = atoi(next_arg());
    else if(s == "-rectangle") {
      SX = atoi(next_arg());
      SY = atoi(next_arg());
      createb_rectangle();
      }
    else if(s == "-mim") load_image_for_mapping(next_arg());
    else if(s == "-cbo") {
      int x = atoi(next_arg());
      int y = atoi(next_arg());
      createb_outer(x, y);
      }
    else if(s == "-cbi") {
      int x1 = atoi(next_arg());
      int y1 = atoi(next_arg());
      int x2 = atoi(next_arg());
      int y2 = atoi(next_arg());
      createb_inner(x1, y1, x2, y2);
      }
    else if(s == "-sb") saveb(next_arg());
    else if(s == "-q") draw_progress = false;
    else if(s == "-cm") computemap();
    else if(s == "-sm") savemap(next_arg());
    else if(s == "-lm") loadmap(next_arg());
    else if(s == "-lm2") loadmap2(next_arg());
    else if(s == "-lmj") {
      string s = next_arg();
      int x = atoi(next_arg());
      int y = atoi(next_arg());
      loadmap_join(s, x, y);
      }
    else if(s == "-back") {
      current_side = sides[current_side].parentid;
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
    else if(s == "-zebra") sides[current_side].period_unit = zebra_period, sides[current_side].period_matrices = zebra_matrices;
    else if(s == "-period") sides[current_side].period = sides[current_side].period_unit * atoi(next_arg());
    else if(s == "-fix") sides[current_side].type = 2;
    else if(s == "-draw") ui();
    else if(s == "-export") export_image(next_arg());
    else if(s == "-spinspeed") spinspeed = atof(next_arg());
    else if(s == "-bandlen") {
      auto& si = sides[current_side];
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
    else if(s == "-hilbert") {
      int lev = atoi(next_arg());
      int pix = atoi(next_arg());
      int border = atoi(next_arg());
      create_hilbert(lev, pix, border);
      }
    else if(s == "-triangle") {
      create_triangle(atoi(next_arg()));
      }
    else if(s == "-tm")
      triangle_mode = true;
    else die("unrecognized argument: " + s);
    }

  return 0;
  }
