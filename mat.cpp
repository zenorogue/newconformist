// Hyperbolic geometry operations
// Copyright (C) 2018 Zeno Rogue, see 'nconf.cpp' for details

// a point in the band coordinates or Poincare disk coordinates
typedef array<ld, 2> cpoint;

// a point in the Minkowski hyperboloid coordinates
typedef array<ld, 3> hyperpoint;

// hyperbolic isometries are linear transformations of the Minkowski hyperboloid;
// they are represented by matrices
typedef array<ld, 9> transmatrix;

ld operator ^ (cpoint v0, cpoint v1) {
  return v0[0] * v1[1] - v0[1] * v1[0];
  }

ld operator | (cpoint v0, cpoint v1) {
  return v0[0] * v1[0] + v0[1] * v1[1];
  }

cpoint operator + (cpoint v0, cpoint v1) {
  return cpoint{v0[0]+v1[0], v0[1]+v1[1]};
  }

cpoint operator - (cpoint v0, cpoint v1) {
  return cpoint{v0[0]-v1[0], v0[1]-v1[1]};
  }

cpoint operator / (cpoint v0, ld d) {
  return cpoint{v0[0]/d, v0[1]/d};
  }

cpoint operator * (cpoint v0, ld d) {
  return cpoint{v0[0]*d, v0[1]*d};
  }

bitmap img[MAXSIDE];
vector<bitmap> img_band[MAXSIDE];
cpoint cscale[MAXSIDE];
ld xcenter[MAXSIDE];
ld period[MAXSIDE];
ld period_unit[MAXSIDE];
vector<transmatrix> period_matrices[MAXSIDE];
int sidetype[MAXSIDE];

// the cosh of the distance of a hyperpoint h from the central point equals h[2]
// this function computes the cosh of distance of t*h

ld mulnorm(const transmatrix& t, hyperpoint h) {
  return t[6] * h[0] + t[7] * h[1] + t[8] * h[2];
  }

// Minkowski hyperboloid to Poincare

cpoint hyper_to_disk(hyperpoint p) {
  return {p[0] / (1+p[2]), p[1] / (1+p[2])};
  }

// matrix multiplication

hyperpoint mul(const transmatrix& t, hyperpoint h) {
  hyperpoint res;
  for(int u=0; u<3; u++) 
    res[u] = 
      t[3*u+0] * h[0] +
      t[3*u+1] * h[1] +
      t[3*u+2] * h[2];
  return res;
  }

transmatrix mul(const transmatrix& t1, const transmatrix& t2) {
  transmatrix res;
  for(int u=0; u<3; u++) for(int v=0; v<3; v++)
    res[3*u+v] = 
      t1[3*u+0] * t2[0+v] +
      t1[3*u+1] * t2[3+v] +
      t1[3*u+2] * t2[6+v];
  return res;
  }
  
// use periodicity to find the closest copy of the hyperpoint p
hyperpoint reperiod(hyperpoint p, const vector<transmatrix>& periods) {
  int iter = 0;
  remap_again:
  iter++; if(iter<30)
  for(auto& z: periods) {
    ld nq = mulnorm(z, p);
    if(nq < p[2]) {
      p = mul(z, p);
      goto remap_again;
      }
    }
  
  return p;
  }

cpoint band_to_disk(cpoint c, int si) {

  ld y = c[1];
  ld x = c[0] - xcenter[si];
  
  y *= 2; y -= 1; // -1 .. 1
  x *= 2; x /= cscale[si][0];

  y *= M_PI / 2;
  x *= M_PI / 2;

  y = 2 * atanh(tan(y/2));
   
  if(period[si] > 0) {
    ld d = period[si];
    while(x > d/2) x -= d;
    while(x < -d/2) x += d;
    }
   
  hyperpoint p = { sinh(x) * cosh(y), sinh(y), cosh(y) * cosh(x)};
  
  p = reperiod(p, period_matrices[si]);

  cpoint pt = hyper_to_disk(p);
  
  return (cpoint{1, 1} + pt) * (img[si].s->h / 2);
  }
