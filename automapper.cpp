
ipoint get_last_point(pointmap& ptmap, ipoint start) {
  vector<ipoint> q;
  ptmap[start].type = 2; q.push_back(start);
  for(int i=0; i<isize(q); i++) {
    auto xy = q[i];
    for(auto k: dv) if(ptmap[k+xy].type == 1)
      ptmap[k+xy].type = 2, q.push_back(k + xy);
    }
  for(auto pt: q) ptmap[pt].type = 1;
  return q.back();
  }

ld frac(ld x) { return x - floor(x); }

void auto_joins() {
  measure(cside());
  construct_btd_for(cside());
  
  vector<ipoint> last_points;
  
  again:
  
  vector<pair<ipoint, int>> q;
  for(int y=0; y<SY; y++) 
  for(int x=0; x<SX; x++) {
    auto xy = ipoint(x, y);
    auto& p = pts[xy];
    if(p.type != 1 || p.side != current_side) continue;
    
    auto pxy = p.x;

    auto csi = &cside();
    auto sid = current_side;
    parent_changed:

    for(int subid: csi->childsides) {
      auto& nsi = sides[subid];
      auto& epts = *nsi.submap;
      
      if(epts[xy].type != 1) continue;
    
      if(epts[xy].x[0] / nsi.cscale[0] * M_PI > nsi.zero_shift) {
        pxy = epts[xy].x;
        csi = &nsi;
        sid = subid;
        goto parent_changed;
        }
      }
    
    if(pxy[1] > 1e-5 && pxy[1] < 1-1e-5) p.type = 2, q.emplace_back(xy, sid);
    }
  
  int nextd = isize(q);
  int d = 0;
  for(int i=0; i<isize(q); i++) {
    if(i == nextd) d++, nextd = isize(q);
    auto xy = q[i].first;
    for(auto k: dv) if(pts[k+xy].type == 1)
      pts[k+xy].type = 2, q.emplace_back(k + xy, q[i].second);
    }

  printf("d=%d until %d\n", d, nextd);
  
  ipoint ending = q.back().first;
  for(ipoint p: last_points) if(p == ending) {
    printf("error: ending repeats\n");
    return;
    }
  last_points.push_back(ending);
  int parent_side = q.back().second;
  for(auto pt: q) pts[pt.first].type = 1;
  
  if(d >= 5) {
    auto& side = new_side(0);
    auto& parside = sides[parent_side];    
    side.parentid = parent_side;
    side.rootid = parside.rootid;
    parside.childsides.push_back(side.id);
    side.submap = new pointmap;
    // side.join = ?
    auto &ppts = *parside.submap;
    auto &epts = *side.submap;
    epts.resize2(SX, SY);
    
    bool over = ppts[ending].x[1] > .5;
    
    ld error = 1e9;
    
    ld low = 1e9;
    
    ipoint lowpoint;
    
    for(int y=0; y<SY; y++)
    for(int x=0; x<SX; x++) {
      auto xy = ipoint(x, y);
      auto& p = epts[xy];
      auto& pold = ppts[xy];
      p.side = side.id;
      p.type = (pold.type == 1 && intdif(pold.x[0] - ppts[ending].x[0]) < 3 * cside().cscale[0]) ? 1 : 0;
      
      ld err = hypot(pold.x[0] - ppts[ending].x[0], pold.x[1] - (over ? .9 : .1));
      if(err < error) error = err, side.join = xy;
      
      ld nlow = frac(pold.x[0] - ppts[ending].x[0] + .5);
      if(nlow < low && p.type == 1) low = nlow, lowpoint = xy;
      }
    
    printf("side = %d->%d->%d join = %d,%d ending = %d,%d\n", parside.rootid, parent_side, side.id, side.join.x, side.join.y, ending.x, ending.y);
    printf("low = %lf (ending = %lf) at %d,%d\n", double(low), double(ppts[ending].x[0]), lowpoint.x, lowpoint.y);

    auto [axy2, ad2] = boundary_point_near(epts, lowpoint);
    auto [bxy2, bd2] = boundary_point_near(epts, ending);

    printf("axy = %d,%d bxy = %d,%d\n", axy2.x, axy2.y, bxy2.x, bxy2.y);
  
    split_boundary(epts, axy2, bxy2, bd2^2);
    computemap(epts);
    measure(side);
    construct_btd_for(side);
    
    goto again;
    }
  }

