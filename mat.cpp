// Hyperbolic geometry operations
// Copyright (C) 2018 Zeno Rogue, see 'nconf.cpp' for details

namespace nconf {

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

// the cosh of the distance of a hyperpoint h from the central point equals h[2]
// this function computes the cosh of distance of t*h

ld mulnorm(const transmatrix& t, hyperpoint h) {
  return t[6] * h[0] + t[7] * h[1] + t[8] * h[2];
  }

ld mulnorm(const transmatrix& t1, const transmatrix& t2) {
  return t1[6] * t2[2] + t1[7] * t2[5] + t1[8] * t2[8];
  }

// Minkowski hyperboloid to Poincare

cpoint hyper_to_disk(hyperpoint p) {
  return {p[0] / (1+p[2]), p[1] / (1+p[2])};
  }

hyperpoint disk_to_hyper(cpoint p) {
  hyperpoint h;
  ld no = sqrt(1 - (p|p));
  h[0] = p[0] / no; h[1] = p[1] / no; h[2] = 1 / no;
  return h;
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

transmatrix spin(ld alpha) {
  transmatrix T;
  for(int i=0; i<9; i++) T[i] = 0;
  T[0] = +cos(alpha); T[1] = +sin(alpha);
  T[3] = -sin(alpha); T[4] = +cos(alpha);
  T[8] = 1;
  return T;
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

transmatrix xpush(ld alpha) {
  transmatrix T;
  for(int i=0; i<9; i++) T[i] = 0;
  T[0] = +cosh(alpha); T[2] = +sinh(alpha);
  T[4] = 1;
  T[6] = +sinh(alpha); T[8] = +cosh(alpha);
  return T;
  }
  
transmatrix ypush(ld alpha) {
  transmatrix T;
  for(int i=0; i<9; i++) T[i] = 0;
  T[4] = +cosh(alpha); T[5] = +sinh(alpha);
  T[0] = 1;
  T[7] = +sinh(alpha); T[8] = +cosh(alpha);
  return T;
  }
  
// use periodicity to find the closest copy of the hyperpoint p
template<class T> T reperiod(T p, const vector<transmatrix>& periods) {
  int iter = 0;
  remap_again:
  iter++; if(iter<30)
  for(auto& z: periods) {
    ld nq = mulnorm(z, p);
    if(nq < p.back()) {
      p = mul(z, p);
      goto remap_again;
      }
    }
  
  return p;
  }

int sig(int a) { return a<2 ? 1 : -1; }

void fixmatrix(transmatrix& T) {
 for(int x=0; x<3; x++) for(int y=0; y<=x; y++) {
    ld dp = 0;
    for(int z=0; z<3; z++) dp += T[z*3+x] * T[z*3+y] * sig(z);
    
    if(y == x) dp = 1 - sqrt(sig(x)/dp);
    
    for(int z=0; z<3; z++) T[3*z+x] -= dp * T[3*z+y];
    }
  }

template<class T> int isize(const T& t) { return t.size(); }
}
