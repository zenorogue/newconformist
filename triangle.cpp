void create_triangle(int edgelength) {
  single_side(stype::standard);
  SX = edgelength * 3;
  SY = int(edgelength * sqrt(3));
  int base = SY * 3 / 4;
  resize_pt();
  for(int y=0; y<SY; y++) 
  for(int x=0; x<SX; x++) {
    pts[y][x].side = 0;
    pts[y][x].type = ptype::outside;
    if(y <= base && y - abs(x-SX/2) * sqrt(3) >= SY/4)
      pts[y][x].type = ptype::inside;
    }
  split_boundary(pts, ipoint(SX/2, base+1), ipoint(SX/2, SY/4-1), 1);
  }

bool mirror(double& dx, double& dy, double ax, double ay, double cx, double cy) {
  double adx = dx - ax, ady = dy - ay;
  cx -= ax; cy -= ay;
  
  ld d = hypot(cx, cy); cx /= d; cy /= d;
  ld cros = adx * cy - ady * cx;

  if(cros < 0) {
    dx = dx - cros * 2 * cy;
    dy = dy + cros * 2 * cx;
    return true;
    }
  return false;
  }

bool triangle_mode;

cpoint band_to_disk_basic(cpoint c, sideinfo& si) {
  ld y = c[1];
  ld x = c[0] - si.xcenter;
  
  y *= 2; y -= 1; // -1 .. 1
  x *= 2; x /= si.cscale[0];

  y *= M_PI / 2;
  x *= M_PI / 2;

  y = 2 * atanh(tan(y/2));

  hyperpoint p = { sinh(x) * cosh(y), sinh(y), cosh(y) * cosh(x)};
  
  return hyper_to_disk(p);
  }

vector2<cld> map_to_stereo;

void draw_triangle(bitmap& b) {
  int edgelength = SX/3;
  
  map_to_stereo.resize2(SX, SY);

  sideinfo *goodside;
    
  for(int y=0; y<SY; y++)
  for(int x=0; x<SX; x++) { 
    double dx = x, dy = y;
    int flips = 0;

    int steps = 0;
    while(true) {
      steps++; if(steps > 100) break;
      // printf("[%lf %lf]\n", dx, dy);
      if(dy > SY*3/4) dy = SY*3/2 - dy, flips++;
      else if(mirror(dx, dy, SX/2, SY/4, SX/3, SY*3/4)) flips++;
      else if(mirror(dx, dy, SX*2/3, SY*3/4, SX/2, SY/4)) flips++;
      else break;
      }
    
    auto& p = pts[dy][dx];    
    // printf("=> done %d\n", p.type);
    if(p.type != ptype::inside) continue;
    goodside = &sides[p.side];
    auto dc = band_to_disk_basic(p.x, *goodside);
    auto cdc = cld(dc[0], dc[1]);
    map_to_stereo[y][x] = (flips&1) ? conj(cld(1,0) / cdc) : cdc;
    }
    
  ld scale = 1/200.;
  
  /*
  { int y = SY/2;
  for(int x=SX/2+1; x<SX; x++)
    map_to_stereo[y][x] = map_to_stereo[y][x-1] + map_to_stereo[y][x] * scale;
  for(int x=SX/2-1; x>=0; x--)
    map_to_stereo[y][x] = map_to_stereo[y][x+1] - map_to_stereo[y][x+1] * scale;
    }
  for(int x=0; x<SX; x++) {
    for(int y=SY/2+1; y<SY; y++)
      map_to_stereo[y][x] = map_to_stereo[y-1][x] + map_to_stereo[y][x] * scale / cld(0, 1);
    for(int y=SY/2-1; y>=0; y--)
      map_to_stereo[y][x] = map_to_stereo[y+1][x] - map_to_stereo[y+1][x] * scale / cld(0, 1);
    } */

  for(int y=0; y<SY; y++)
  for(int x=0; x<SX-1; x++)
    map_to_stereo[y][x] = (map_to_stereo[y][x+1] - map_to_stereo[y][x]) / scale;
    
  
  for(int y=0; y<SY; y++)
  for(int x=0; x<SX; x++) { 
    auto& si = *goodside;

    auto dc = map_to_stereo[y][x];
    ld longitude = arg(dc);
    ld r2 = norm(dc);
    ld r = sqrt(r2);
    
    ld zg = (r2 - 1)/(r2 + 1);
    
    ld latitude = atan2(-zg, r*(1-zg));
    
    // if(flips&1) latitude = -latitude;
    
    while(longitude < 0) longitude += 2 * M_PI;
    while(longitude >= 2 * M_PI) longitude -= 2 * M_PI;
    
    auto& img = si.img;
    
    int ax = img.s->w * longitude / 2 / M_PI;
    int ay = img.s->h * (latitude + M_PI/2) / M_PI;
    // ay %= img[si].s->h; if(ay < 0) ay += img[si].s->h;
    
    // b[y][x] = latitude * 60 + 128 + (int(longitude * 40) << 8); //  
    b[y][x] = img[ay][ax];
    }    
  b.draw();
  printf("draw\n");
  }

