/* sprite_flower.c
 * arg: u24=reserved, u8=win_tileid
 */
 
#include "game/hummfu.h"

struct sprite_flower {
  struct sprite hdr;
  uint8_t tileid0;
  uint8_t win_tileid;
  int bloomed;
  double shockclock;
};

#define SPRITE ((struct sprite_flower*)sprite)

static int _flower_init(struct sprite *sprite) {
  SPRITE->tileid0=sprite->tileid;
  SPRITE->win_tileid=sprite->arg;
  if (sprite->tileid==SPRITE->win_tileid) {
    fprintf(stderr,"flower sprite requires a different win_tileid\n");
    return -1;
  }
  sprite->pl=-0.333;
  sprite->pr= 0.333;
  sprite->pt=-0.333;
  sprite->pb= 0.333;
  return 0;
}

static void flower_check_completion() {
  struct sprite **p=g.scene.spritev;
  int i=g.scene.spritec;
  for (;i-->0;p++) {
    struct sprite *sprite=*p;
    if (sprite->defunct) continue;
    if (sprite->type!=&sprite_type_flower) continue;
    if (!SPRITE->bloomed) return;
  }
  scene_begin_victory(&g.scene);
}

static void _flower_update(struct sprite *sprite,double elapsed) {

  if (SPRITE->shockclock>0.0) {
    if ((SPRITE->shockclock-=elapsed)<=0.0) {
      sprite->tileid=SPRITE->bloomed?SPRITE->win_tileid:SPRITE->tileid0;
    }
  }
  
  if (SPRITE->bloomed) {
    if ((SPRITE->win_tileid==0x1d)&&!g.scene.hero&&(SPRITE->shockclock<=0.0)) sprite->tileid=0x3b;
    return;
  }
  
  if (!g.scene.hero) return;
  const double radius=0.500;
  double dx=g.scene.hero->x-sprite->x;
  if ((dx<-radius)||(dx>radius)) return;
  double dy=g.scene.hero->y-sprite->y;
  if ((dy<-radius)||(dy>radius)) return;
  double pan=(sprite->x*2.0)/NS_sys_mapw-1.0;
  egg_play_sound(RID_sound_bloom,1.0,pan);
  sprite->tileid=SPRITE->win_tileid;
  SPRITE->bloomed=1;
  scene_spawn_sprite(&g.scene,sprite->x,sprite->y-0.500,RID_sprite_flowerlove,0);
  
  flower_check_completion();
}

static int _flower_strike(struct sprite *sprite,struct sprite *assailant) {
  if (!assailant) return 0;
  if (SPRITE->shockclock>0.0) return 0;
  SPRITE->shockclock=0.333;
  sprite->tileid=SPRITE->bloomed?SPRITE->win_tileid:SPRITE->tileid0;
  if (assailant->x<sprite->x) {
    sprite->tileid+=0x20;
  } else {
    sprite->tileid+=0x10;
  }
  return 1;
}

const struct sprite_type sprite_type_flower={
  .name="flower",
  .objlen=sizeof(struct sprite_flower),
  .init=_flower_init,
  .update=_flower_update,
  .strike=_flower_strike,
};
