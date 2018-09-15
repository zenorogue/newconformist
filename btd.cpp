ld cspin;

pair<ld, ld> unband(cpoint& c, int si, ld shift) {
  ld y = c[1];
  ld x = c[0] + shift;
  
  y *= 2; y -= 1; // -1 .. 1
  x *= 2; x /= cscale[si][0];

  y *= M_PI / 2;
  x *= M_PI / 2;

  y = 2 * atanh(tan(y/2));
  
  return make_pair(x, y);
  }

hyperpoint equirectangular(ld x, ld y) {
  return { sinh(x) * cosh(y), sinh(y), cosh(y) * cosh(x)};
  }    

void set_column(transmatrix& M, int id, const hyperpoint& h) { 
  M[id] = h[0];
  M[id+3] = h[1];
  M[id+6] = h[2];
  }

ld det(const transmatrix& T) {
  ld det = 0;
  for(int i=0; i<3; i++) 
    det += T[i] * T[3+(i+1)%3] * T[6+(i+2)%3];
  for(int i=0; i<3; i++) 
    det -= T[i] * T[3+(i+2)%3] * T[6+(i+1)%3];
  return det;
  }

transmatrix inverse(const transmatrix& T) {  
  ld d = det(T);
  transmatrix T2;
  
  for(int i=0; i<3; i++) 
  for(int j=0; j<3; j++)
    T2[j*3+i] = (T[(i+1)%3*3+(j+1)%3] * T[(i+2)%3*3+(j+2)%3] - T[(i+1)%3*3+(j+2)%3] * T[(i+2)%3*3+(j+1)%3]) / d;

  return T2;
  }

int debugsi;

bool need_btd;

void shift_on_line(transmatrix& M, ld newx, ld& current_shift, int si) {
  while(newx > current_shift - 5) {
    M = mul(M, xpush(5));
    fixmatrix(M);
    M = reperiod(M, period_matrices[si]);
    current_shift += 5;
    }

  while(newx < current_shift + 5) {
    M = mul(M, xpush(-5));
    fixmatrix(M);
    M = reperiod(M, period_matrices[si]);
    current_shift -= 5;
    }
 
  M = mul(M, xpush(newx - current_shift));
  current_shift = newx;
  }

void construct_btd() {
  need_btd = (cspin > 0 || extrapts.size());

  transmatrix M = spin(cspin);
  ld current_shift = 0;
  
  int si = 0;
  
  for(auto& e: extrapts) {

    auto [old_x0, old_y0] = unband(pts[e.y][e.x].x, si, 0);
    auto [old_x1, old_y1] = unband(pts[e.y][e.x+10].x, si, 0);
    auto [old_x2, old_y2] = unband(pts[e.y+10][e.x].x, si, 0);

    auto [new_x0, new_y0] = unband(e.epts[e.y][e.x].x, e.sideid, 0);
    auto [new_x1, new_y1] = unband(e.epts[e.y][e.x+10].x, e.sideid, 0);
    auto [new_x2, new_y2] = unband(e.epts[e.y+10][e.x].x, e.sideid, 0);
    
    shift_on_line(M, old_x0, current_shift, 0);
    
    transmatrix mold, mnew;
    set_column(mold, 0, equirectangular(0, old_y0));
    set_column(mold, 1, equirectangular(old_x1 - old_x0, old_y1));
    set_column(mold, 2, equirectangular(old_x2 - old_x0, old_y2));

    set_column(mnew, 0, equirectangular(0, new_y0));
    set_column(mnew, 1, equirectangular(new_x1 - new_x0, new_y1));
    set_column(mnew, 2, equirectangular(new_x2 - new_x0, new_y2));
      
    M = mul(mul(M, mold), inverse(mnew));
    fixmatrix(M);
    
    e.matrix = M;
    e.shift = new_x0;
    }  
  }

cpoint band_to_disk(int px, int py, int si) {

  cpoint c = pts[py][px].x;
  
  hyperpoint p;
  
  if(need_btd) {
    int csi = si;
    
    auto [x,y] = unband(c, si, 0);
    ld shift = 0;
    transmatrix M = spin(cspin);

    for(auto& e: extrapts) {
    
      auto [nx, ny] = unband(e.epts[py][px].x, e.sideid, 0);

      if(nx > x) {
        M = e.matrix;
        shift = e.shift;
        csi = e.sideid;
        x = nx; y = ny;
        }
      }
    
    shift_on_line(M, x, shift, si);

    p = {0, sinh(y), cosh(y)};
    p = mul(M, p);
    p = reperiod(p, period_matrices[si]);
    
    debugsi = csi;
    }
  
  else {

    auto [x,y] = unband(c, si, -xcenter[si]);
    
    if(period[si] > 0) {
      ld d = period[si];
      while(x > d/2) x -= d;
      while(x < -d/2) x += d;
      }
     
    p = equirectangular(x, y);
    
    p = mul(spin(cspin), p);
    
    p = reperiod(p, period_matrices[si]);    
    }
  
  cpoint pt = hyper_to_disk(p);  
  return (cpoint{1, 1} + pt) * (img[si].s->h / 2);
  }
