// to make the Bad Apple video
// the orig video: https://www.youtube.com/watch?v=P9EZy8ZzlVU
// making of: https://www.youtube.com/watch?v=NGixk6jVVNM

// to render all segments in parallel: (you need ../bad-apple.mkv of size 480x360, 6572 frames)
// for s in {0..17}
// do ./nconf -badapple-flags 4 -badapple-seg $s &
// done
// for s in {0..17}
// do wait
// done

#include <sys/wait.h>
#include <set>

namespace nconf {

int badapple_flags;

int read_all(int fd, void *buf, int cnt) {
  char *cbuf = (char*) buf;
  while(cnt > 0) {
    int qt = read(fd, cbuf, cnt);
    if(qt <= 0) break;
    cbuf += qt;
    cnt -= qt;
    }
  return cnt;
  }

const int orig_sx = 480;
const int orig_sy = 360;

double badapple_scale = 1;

// windy = black
bitmap bblack, bwhite, bout, bblack1;

vector2<cpoint> last_points, next_points;
vector2<int> map_id;

auto& get_map(ipoint p) { return map_id[p]; }

vector2<color> ipixels, opixels;

vector<int> which_side;

color& ipixels_scaled(int x, int y) {
  return ipixels[(y+.5) / badapple_scale][(x+.5) / badapple_scale];
  }

int side(int x, int y) {
  return part(ipixels_scaled(x, y), 1) < 128 ? 0 : 1;
  };

int sideno;
int psideno;

void remove_stars(vector<ipoint>& pixels, int s) {
  for(int y=4; y<SY-4; y++)
  for(int x=4; x<SX-4; x++) {
    ipoint p{x, y};
    if(side(p.x, p.y) != s) {
      for(int a=-4; a<4; a++) {
        if(side(p.x-a, p.y-4) != s) goto next;
        if(side(p.x-4, p.y+a) != s) goto next;
        if(side(p.x+4, p.y+4) != s) goto next;
        if(side(p.x+4, p.y-a) != s) goto next;
        }
      }
    pixels.push_back(p); get_map(p) = sideno;
    next: ;
    }
  }

vector<ipoint> compute_pixels(int x, int y, int s) {
  printf("point %d,%d has side %d while recorded %d:%d, sideno = %d/%d\n", x, y, s, map_id[y][x], which_side[map_id[y][x]], sideno, psideno);

  
  vector<ipoint> pixels;

  auto add = [&] (ipoint p) {
    if(side(p.x, p.y) != s) return;
    if(p.x < 2 || p.x >= SX-2) return;
    if(p.y < 2 || p.y >= SY-2) return;
    if(get_map(p) != psideno) return;
    pixels.push_back(p);
    get_map(p) = sideno;
    };

  add(ipoint{x, y});
  
  for(int j=0; j<isize(pixels); j++) 
  for(int d=0; d<4; d++)
    add(pixels[j] + dv[d]);
  
  printf("number of points: %d starting from: %d,%d\n", int(pixels.size()), x, y);
  
  // remove_stars(pixels, s);
  
  return pixels;
  }

void extend_outside(vector<ipoint>& pixels) { return;
  vector2<char> a_outside;
  a_outside.resize2(SX, SY);
  
  for(int y=0; y<SY; y++)
  for(int x=0; x<SX; x++) a_outside[y][x] = false;
  
  vector<ipoint> apixels;
  
  auto add1 = [&] (ipoint p) {
    if(p.x < 0 || p.x >= SX) return;
    if(p.y < 0 || p.y >= SY) return;
    if(get_map(p) == sideno) return;
    if(a_outside[p]) return;
    apixels.push_back(p);
    a_outside[p] = true;
    };

  for(int y=0; y<SY; y++) add1({0, y}), add1({SX-1, y});
  for(int x=0; x<SX; x++) add1({x, 0}), add1({x, SY-1});

  for(int j=0; j<isize(apixels); j++) for(int d=0; d<4; d++) add1(apixels[j] + dv[d]);
  
  for(int y=0; y<SY; y++) for(int x=0; x<SX; x++) if(!a_outside[y][x]) if(map_id[y][x] != sideno) {
    pixels.push_back(ipoint{x, y});
    map_id[y][x] = sideno;
    }
  
  printf("number of points now: %d (apixels = %d)\n", int(pixels.size()), int(apixels.size()));
  }

bool prepare_for_mapping(const vector<ipoint> pixels) {
   sides.clear();
   create_side(stype::standard);
   for(int y=0; y<SY; y++) for(int x=0; x<SX; x++) pts[y][x].type = ptype::outside;

  for(auto p: pixels)
    pts[p].type = ptype::inside;

  auto ab = all_boundary(pts);
  
  int end1 = 0, end2 = 0;
  ld bestdist = 0;
  for(int i=0; i<int(ab.size()); i++)
  for(int j=0; j<i; j++) {
    ld dist = hypot(std::get<0>(ab[i]) - std::get<0>(ab[j]));
    if(dist > bestdist) bestdist = dist, end1 = i, end2 = j;
    }
  
  if(ab.empty()) {
    printf("ab is empty\n");
    return false;
    }
  
  auto [axy1, ad] = ab[end1];
  auto [bxy1, bd] = ab[end2];
  
  split_boundary(pts, axy1, bxy1, bd^2);
  return true;
  }

vector<ipoint> choose_best(const vector<ipoint> origpixels) {
  auto& si = cside();
  printf("choosing best points\n");
  auto bad_of = [&] (cpoint p) {
    p[1] *= 2; p[1] -= 1;
    return p[1] * p[1] + pow((p[0] - si.xcenter) / si.cscale[0], 2) / 2;
    };
  
  vector<ipoint> mpixels;
  for(auto p: origpixels) if(p.x % 8 == 0 && p.y % 8 == 0) mpixels.push_back(p);
  
  sort(mpixels.begin(), mpixels.end(), [&] (ipoint px, ipoint py) {
    ld bad_x = bad_of(pts[px].x);
    ld bad_y = bad_of(pts[py].x);
    return bad_x < bad_y;
    });

  mpixels.resize((mpixels.size() + 2)/ 3);
  
  return mpixels;
  }

void draw_shift(const vector<ipoint> pixels, const vector<ipoint> mpixels, int iter) {
  auto& si = cside();
  if(!screen.s) return;
  string tit = title + " iteration " + std::to_string(iter);
  SDL_WM_SetCaption(tit.c_str(), 0);

  for(int y=0; y<SY; y++) for(int x=0; x<SX; x++) {
    cpoint pt = last_points[y][x];
    cpoint pm = (cpoint{1, 1} + pt) * (si.img.s->h / 2);
    screen[y][x] = 0xFF000000;
    part(screen[y][x], 0) = part(bblack1[pm[1]][pm[0]], 0) / 2;
    part(screen[y][x], 1) = part(bblack1[pm[1]][pm[0]], 1) / 2;
    }
    
  for(auto pi: pixels) {
    ld xval = 0, yval = 0;
    int siid = 0;
    auto dc = band_to_disk(pi.x, pi.y, si, siid, xval, yval, false);
    cpoint pm = (cpoint{1, 1} + dc) * (si.img.s->h / 2);
    auto pix = bblack1[pm[1]][pm[0]];
    part(screen[pi], 2) = part(pix, 2);
    }

  for(auto pi: mpixels) {
    ld xval = 0, yval = 0;
    int siid = 0;
    auto dc = band_to_disk(pi.x, pi.y, si, siid, xval, yval, false);
    auto dch = disk_to_hyper(dc);
    ld bestdist = 0.2;
    auto mem = disk_to_hyper(last_points[pi.y][pi.x]);

    ld dist = dch[2] * mem[2] - dch[0] * mem[0] - dch[1] * mem[1] - 1;
    if(dist < bestdist) bestdist = dist;
    
    screen[pi.y][pi.x] = 0xFF00FF00 + 0xFF01 * int(dist * 255 * 5);
    screen[pi.y+1][pi.x] = 0xFF00FF00 + 0xFF01 * int(dist * 255 * 5);
    screen[pi.y-1][pi.x] = 0xFF00FF00 + 0xFF01 * int(dist * 255 * 5);
    screen[pi.y][pi.x+1] = 0xFF00FF00 + 0xFF01 * int(dist * 255 * 5);
    screen[pi.y][pi.x-1] = 0xFF00FF00 + 0xFF01 * int(dist * 255 * 5);
    }
  
  screen.draw();
  if(states_to_video) {
    for(int y=0; y<SY; y++)
      write(video_out, &screen[y][0], 4 * SX);
    }
  }

shift_data choose_best_shift(const vector<ipoint> pixels, const vector<ipoint> mpixels, ld per) {
  auto& si = cside();
  shift_data best_shift;
  ld best_dist = 1e8;
  
  auto eps = [&] () { return (rand() % 1000 - 499.5) / 1000; };
  
  best_shift.px = best_shift.py = best_shift.ps = 0;

  if(badapple_flags & 1) return best_shift;  
  
  int iter = 20000;
  int iter1 = 12000;
  
  for(int att=0; att<iter; att++) {
    cshift = best_shift;
    ld dv = 1;
    if(att > iter1) dv = exp((att - iter1) * 6. / (iter - iter1));
    if(att < iter1) cshift.px = cshift.py = cshift.ps = 0;
    if(att < iter1 || (att & 1)) {
      cshift.px += per * eps() / dv;
      cshift.py += per * eps() / dv;
      }
    cshift.ps += M_PI * eps() / dv;
    ld total_dist = 0;
    for(auto pi: mpixels) {
      ld xval = 0, yval = 0;
      int siid = 0;
      auto dc = band_to_disk(pi.x, pi.y, si, siid, xval, yval, false);
      auto dch = disk_to_hyper(dc);
      ld bestdist = 1;
      auto mem = disk_to_hyper(last_points[pi.y][pi.x]);

      ld dist = dch[2] * mem[2] - dch[0] * mem[0] - dch[1] * mem[1] - 1;
      if(dist < bestdist) bestdist = dist;
      // printf("** %Lf,%Lf,%Lf vs %Lf,%Lf,%Lf = %Lf [%Lf,%Lf]\n", dch[0], dch[1], dch[2], mem[0], mem[1], mem[2], dist, dc[0], dc[1]);
      
      if(0) for(auto per: si.period_matrices) {
        auto mem1 = mul(per, mem);
        ld dist = dch[2] * mem1[2] - dch[0] * mem1[0] - dch[1] * mem1[1] - 1;
        // if(qty < 10 && qty1 < 3) printf("   %Lf,%Lf,%Lf vs %Lf,%Lf,%Lf = %Lf\n", dch[0], dch[1], dch[2], mem1[0], mem1[1], mem1[2], dist);
        if(dist < bestdist) bestdist = dist;
        }
      total_dist += bestdist;
      }

    if(badapple_flags & 2) draw_shift(pixels, mpixels, att);

    if(total_dist < best_dist) {
      printf("#%3d : for %Lf,%Lf,%Lf obtained %Lf/%d\n", att, cshift.px, cshift.py, cshift.ps, total_dist, int(mpixels.size()));
      best_dist = total_dist; best_shift = cshift;
      
      if(badapple_flags & 4) draw_shift(pixels, mpixels, att);
      }

    }

  printf("WON: %Lf,%Lf,%Lf\n", best_shift.px, best_shift.py, best_shift.ps);
  return best_shift;
  }

int next_region;

void process_after_split(const vector<ipoint> origpixels, int s) {
  auto pixels = origpixels;

  string t = title;
  next_region++;
  title = title + " region "  + std::to_string(next_region);

  extend_outside(pixels);
  if(!prepare_for_mapping(pixels)) { title = t; return; }
  computemap(pts);
  
  if(badapple_scale <= .5) {
    for(int y=0; y<SY; y++) {
      printf("\"");
      for(int x=0; x<SX; x++)
        printf("%d", int(pts[y][x].type));
      printf("\",\n");
      }
    }

  printf("measuring\n");
  bool mea = measure(cside());
  if(!mea) {
    printf("failed\n");
    for(auto pi: pixels) 
      opixels[pi.y][pi.x] = ipixels_scaled(pi.x, pi.y);
    title = t;
    return;
    }
  
  auto& pattern = s ? bwhite : bblack;        
  auto& si = cside();
  si.img = std::move(pattern);
  ld per = si.period_unit * 3;
  // si.period = si.period_unit * (s ? 6 : 9);
  si.period_unit = zebra_period, si.period_matrices = zebra_matrices;
          
  auto mpixels = choose_best(origpixels);
  
  use_shift = true;
  cshift = choose_best_shift(origpixels, mpixels, per);

  printf("drawing\n");
  for(auto xy: pixels) {
    if(pts[xy].type == ptype::outside) continue;
    if(pts[xy+ipoint{1,0}].type == ptype::outside) continue;
    if(pts[xy+ipoint{0,1}].type == ptype::outside) continue;
    if(pts[xy+ipoint{1,1}].type == ptype::outside) continue;

    int px = xy.x, py = xy.y;
    
    int qty = 4;
    
    int ps[4];
    for(int p=0; p<4; p++) ps[p] = 0;
    
    auto rec = pts[xy].x;

    for(int a=0; a<qty; a++)
    for(int b=0; b<qty; b++) {
      cpoint c = (pts[py][px].x * (qty-a) * (qty-b) + 
                  pts[py][px+1].x * a * (qty-b) + 
                  pts[py+1][px].x * (qty-a) * b +
                  pts[py+1][px+1].x * a * b) / qty / qty;
      pts[xy].x = c;
      ld xval = 0, yval = 0;
      int tsiid = 0;
      auto dc = band_to_disk(px, py, si, tsiid, xval, yval);
      color co = si.img[dc[1]][dc[0]];
      for(int p=0; p<4; p++) ps[p] += part(co, p);
      }
    
    pts[xy].x = rec;

    for(int p=0; p<4; p++) part(bout[py][px], p) = (ps[p] + qty*qty/2) / qty / qty;
    }
  
  // draw(bout);
  pattern = std::move(si.img);
  
  for(auto pi: pixels) {
    ld xval = 0, yval = 0;
    int siid = 0;
    auto dc = band_to_disk(pi.x, pi.y, si, siid, xval, yval, false);
    next_points[pi.y][pi.x] = dc;
    }

  for(auto pi: pixels) opixels[pi.y][pi.x] = bout[pi.y][pi.x];
  
  if(0) for(auto pi: mpixels) opixels[pi.y][pi.x] = 0xFFFF0000;
  
  if(0) for(auto pi: pixels) if(pi.x % 2 == 0 && pi.y % 2 == 0) {
    ld xval = 0, yval = 0;
    int siid = 0;
    auto dc = band_to_disk(pi.x, pi.y, si, siid, xval, yval, false);
    auto dch = disk_to_hyper(dc);
    ld bestdist = 0.2;
    auto mem = disk_to_hyper(last_points[pi.y][pi.x]);

    ld dist = dch[2] * mem[2] - dch[0] * mem[0] - dch[1] * mem[1] - 1;
    if(dist < bestdist) bestdist = dist;
    
    opixels[pi.y][pi.x] = 0xFF00FF00 + 0xFF00 * int(dist * 255 * 5);
    }
  
  int cnos = 0, nsno = 0;
  for(auto pix: pixels) {
    if(side(pix.x, pix.y) != s) cnos++;
    if(get_map(pix) != sideno) nsno++;
    }
  printf("errors: %d,%d\n", cnos, nsno);
  int qty = 0;
  for(auto pi: pixels) if(qty++<10) printf("%d,%d:%d,%d ", pi.x, pi.y, get_map(pi), side(pi.x, pi.y));
  printf("\n");
  
  title = t;
  }

void remove_line(ipoint a, ipoint b, int s) {
  printf("removing line from %d,%d to %d,%d\n", a.x, a.y, b.x, b.y);
  which_side[sideno] = !s;
  int csideno = which_side.size();
  which_side.push_back(s);
  for(int it=0; it<=1000; it++) for(int ay=-1; ay<=1; ay++) for(int ax=-1; ax<=1; ax++) {
    int y = a.y + (b.y - a.y) * it / 1000 + ay;
    int x = a.x + (b.x - a.x) * it / 1000 + ax;
    if(map_id[y][x] == sideno)
      map_id[y][x] = csideno;
    }
  }

void split_and_process(const vector<ipoint> origpixels, int s) {

  tuple<ipoint, int> start_bound;
  for(int y=1; y<SY-1; y++) for(int x=0; x<SX-1; x++) {
    ipoint xy(x, y);
    for(int d=0; d<4; d++)
      if(get_map(xy) == sideno && get_map(xy + dv[d]) != sideno) {
        start_bound = {xy, d};
        goto next;
        }
    }
  
  next:
  vector<pair<ipoint, int>> outer;
  
  auto [sbxy, sd] = start_bound;
  auto [bxy, d] = start_bound;

  for(int iter=0; iter<100000000; iter++) {
    d &= 3;
    outer.emplace_back(bxy, d);
    auto cxy = bxy + dv[d];
    if(get_map(cxy) == sideno) { bxy = cxy; d--; }
    else d++;
    if(bxy == sbxy && d == sd) break;
    }
  
  printf("found boundary after %d iterations\n", int(outer.size()));
  
  std::map<pair<ipoint, int>, int> outer_map;
  for(int i=0; i<int(outer.size()); i++) outer_map.emplace(outer[i], i);
  std::set<ipoint> outer_set;
  for(auto m: outer_map) outer_set.insert(std::get<0>(m.first));
  
  vector<ipoint> inner;

  for(int y=1; y<SY-1; y++) for(int x=0; x<SX-1; x++) {
    ipoint xy(x, y);
    for(int d=0; d<4; d++)
      if(get_map(xy) == sideno && get_map(xy + dv[d]) != sideno && !outer_set.count(xy)) {
        inner.push_back(xy);
        }
    }

  printf("outer = %d, inner = %d\n", int(outer_set.size()), int(inner.size()));
  
  if(inner.size()) {
    ipoint cl1, cl2;
    ld bestdist = 1e8;
    for(auto [o, d]: outer) for(auto i: inner) {
      ld dist = hypot(o - i);
      if(dist < bestdist) bestdist = dist, cl1 = i, cl2 = o; 
      }
    remove_line(cl1, cl2, s);
    return;
    }
  
  if(1) {
    ld bestval = 1;
    ipoint cl1, cl2;
    int os = outer.size();
    int ii=0, ij=0;

    for(int i=0; i<os; i++) {
      auto [xy, d] = outer[i];
      auto oxy = xy;
      if(d >= 2) continue;
      d ^= 2;
      int dist = 0;
      while(!outer_map.count({xy, d})) xy += dv[d], dist++;
      int j = outer_map[{xy, d}];
      
      int idist = abs(i - j);
      
      if(idist > os/2) idist = os - idist;

      ld val = 20 * (dist+1) / idist;
      if(val >= bestval) continue;

      bestval = val, cl1 = oxy, cl2 = xy, ii = i, ij = j;
      }
    if(bestval < 1) {
      printf("indices = %d,%d\n", ii, ij);
      remove_line(cl1, cl2, s);
      return;
      }
    }
  
  process_after_split(origpixels, s);
  }

void process_frame() {
  next_region = 0;
  for(int y=0; y<SY; y++) for(int x=0; x<SX; x++) map_id[y][x] = 0; 
  
  for(int y=0; y<SY; y++) for(int x=0; x<SX; x++) 
    opixels[y][x] = ipixels_scaled(x, y);  
  
  which_side = {2};
  
  sideno = 0;
  
  for(int y=2; y<SY-2; y++)
  for(int x=2; x<SX-2; x++)
    if(side(x, y) != which_side[map_id[y][x]]) {
      // -cbi
      sideno = which_side.size();
      
      psideno = map_id[y][x];
      
      int s = side(x, y);
      which_side.push_back(s);

      auto pixels = compute_pixels(x, y, s);

      for(auto pi: pixels) map_id[pi.y][pi.x] = sideno;
      
      if(pixels.size() < 100) {
        for(auto pi: pixels) {
          opixels[pi] = ipixels_scaled(pi.x, pi.y);
          }
        continue;
        }
      
      split_and_process(pixels, s);
      }

  last_points = next_points;
  }

int video_out;

void badapple(int fbase, int froof, const char *outfile) {
  const int frames = 6572;
  SX = orig_sx * badapple_scale; SY = orig_sy * badapple_scale;
  ipixels.resize2(orig_sx, orig_sy);
  opixels.resize2(SX, SY);
  last_points.resize2(SX, SY);
  next_points.resize2(SX, SY);
  map_id.resize2(SX, SY);
  
  resize_pt();
  
  bblack1 = readPng("badapple-black.png");

  bblack = readPng("badapple-black.png");
  bwhite = readPng("badapple-white.png");

  printf("bout initializing..\n");
  bout = emptyBitmap(SX, SY);
  printf("bout initialized\n");
  
  /* actually load */
  array<int, 2> tab;
  if(pipe(&tab[0])) {
    printf("Error: %s", strerror(errno));
    exit(1);
    }

  int pid = fork();
  fflush(stdout);

  fprintf(stderr, "pipe is %d:%d\n", tab[0], tab[1]);

  if(pid == 0) {
    fprintf(stderr, "in child\n");
   fprintf(stderr, "making fformat\n");
    char fformat[200];
    sprintf(fformat, "ffmpeg -y -i ../bad-apple.mkv -f rawvideo -pix_fmt bgra /dev/fd/%d 2>/dev/null", tab[1]);
    int sys = system(fformat);
    ::close(tab[0]);
    fprintf(stderr, "input system call returned %d: %s\n", sys, strerror(errno));
    ::close(tab[1]);
    exit(0);
    }

  array<int, 2> tab_out;
  if(pipe(&tab_out[0])) {
    printf("Error: %s", strerror(errno));
    exit(1);
    }
  
  int pid_out = fork();
  if(pid_out == 0) {
    close(0);
    if(dup(tab_out[0]) != 0) exit(1);
    if(close(tab_out[1]) != 0) exit(1);
    if(close(tab_out[0]) != 0) exit(1);
    char fformat[200];
    sprintf(fformat, "ffmpeg -y -f rawvideo -pix_fmt bgra -s %dx%d -r 30 -i - -pix_fmt yuv420p -codec:v libx264 %s 2>/dev/null", SX, SY, outfile);
    int sys = system(fformat);
    fprintf(stderr, "output system call returned %d: %s\n", sys, strerror(errno));
    exit(0);
    }
  
  close(tab_out[0]);
  
  text_progress = false;

  ::close(tab[1]);
  video_out = tab_out[1];
  
  for(int i=0; i<frames; i++) {
  
    // auto oipixels = ipixels;
    
    for(int y=0; y<orig_sy; y++) read_all(tab[0], &ipixels[y][0], 4 * orig_sx);
    
    // if(i < 150 || i >= 180) continue;
    if(i < fbase || i >= froof) continue;
    // if(i != 571) continue;
    printf("\n\nprocessing frame %d\n", i);
    
    title = "Bad Apple frame " + std::to_string(i);
    
    process_frame();
    
    if(!states_to_video) for(int y=0; y<SY; y++) 
      write(tab_out[1], &opixels[y][0], 4 * SX);
    printf("written\n");

    printf("test export\n");
    for(int y=0; y<SY; y++)
    for(int x=0; x<SX; x++)
      bout[y][x] = opixels[y][x] | 0xFF000000;
    char tformat[100]; sprintf(tformat, "testing%05d.png", i);
    writePng(tformat, bout);
    }
  
  video_out = 0;
  
  ::close(tab[0]);
  printf("waiting for input process to terminate\n");
  wait(nullptr);

  ::close(tab_out[1]);
  printf("waiting for output process to terminate\n");
  wait(nullptr);

  printf("all done\n");
  }

vector<int> fdivides = { 0, 444, 820, 1263, 1686, 1910, 2247, 2501, 2951, 3273, 3672, 3982, 4188, 4593, 5036, 5354, 5946, 6360, 6513, 6572 };

void badapple_segment(int seg) {
  char buf[64];
  sprintf(buf, "bad-apple-nconf2-%02d.mp4", seg);
  badapple(fdivides[seg], fdivides[seg+1], buf);
  }

void badapple_segment_info() {
  for(int seg=0; seg<18; seg++) {
    printf("expected length = %f\n", (fdivides[seg+1] - fdivides[seg]) / 30.);
    char buf[640];
    sprintf(buf, "mp4info bad-apple-nconf-%02d.mp4", seg);
    system(buf);
    }
  }

}
