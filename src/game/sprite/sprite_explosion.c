/* sprite_explosion.c
 * A big honking explosion a la Blaster Master.
 * 4x3 tiles. Our nominal position is a half meter up from bottom center.
 * We produce the sound effect too.
 */
 
#include "game/hummfu.h"

struct sprite_explosion {
  struct sprite hdr;
  double ttl;
  double animclock;
  int animframe;
};

#define SPRITE ((struct sprite_explosion*)sprite)

static int _explosion_init(struct sprite *sprite) {
  egg_play_sound(RID_sound_explode,0.5,0.0);
  SPRITE->ttl=2.0;
  return 0;
}

static void _explosion_update(struct sprite *sprite,double elapsed) {
  if ((SPRITE->ttl-=elapsed)<=0.0) sprite->defunct=1;
  if ((SPRITE->animclock-=elapsed)<=0.0) {
    SPRITE->animclock+=0.200;
    if (++(SPRITE->animframe)>=4) SPRITE->animframe=0;
  }
}

static void _explosion_render(struct sprite *sprite) {
  int dstx=(int)(sprite->x*NS_sys_tilesize);
  int dsty=(int)(sprite->y*NS_sys_tilesize);
  dstx-=NS_sys_tilesize+(NS_sys_tilesize>>1); // (dstx,dsty) refer to the top left of our 4x3 grid.
  dsty-=NS_sys_tilesize<<1;
  dsty+=2; // Cheating downward to taste.
  #define TILE(dx,dy,dt) graf_tile(&g.graf,dstx+((dx)*NS_sys_tilesize),dsty+((dy)*NS_sys_tilesize),tileid+(dt),sprite->xform);

  // The big explosion picture only shows up for a very brief interval.
  if (SPRITE->ttl>1.825) {
    uint8_t tileid=0x60;
    TILE(0,0,0x00)
    TILE(1,0,0x01)
    TILE(2,0,0x02)
    TILE(3,0,0x03)
    TILE(0,1,0x10)
    TILE(1,1,0x11)
    TILE(2,1,0x12)
    TILE(3,1,0x13)
    TILE(0,2,0x20)
    TILE(1,2,0x21)
    TILE(2,2,0x22)
    TILE(3,2,0x23)
    
  // Then for most of our life, it's just smoke in the lower row.
  } else {
    uint8_t tileid=0x64+SPRITE->animframe;
    TILE(0,2,0)
    TILE(1,2,0)
    TILE(2,2,0)
    TILE(3,2,0)
  }
}

const struct sprite_type sprite_type_explosion={
  .name="explosion",
  .objlen=sizeof(struct sprite_explosion),
  .init=_explosion_init,
  .update=_explosion_update,
  .render=_explosion_render,
};
