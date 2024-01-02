// translate computed coordinates to Poincare disk coordinates
// includes the algorithm for syncing the branched shapes (construct_btd)

namespace nconf {
ld cspin;

pair<ld, ld> unband(cpoint& c, sideinfo& si, ld shift) {
  ld y = c[1];
  ld x = c[0] + shift;
  
  y *= 2; y -= 1; // -1 .. 1
  x *= 2; x /= si.cscale[0];

  y *= M_PI / 2;
  x *= M_PI / 2;

  y = -2 * atanh(tan(y/2));
  
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

transmatrix get_matrix_at(sideinfo& si, ld x) {
  x -= si.zero_shift;

  if(si.type == stype::fixed_ring) {
    ld d = si.period;
    x -= si.xcenter / si.cscale[0];
    while(x > d/2) x -= d;
    while(x < -d/2) x += d;
    return mul(si.matrixlist[0], xpush(x));
    }     

  int x0 = int(x);
  if(x0 >= 0) {
    while(isize(si.matrixlist) <= x0) {
      transmatrix M = mul(si.matrixlist.back(), xpush(1));
      fixmatrix(M);
      M = reperiod(M, rootof(si).period_matrices);
      si.matrixlist.push_back(M);
      }
    return mul(si.matrixlist[x0], xpush(x - x0));
    }
  else {
    if(si.rmatrixlist.empty()) si.rmatrixlist.push_back(si.matrixlist[0]);
    x0 = -x0;
    while(isize(si.rmatrixlist) <= x0) {
      transmatrix M = mul(si.rmatrixlist.back(), xpush(-1));
      fixmatrix(M);
      M = reperiod(M, rootof(si).period_matrices);
      si.rmatrixlist.push_back(M);
      }
    return mul(si.rmatrixlist[x0], xpush(x + x0));
    }
  }

void construct_btd_for(sideinfo& si) {
  si.matrixlist.clear();
  si.rmatrixlist.clear();

  if(si.parentid == si.id) {
    si.need_btd = cspin > 0;
    si.matrixlist.push_back(spin(cspin));
    si.zero_shift = -si.animshift;
    }
  
  else {      
    // auto& root = rootof(si);
    auto& par = sides[si.parentid];
    par.need_btd = true;
    
    ipoint ex0 = si.join;
    ipoint ex1 = ex0 + ipoint(1, 0);
    ipoint ex2 = ex0 + ipoint(0, 1);
    
    auto& ppts = *par.submap;

    auto [old_x0, old_y0] = unband(ppts[ex0].x, par, 0);
    auto [old_x1, old_y1] = unband(ppts[ex1].x, par, 0);
    auto [old_x2, old_y2] = unband(ppts[ex2].x, par, 0);
    
    auto& epts = *si.submap;

    auto [new_x0, new_y0] = unband(epts[ex0].x, si, 0);
    auto [new_x1, new_y1] = unband(epts[ex1].x, si, 0);
    auto [new_x2, new_y2] = unband(epts[ex2].x, si, 0);
    
    transmatrix T = get_matrix_at(par, old_x0);
    
    transmatrix mold, mnew;
    set_column(mold, 0, equirectangular(0, old_y0));
    set_column(mold, 1, equirectangular(old_x1 - old_x0, old_y1));
    set_column(mold, 2, equirectangular(old_x2 - old_x0, old_y2));

    set_column(mnew, 0, equirectangular(0, new_y0));
    set_column(mnew, 1, equirectangular(new_x1 - new_x0, new_y1));
    set_column(mnew, 2, equirectangular(new_x2 - new_x0, new_y2));
      
    T = mul(T, mul(mold, inverse(mnew)));
    fixmatrix(T);
    
    si.matrixlist.push_back(T);
    si.zero_shift = new_x0;
    
    ld dx0 = old_x0 - par.zero_shift;
    
    si.parentrel_x = int(dx0);
    si.parentrel_matrix = mul(xpush(dx0-si.parentrel_x), mul(mold, inverse(mnew)));
    }
  }

void construct_btd() {
  static int p;
  p++;
  for(auto& si: sides) 
    construct_btd_for(si);
  }

ld btd_at = 2;

struct shift_data {
  ld px, py, ps;
  };
bool use_shift;
shift_data cshift;

cpoint band_to_disk(int px, int py, sideinfo& si, int& tsiid, ld& xval, ld& yval, bool to_img = true) {

  cpoint c = pts[py][px].x;
  
  hyperpoint p;
  
  if(si.need_btd) {
  
    auto csi = &si;
    
    auto [x,y] = unband(c, *csi, 0);
    
    parent_changed:
        
    if(use_childsides) for(int subid: csi->childsides) {
      auto& nsi = sides[subid];
      auto& epts = *nsi.submap;
      
      if(epts[py][px].type != ptype::inside) continue;
    
      auto [nx, ny] = unband(epts[py][px].x, nsi, 0);
      
      if(nx > nsi.zero_shift) {
        x = nx; y = ny;
        csi = &nsi;
        goto parent_changed;
        }
      }
    
    p = {0, sinh(y), cosh(y)};
    p = mul(get_matrix_at(*csi, x), p);
    p = reperiod(p, si.period_matrices);
    tsiid = csi->id;
    yval = y;
    xval = x;
    }
  
  else {

    auto [x,y] = unband(c, si, -si.xcenter);
    
    if(std::isinf(x) || std::isnan(x)) {
      printf("c = %Lf,%Lf\n", c[0], c[1]);
      printf("xcenter = %Lf\n", si.xcenter);
      printf("scale = %Lf\n", si.cscale[0]);
      exit(1);
      }

    if(si.period > 0) {
      ld d = si.period;
      while(x > d/2) x -= d;
      while(x < -d/2) x += d;
      }
     
    p = equirectangular(x, y);
    
    if(use_shift) {
      p = mul(xpush(cshift.px), p);
      p = mul(ypush(cshift.py), p);
      p = mul(spin(cshift.ps), p);
      }

    p = mul(spin(cspin), p);
    
    p = reperiod(p, si.period_matrices);
    yval = y;
    }
  
  cpoint pt = hyper_to_disk(p);  
  #if CAP_BMP
  if(!si.img.s || !to_img) return pt;
  return (cpoint{1, 1} + pt) * (si.img.s->h / 2);
  #else
  return pt;
  #endif
  }

void measure_if_needed();

void prepare_all_matrices() {
  measure_if_needed();
  construct_btd();

  for(int y=0; y<SY; y++)
  for(int x=0; x<SX; x++) {
    auto& p = pts[y][x];

    if(inner(p.type)) {
      ld xval = 0, yval = 0;
      int tsiid = p.side;
      /*auto dc =*/ band_to_disk(x, y, sides[tsiid], tsiid, xval, yval);
      }
    }
  }
  
void create_viewlist(int current_side, string fname) {
  prepare_all_matrices();

  FILE *f = fopen(fname.c_str(), "wt");
  for(auto& si: sides) {
    if(si.rootid == current_side) {
      if(si.parentid != si.id) {
        fprintf(f, "%d %d %d ", si.id, si.parentid, si.parentrel_x);
        for(int i=0; i<9; i++) fprintf(f, "%lf ", double(si.parentrel_matrix[i]));
        }
      else fprintf(f, "%d ", si.id);
      fprintf(f, "%d %d\n", isize(si.matrixlist), isize(si.rmatrixlist));
      }
    }
  fprintf(f, "%d\n", -1);
  fclose(f);
  }

#if CAP_GD
void read_viewlist(int /* ignored current_side */, string format) {
  prepare_all_matrices();

  for(auto& si: sides) {
    for(int i=0; i<isize(si.matrixlist); i++) {
      char buf[1000];
      sprintf(buf, format.c_str(), si.id, i);
      FILE *f = fopen(buf, "rb");
      if(f) {
        printf("found %s\n", buf);
        fclose(f);
        si.img_line.push_back(readPng(buf));        
        }
      }
    }
  }
#endif
}
