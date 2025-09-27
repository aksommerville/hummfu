/* sprite_soulball.c
 * arg: u16=reserved, u8=wave, u8=angle
 */
 
#include "game/hummfu.h"

#define FRAME_TIME 0.100
#define SPEED 4.000
#define SPEED_EXTRA 2.000
#define GRAVITY 9.000
#define WAVE_DELAY 0.200

struct sprite_soulball {
  struct sprite hdr;
  int animframe;
  double waitclock;
  double animclock;
  double dx,dy;
};

#define SPRITE ((struct sprite_soulball*)sprite)

static int _soulball_init(struct sprite *sprite) {
  sprite->tileid=0x1a;
  SPRITE->animframe=rand()&7;
  SPRITE->animclock=((rand()&0xffff)*FRAME_TIME)/65535.0;
  
  uint8_t wave=sprite->arg>>8;
  SPRITE->waitclock=wave*WAVE_DELAY;
  
  uint8_t angle=sprite->arg;
  double t=(angle*M_PI*2.0)/256.0;
  double speed=SPEED+((rand()&0xffff)*SPEED_EXTRA)/65535.0;
  SPRITE->dx=cos(t)*speed;
  SPRITE->dy=-sin(t)*speed;
  
  return 0;
}

static void _soulball_update(struct sprite *sprite,double elapsed) {
  if (SPRITE->waitclock>0.0) {
    SPRITE->waitclock-=elapsed;
    return;
  }
  if ((SPRITE->animclock-=elapsed)<=0.0) {
    SPRITE->animclock+=FRAME_TIME;
    if (++(SPRITE->animframe)>=8) SPRITE->animframe=0;
    switch (SPRITE->animframe) {
      case 0: sprite->tileid=0x15; break;
      case 1: sprite->tileid=0x16; break;
      case 2: sprite->tileid=0x17; break;
      case 3: sprite->tileid=0x18; break;
      case 4: sprite->tileid=0x19; break;
      case 5: sprite->tileid=0x18; break;
      case 6: sprite->tileid=0x17; break;
      case 7: sprite->tileid=0x16; break;
    }
  }
  SPRITE->dy+=GRAVITY*elapsed;
  sprite->x+=SPRITE->dx*elapsed;
  sprite->y+=SPRITE->dy*elapsed;
}

const struct sprite_type sprite_type_soulball={
  .name="soulball",
  .objlen=sizeof(struct sprite_soulball),
  .init=_soulball_init,
  .update=_soulball_update,
};
