#include <complex>
#include <iostream>

bool spiral_mode = false;

cld mix(cld a1, cld a2, ld a, ld b, ld c) {
  return a1 + (a2-a1) * (b-a) / (c-a);
  }

void draw_spiral(bitmap& b, ld t) {
  int edgelength = SX/3;
  
  cld a (mousex, mousey);
  
  double d = mousex * 1. / SX;
  
  ld bscale = 1/3.2;
  
  cld ref(SX/2, SY*3/4);

  // d==0 : -1600, refy
  // d==1: 336, 230
  
  // ax = -1600 + (336+1600) * d;
  // ay = refy + (230-refy) * d;
  
  // a = ref + (mouse - ref) * 3;
  
  auto& band = sides[0].img_band[0];
  
  int CX = 0;
  int CY = band.s->h;
  // ld k = -2*M_PI*M_PI / log(2.6180339);
  // int Yshift = CY * k / M_PI;

  for(auto& bandimg: sides[0].img_band) CX += bandimg.s->w;
  
  ld z = (sin(t) + 1) / 2;

  ld alimit = real(ref) - CY * bscale / 2 / M_PI;
  printf("alimit = %lf\n", double(alimit));
  
  a = cld(alimit + 1000 - (1000/z), 147);

  // a = mix(cld(340, 147), cld(-5000, 300), -1, , 1);
  
  /* if(t < 1000)      a = mix(cld(326, 378), cld(322, 147), 0, t, 1000);
  else if(t < 2000) a = mix(cld(322, 147), cld(0, 0), 1000, t, 2000);
  else if(t < 3000) a = mix(cld(0, 0), cld(-5000, 300), 2000, t, 3000);
  else if(t < 4000) a = mix(cld(-5000, 300), cld(-5000, 300), 3000, t, 4000);
  else if(t < 5000) a = mix(cld(-2000, 600), cld(326, 378), 4000, t, 5000); */
    
  // 326 378  
  // 322 147
  // 0 0
  // -5000 300
  // 0 600  
  
  std::cout << a << std::endl;
    
  // printf("%d %d\n", ax, ay);
  
  for(int y=0; y<SY; y++)
  for(int x=0; x<SX; x++) { 
  
    cld xy(x, y);
  
    cld start = xy - a;
    
    cld z = ref - a;
    
    start /= z;
    
    start = log(start) + cld(1, 0);
    
    start *= z;
    
    cld period = cld(0, 2*M_PI) * z / bscale;
    if(imag(period) < 0) period = -period;
    
    // 2pi i * z 
    
    start += a;
    
    start -= ref;
    
    start /= bscale;
    
    start = conj(start);
    
    start += cld(CX / 2, CY/2);
    
    ld ry = imag(start);
    ld rx = real(start);
    
    int frac = int(ry / imag(period));
    
    ry -= frac * imag(period), rx -= frac * real(period);
    if(ry < 0) ry += imag(period), rx += real(period);
    
    int yy = int(ry), yx = int(rx);
    
    int periods = 0;
    while(yy >= CY && periods < 8) yy -= CY, periods++;
    
    if(yy >= CY) {
      ld mx = imag(period);
      ld v = (yy - CY) * (mx - yy);
      ld vmx = pow(mx - CY, 2) / 4;
      v /= vmx;
      v = 1 - v;
      b[y][x] = 0;
      // part(b[y][x], 2) = int(255 * v);
      continue;
      }
    
    yx %= CX;
    while(yx < 0) yx += CX;
    
    b[y][x] = band[yy][yx];

    for(auto& bandimg: sides[0].img_band) 
      if(yx < bandimg.s->w) {
        b[y][x] = band[yy][yx];
        break;
        }
      else {
        yx -= bandimg.s->w;
        }
      
    while(periods) {
      for(int a=0; a<3; a++) part(b[y][x], a) >>= 1;
      periods--;
      }

    if(xy == ref) {
      b[y][x] = 0xFFFFFF; continue;
      }
    
    /*
    cld zz(ref - xy);
    cld per = cld(0, 2*M_PI) * zz;
    if(abs(imag(per)) < CY) part(b[y][x], 2) = 0x40; */
    
    // 2pi * (refx-ax)/bscale == CY
    // refx-ax = CY * bscale / 2pi
    // ax = refx - CY * bscale / 2pi
    
    }    
  b.draw();
  }

