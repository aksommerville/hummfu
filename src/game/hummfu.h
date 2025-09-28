#ifndef EGG_GAME_MAIN_H
#define EGG_GAME_MAIN_H

#include "egg/egg.h"
#include "opt/stdlib/egg-stdlib.h"
#include "opt/graf/graf.h"
#include "opt/font/font.h"
#include "opt/res/res.h"
#include "egg_res_toc.h"
#include "shared_symbols.h"
#include "scene.h"
#include "modals.h"
#include "score.h"
#include "sprite/sprite.h"

#define FBW 320
#define FBH 176

#define SFXV_LIMIT 4

extern struct g {
  void *rom;
  int romc;
  struct graf graf;
  struct font *font;
  struct rom_entry *resv;
  int resc,resa;
  int pvinput;
  int framec;
  int texid_sprites;
  int texid_terrain;
  uint8_t physics[256]; // tilesheet:terrain; captured at init
  struct score score; // The session score. Updates only as each level completes.
  int killc_max; // Count of killable monsters across all maps, for scoring purposes.
  int breakc_max; // '' breakable boxes.
  // There's no corrollary to (killc_max,breakc_max) for flowers -- to get a score at all, you have to bloom all of them.
  char hiscore[SCORE_LENGTH];
  
  // No generalized modes. We're doing one of these two things.
  struct scene scene;
  struct gameover gameover;
  
  int sfxv[SFXV_LIMIT];
  int sfxc;
} g;

int hummfu_get_res(void *dstpp,int tid,int rid);
int hummfu_load_label(int *w,int *h,int ix); // => texid; strings:1

void hummfu_sfx(int soundid);
void hummfu_sfx_spatial(int soundid,double x); // (x) in sprite or map space -- 0..NS_sys_mapw

#endif