void auto_map_at(ipoint at) {
  current_side = new_side(0).id;
  cside().inner_point = at;
  auto atpixel = get_heart(at);

  for(int y=0; y<SY; y++) 
  for(int x=0; x<SX; x++) {
    auto xy = ipoint(x, y);
    auto& p = pts[xy];
    p.baktype = p.type;
    p.type = 0;
    }
  
  vector<ipoint> inpoints;
  pts[at].type = 1;
  inpoints.push_back(at);
  
  auto isin = [atpixel] (ipoint xy) {
    if(xy.x == 0 || xy.y == 0 || xy.x == SX-1 || xy.y == SY-1)
      return false;
    return get_heart(xy) == atpixel;
    };
  
  int chi1 = 0, chi2 = 0;
  
  for(int i=0; i<isize(inpoints); i++) {
    auto xy = inpoints[i];
    for(auto k: dv) {
      auto xy2 = xy + k;
      if(isin(xy2)) {
        chi1++;
        if(pts[xy2].type == 0)
          pts[xy2].type = 1, inpoints.push_back(xy2);
        }
      }
    if(isin(xy + ipoint(1,0)) && isin(xy + ipoint(0,1)) && isin(xy + ipoint(1,1)))
      chi2++;
    }
  
  for(auto xy: inpoints) pts[xy].side = current_side;
  
  int chi0 = isize(inpoints);
  chi1 /= 2;
  
  int euler_characteristics = chi0 - chi1 + chi2;
  int holes = 1 - euler_characteristics;

  printf("chi (%d,%d,%d), number of holes = %d\n", chi0, chi1, chi2, holes);  
  
  if(holes >= 2) {
    cside().type = 3;
    merge_sides();
    printf("after holes: type = %d\n", pts[10][10].type);
    return;
    }
  
  if(holes == 0) {

    ipoint axy = get_last_point(pts, at);
    ipoint bxy = get_last_point(pts, axy);
    printf("axy = %d,%d bxy = %d,%d\n", axy.x, axy.y, bxy.x, bxy.y);
    
    auto [axy1, ad] = boundary_point_near(pts, axy);
    auto [bxy1, bd] = boundary_point_near(pts, bxy);
  
    printf("axy1 = %d,%d bxy1 = %d,%d\n", axy1.x, axy1.y, bxy1.x, bxy1.y);
  
    split_boundary(pts, axy, bxy, bd^2);
    }
  
  if(holes == 1) {
    cside().type = 1;
    inpoints.clear();
    inpoints.emplace_back(0,0);
    pts[0][0].type = 4;
    
    for(int i=0; i<isize(inpoints); i++) {
      auto xy = inpoints[i];
      for(auto k: dv) {
        auto xy2 = xy + k;
        if(xy2.x < 0 || xy2.y < 0 || xy2.x >= SX || xy2.y >= SY) continue;
        if(isin(xy2)) continue;
        if(pts[xy2].type == 0)
          pts[xy2].type = 4, inpoints.push_back(xy2);
        }
      }
    
    for(int y=0; y<SY; y++)
    for(int x=0; x<SX; x++)
      if(pts[y][x].type == 0)
        pts[y][x].type = 5;

    ipoint splitat;
    for(int y=0; y<SY; y++)
    for(int x=0; x<SX; x++)
      if(pts[y][x].type == 5 && pts[y][x-1].type == 1) {
        splitat = ipoint(x, y);
        break;
        }

    for(int y=0; y<SY; y++)
    for(int x=0; x<=splitat.x; x++)
      if(pts[y][x].type == 1) {
         if(y < splitat.y)
           pts[y][x].type = 2;
         else
           pts[y][x].type = 3;
        }
    }

  computemap(pts);
  
  auto_joins();
  
  merge_sides();
  }
  
