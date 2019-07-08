void create_diamond(int edgelength) {
  single_side(stype::standard);
  SX = SY = edgelength * 2 + 1;
  resize_pt();
  for(int y=0; y<SY; y++) 
  for(int x=0; x<SX; x++) {
    pts[y][x].side = 0;
    pts[y][x].type = ptype::outside;
    if(abs(y-SY/2) + abs(x-SX/2) < edgelength)
      pts[y][x].type = ptype::inside;
    }
  split_boundary(pts, ipoint(0, edgelength), ipoint(edgelength*2, edgelength), 0);
  }

void do_quincunx() {
  auto& side = new_side(stype::standard);
  int edgelength = SX/2;
  for(int y=0; y<SY; y++)
  for(int x=0; x<SX; x++) {
    if(y == 0 || x == 0 || y == SY-1 || x == SX-1) continue;
    int ax = x, ay = y;
    int dx = ax > edgelength ? SX - 1 - ax : ax;
    int dy = ay > edgelength ? SY - 1 - ay : ay;
    if(dx + dy >= edgelength) continue;
    tie(dx, dy) = make_pair(edgelength - dy, edgelength - dx);
    dx = ax > edgelength ? SX - 1 - dx : dx;
    dy = ay < edgelength ? SY - 1 - dy : dy;
    
    pts[ay][ax] = pts[dy][dx];
    pts[ay][ax].side = side.id;
    }
  current_side = side.id; 
  }

int gmodc(int x, int siz) {
  x += siz/2;
  x %= siz;
  if(x < 0) x += siz;
  return x;
  }
  
void load_image_for_quincunx(const string& fname, ld scale) {
  bitmap b = readPng(fname);
  int Res = 1000;
  int BXY = 2 * Res + 1;
  sides[0].img = emptyBitmap(BXY, BXY);
  sides[1].img = emptyBitmap(BXY, BXY);
  for(int x=0; x<BXY; x++)
  for(int y=0; y<BXY; y++) {
    ld y0 = y - Res;
    ld x0 = x - Res;
    sides[0].img[y][x] = b[gmodc(y0*scale, b.s->h)][gmodc(x0*scale, b.s->w)] | 0xFF000000;
    ld rad0 = (x0*x0 + y0*y0) / (Res*Res);
    x0 /= rad0;
    y0 /= rad0;
    y0 = -y0;    
    sides[1].img[y][x] = b[gmodc(y0*scale, b.s->h)][gmodc(x0*scale, b.s->w)] | 0xFF000000;
    }
  writePng("side0.png", sides[0].img);
  writePng("side1.png", sides[1].img);
  }
