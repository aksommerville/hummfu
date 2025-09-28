/* sprite_laser.c
 * arg: u24=reserved, u8=direction (0,EGG_XFORM_XREV = right,left)
 */
 
#include "game/hummfu.h"

#define SPEED 8.0

struct sprite_laser {
  struct sprite hdr;
  double animclock;
  int animframe;
  uint8_t tileid0;
  double dx;
  struct sprite *owner; // WEAK, and do not dereference!
};

#define SPRITE ((struct sprite_laser*)sprite)

static int _laser_init(struct sprite *sprite) {
  SPRITE->tileid0=sprite->tileid;
  SPRITE->dx=(sprite->arg&EGG_XFORM_XREV)?-SPEED:SPEED;
  return 0;
}

static void _laser_update(struct sprite *sprite,double elapsed) {
  if ((SPRITE->animclock-=elapsed)<=0.0) {
    SPRITE->animclock+=0.100;
    if (++(SPRITE->animframe)>=2) SPRITE->animframe=0;
    sprite->tileid=SPRITE->tileid0+SPRITE->animframe;
  }
  sprite->x+=SPRITE->dx*elapsed;
  int col=(int)sprite->x;
  if ((col<0)||(col>=NS_sys_mapw)) {
    sprite->defunct=1;
    return;
  }
  int row=(int)sprite->y;
  if ((row<0)||(row>=NS_sys_maph)) {
    sprite->defunct=1;
    return;
  }
  switch (g.physics[g.scene.map[row*NS_sys_mapw+col]]) {
    case NS_physics_solid: {
        sprite->defunct=1;
      } return;
  }
  struct sprite **p=g.scene.spritev;
  int i=g.scene.spritec;
  for (;i-->0;p++) {
    struct sprite *victim=*p;
    if (!victim->type->strike) continue;
    if (victim->defunct) continue;
    if (victim==SPRITE->owner) continue;
    if (sprite->x<victim->x+victim->pl) continue;
    if (sprite->x>victim->x+victim->pr) continue;
    if (sprite->y<victim->y+victim->pt) continue;
    if (sprite->y>victim->y+victim->pb) continue;
    if (victim->type->strike(victim,sprite)) {
      if (victim->type==&sprite_type_flower) ; // Pass thru flowers.
      else sprite->defunct=1;
      return;
    }
  }
}

const struct sprite_type sprite_type_laser={
  .name="laser",
  .objlen=sizeof(struct sprite_laser),
  .init=_laser_init,
  .update=_laser_update,
};

void sprite_laser_set_owner(struct sprite *sprite,struct sprite *owner) {
  if (!sprite||(sprite->type!=&sprite_type_laser)) return;
  SPRITE->owner=owner;
}