void drawsides() {
  paused = true;
  do {
    initGraph(SX, SY, "conformist", false);
  
    for(int y=0; y<SY; y++)
    for(int x=0; x<SX; x++) {
      screen[y][x] = // ((x+y)&1) ? pts[y][x].side * 0x147931 : 
        pts[y][x].type ? 0xFFFFFF : 0;
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
        
        break;
        }
      
      }
    }
  while(paused);  
  }

void auto_map_all() {
  for(int y=0; y<SY; y++)
  for(int x=0; x<SX; x++)
    pts[y][x].type = 0;

  for(int y=2; y<SY-2; y++)
  for(int x=2; x<SX-2; x++)
    if(pts[y][x].type == 0 && get_heart({x,y}) == get_heart({x+1, y}) && pts[y][x+1].type == 0) {
      printf("Mapping (%d,%d)\n", x, y);
      auto_map_at(ipoint(x, y));  
      }
  }

template<class T> void save(FILE *f, const T& x) {
  fwrite(&x, sizeof(T), 1, f);
  }

template<class T> void load(FILE *f, T& x) {
  fread(&x, sizeof(T), 1, f);
  }

void save_all_maps(const string fname) {
  FILE *f = fopen(fname.c_str(), "wb");
  if(!f) pdie("save_all_maps");
  save(f, SX);
  save(f, SY);  
  int nsides = isize(sides);
  save(f, nsides);
  
  for(int y=0; y<SY; y++)
  for(int x=0; x<SX; x++) {
    auto& p = pts[y][x];
    save(f, p.type);
    save(f, p.side);
    if(p.type > 0 && p.type < 4)
      save(f, p.x);
    }
  
  for(auto& p: sides) {
    save(f, p.inner_point);
    save(f, p.type);
    save(f, p.join);
    save(f, p.parentid);
    if(p.parentid != p.id) {
      auto& epts = *p.submap;
      for(int y=0; y<SY; y++)
      for(int x=0; x<SX; x++) {
        auto& p = epts[y][x];
        save(f, p.type);
        save(f, p.side);
        if(p.type > 0 && p.type < 4)
          save(f, p.x);
        }
      }
    }
  fclose(f);
  }

void load_all_maps(const string fname) {
  FILE *f = fopen(fname.c_str(), "rb");
  if(!f) pdie("save_all_maps");
  load(f, SX);
  load(f, SY);  
  resize_pt();
  int nsides;
  load(f, nsides);
  sides.clear();
  for(int i=0; i<nsides; i++) new_side(0);
  
  for(int y=0; y<SY; y++)
  for(int x=0; x<SX; x++) {
    auto& p = pts[y][x];
    load(f, p.type);
    load(f, p.side);
    if(p.type > 0 && p.type < 4)
      load(f, p.x);
    }
  
  for(auto& p: sides) {
    load(f, p.inner_point);
    load(f, p.type);
    load(f, p.join);
    load(f, p.parentid);
    string sidetypenames[4] = { "standard", "ring", "fixed ring", "illegal" };
    if(p.parentid == p.id) {
      p.rootid = p.id;
      printf("Side %d: %s at (%d,%d)\n", p.id, sidetypenames[p.type].c_str(), p.inner_point.x, p.inner_point.y);
      }
    else {
      p.rootid = sides[p.parentid].rootid;
      p.submap = new pointmap;
      auto &epts = *p.submap;
      epts.resize2(SX, SY);
      for(int y=0; y<SY; y++)
      for(int x=0; x<SX; x++) {
        auto& p = epts[y][x];
        load(f, p.type);
        load(f, p.side);
        if(p.type > 0 && p.type < 4)
          load(f, p.x);
        }
      int par = p.id;
      while(sides[par].parentid != par) par = sides[par].parentid, printf("  ");
      printf("%d joins at %d,%d\n", p.id, p.join.x, p.join.y);
      }
    }
  fclose(f);
  }
