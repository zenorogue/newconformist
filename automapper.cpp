
ipoint get_last_point(ipoint start) {
  vector<ipoint> q;
  pts[start].type = 2; q.push_back(start);
  for(int i=0; i<isize(q); i++) {
    auto xy = q[i];
    for(auto k: dv) if(pts[k+xy].type == 1)
      pts[k+xy].type = 2, q.push_back(k + xy);
    }
  for(auto pt: q) pts[pt].type = 1;
  return q.back();
  }

void auto_mapin() {
  single_side(0);
  auto outpixel = get_heart(ipoint{0,0});
  
  ipoint start;

  for(int y=0; y<SY; y++) 
  for(int x=0; x<SX; x++) {
    auto xy = ipoint(x, y);
    auto& p = pts[xy];

    p.side = 0;
    if(get_heart(xy) != outpixel && x && y && x < SX-1 && y < SY-1)
      p.type = 1, start = xy;
    else
      p.type = 0;
    }
  
  ipoint axy = get_last_point(start);
  ipoint bxy = get_last_point(axy);
  
  auto [axy1, ad] = boundary_point_near(addmargin(axy));
  auto [bxy1, bd] = boundary_point_near(addmargin(bxy));
  
  split_boundary(axy1, bxy1, bd^2);
  
  computemap(pts);
  }

