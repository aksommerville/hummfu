/* scene.h
 * The game's model world.
 * Responsible for everything except the two modals.
 */
 
#ifndef SCENE_H
#define SCENE_H

#include "score.h"

struct sprite;

struct scene {
  int active;
  int mapid;
  int bgtexid;
  struct sprite **spritev;
  int spritec,spritea;
  struct sprite *hero; // WEAK, OPTIONAL
  const uint8_t *map; // (NS_sys_mapw*NS_sys_maph)
  double strikeclock;
  int strikex,strikey;
  double deathclock;
  double winclock;
  double fadein;
  
  /* This score is per-level.
   * When map:1 begins, we flush it to the defaults, and also flush (g.score).
   * Beginning the same map again resets all but clock and death count.
   * Beginning any other map commits what we have to (g.score) and resets completely.
   * Statistics sources like sprite controllers may increment the counters here directly.
   */
  struct score score;
};

void scene_update(struct scene *scene,double elapsed,int input,int pvinput);
void scene_render(struct scene *scene);

int scene_begin(struct scene *scene,int mapid);

struct sprite *scene_spawn_sprite(struct scene *scene,double x,double y,int spriteid,uint32_t arg);

void scene_highlight_strike(struct scene *scene,double x,double y);
void scene_begin_death(struct scene *scene);
void scene_begin_victory(struct scene *scene);
void scene_no_fade_in(struct scene *scene);

#endif
