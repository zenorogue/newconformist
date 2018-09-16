// a libsdl/libgd wrapper
// Copyright (C) 2018 Zeno Rogue, see 'nconf.cpp' for details

#ifndef _GRAPH_H_
#define _GRAPH_H_

#include <cstdio>
#include <cmath>
#include <string>

#include <SDL/SDL.h>
#include <gd.h>

// ipoint

struct ipoint {
  int x, y;
  ipoint(int _x, int _y): x(_x), y(_y) {}
  ipoint() {}
  ipoint& operator += (const ipoint& a) { x += a.x; y += a.y; return *this; }
  ipoint& operator -= (const ipoint& a) { x -= a.x; y -= a.y; return *this; }
  ipoint operator + (ipoint a) const { a += (*this); return a; }
  ipoint operator - (ipoint a) const { a -= (*this); return a; }
  };

typedef long double ld;

// not necessary in C++17
// template<class T> int size(const T& t) { return t.size(); }

typedef unsigned int color;
typedef unsigned char colorpart;

colorpart& part(color& col, int i) {
  colorpart* c = (colorpart*) &col;
  return c[i];
  }

void die(const std::string& s) { 
  fprintf(stderr, "%s\n", s.c_str());
  exit(1);
  }

void pdie(const std::string& s) { 
  perror(s.c_str());
  exit(1);
  }

color errpixel;

struct pixelrow {
  color *v;
  int size;
  color& operator [] (int x) { if(x >= size || x < 0) return errpixel; else return v[x]; }
  };

struct bitmap {
  SDL_Surface *s;
  bool locked;
  bool isscreen;
  void draw() {
    if(isscreen) beunlocked(), SDL_UpdateRect(s, 0, 0, 0, 0);
    }
  void belocked() {
    if(isscreen && !locked) locked = true, SDL_LockSurface(s);
    }
  void beunlocked() {
    if(isscreen && locked) locked = false, SDL_UnlockSurface(s);
    }
  color& operator [] (ipoint xy) const { return (*this)[xy.y][xy.x]; }
  pixelrow operator [] (int y) const {
    if(y<0 || y>=s->h) return pixelrow {&errpixel, 1};
    unsigned char *dst = (unsigned char*) s->pixels;
    dst += y*s->pitch;
    color* ptr = (color*) dst;
    return pixelrow{ptr, s->w};
    }
  ~bitmap() { if(s && !isscreen) SDL_FreeSurface(s); }
  bitmap& operator= (bitmap&& b) { s = b.s; locked = b.locked; isscreen = b.isscreen; b.s = NULL; return *this; }
  bitmap(bitmap&& b) { s = b.s; locked = b.locked; isscreen = b.isscreen; b.s = NULL; }
  bitmap(SDL_Surface *s, bool l, bool is) : s(s), locked(l), isscreen(is) {}
  bitmap() { s = NULL; }
  };                           

bitmap screen ( NULL, false, true );

bitmap surfaceToBitmap(SDL_Surface *s) {
  bitmap b;
  b.s = s; b.locked = true; b.isscreen = false;
  return b;
  }

void initGraph(int sx, int sy, const std::string& title, int flags) {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("SDL error\n");
    SDL_Quit();
    }

  if(screen.s && sx == screen.s->w && sy == screen.s->h) return;
  screen.s = SDL_SetVideoMode(sx, sy, 32, flags);

  SDL_WM_SetCaption(title.c_str(), 0);
  SDL_EnableKeyRepeat(500, 10);
  SDL_EnableUNICODE(1);
  }

SDL_Surface *emptySurface(int sx, int sy) {
  return SDL_CreateRGBSurface(SDL_SWSURFACE, sx, sy, 32, 0xFF<<16,0xFF<<8,0xFF,0xFF<<24);
  }

bitmap emptyBitmap(int sx, int sy) {
  return bitmap(emptySurface(sx, sy), true, false);
  }

bitmap fromGD(gdImagePtr im) {
  bitmap tgt = emptyBitmap(im->sx, im->sy);
  for(int y=0; y<im->sy; y++)
  for(int x=0; x<im->sx; x++) {
    if(im->trueColor)
      tgt[y][x] = im->tpixels[y][x];
    else {
      int p = im->pixels[y][x];
      color& px(tgt[y][x]);
      part(px, 2) = im->red[p];
      part(px, 1) = im->green[p];
      part(px, 0) = im->blue[p];
      part(px, 3) = 255 - im->alpha[p];
      }
    part(tgt[y][x], 3) = 255 - part(tgt[y][x], 3);
    }
  return tgt;
  }

gdImagePtr toGD(const bitmap &bmp) {
  gdImagePtr im = gdImageCreateTrueColor(bmp.s->w, bmp.s->h);
  for(int y=0; y<im->sy; y++)
  for(int x=0; x<im->sx; x++) {
    color c = bmp[y][x];
    part(c, 3) = 255 - part(c, 3);
    im->tpixels[y][x] = c;
    }
  return im;
  }

bitmap readPng(const std::string& fname) {
  FILE *f = fopen(fname.c_str(), "rb");
  if(!f) pdie("readPng");
  gdImagePtr im = gdImageCreateFromPng(f);
  bitmap tgt = fromGD(im);
  gdImageDestroy(im);
  fclose(f);
  return tgt;
  }

void writePng(const std::string& fname, const bitmap& bmp) {
  FILE *f = fopen(fname.c_str(), "wb");
  if(!f) pdie("writePng");
  gdImagePtr im = toGD(bmp);
  gdImageSaveAlpha(im, 1);
  gdImagePng(im, f);
  gdImageDestroy(im);
  fclose(f);
  }

#endif