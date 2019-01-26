
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

void auto_map(int inout) {
  auto outpixel = get_heart(ipoint{0,0});
  
  if(inout == 0) {  
    current_side = new_side(inout).id;
    ipoint start;
  
    for(int y=0; y<SY; y++) 
    for(int x=0; x<SX; x++) {
      auto xy = ipoint(x, y);
      auto& p = pts[xy];
      
      p.baktype = p.type;
  
      if(get_heart(xy) != outpixel && x && y && x < SX-1 && y < SY-1)
        p.type = 1, start = xy, p.side = current_side;
      else
        p.type = 0;
      }
    
    ipoint axy = get_last_point(pts, start);
    ipoint bxy = get_last_point(pts, axy);
    printf("axy = %d,%d bxy = %d,%d\n", axy.x, axy.y, bxy.x, bxy.y);
    
    auto [axy1, ad] = boundary_point_near(pts, axy);
    auto [bxy1, bd] = boundary_point_near(pts, bxy);
  
    printf("axy1 = %d,%d bxy1 = %d,%d\n", axy1.x, axy1.y, bxy1.x, bxy1.y);
    
    split_boundary(pts, axy1, bxy1, bd^2);
    }
  else {
    for(int y=5; y<SY-5; y++) 
    for(int x=5; x<SX-5; x++) 
      if(get_heart(ipoint(x,y)) == outpixel && get_heart(ipoint(x+1, y)) != outpixel) {
        createb_outer(ipoint(x, y));
        goto done;
        }    
    done: ;
    }
  
  computemap(pts);
  
  auto_joins();
  
  merge_sides();
  }

