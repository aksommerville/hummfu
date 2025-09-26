#ifndef EGG_GAME_MAIN_H
#define EGG_GAME_MAIN_H

#include "egg/egg.h"
#include "opt/stdlib/egg-stdlib.h"
#include "opt/graf/graf.h"
#include "opt/res/res.h"
#include "egg_res_toc.h"
#include "shared_symbols.h"
#include "scene.h"
#include "modals.h"
#include "sprite/sprite.h"

#define FBW 320
#define FBH 176

extern struct g {
  void *rom;
  int romc;
  struct graf graf;
  struct rom_entry *resv;
  int resc,resa;
  int pvinput;
  int framec;
  int texid_sprites;
  int texid_terrain;
  uint8_t physics[256]; // tilesheet:terrain; captured at init
  
  // No generalized modes. We're doing one of these three things.
  struct scene scene;
  struct hello hello;
  struct gameover gameover;
} g;

int hummfu_get_res(void *dstpp,int tid,int rid);

#endif
