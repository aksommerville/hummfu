/* sprite_bear.c
 */
 
#include "game/hummfu.h"

#define WALK_SPEED 2.500

struct sprite_bear {
  struct sprite hdr;
  double animclock;
  int animframe;
  double turnclock;
  int eating;
};

#define SPRITE ((struct sprite_bear*)sprite)

static int _bear_init(struct sprite *sprite) {
  sprite->solid=1;
  sprite->pl=-1.500;
  sprite->pr= 1.500;
  sprite->pt=-1.250;
  sprite->pb= 0.500;
  return 0;
}

static void bear_eat_bird(struct sprite *sprite,struct sprite *bird) {
  if (!bird||!bird->type->strike) return;
  if (!bird->type->strike(bird,sprite)) return;
  SPRITE->eating=1;
  SPRITE->animclock=0;
  SPRITE->animframe=0;
}

static void _bear_update(struct sprite *sprite,double elapsed) {

  if (SPRITE->turnclock>0.0) {
    if ((SPRITE->turnclock-=elapsed)<=0.0) {
      sprite->xform^=EGG_XFORM_XREV;
    }
    SPRITE->animframe=0;
    SPRITE->animclock=0.0;
    return;
  }
  
  if ((SPRITE->animclock-=elapsed)<0.0) {
    SPRITE->animclock+=0.250;
    if (++(SPRITE->animframe)>=4) SPRITE->animframe=0;
  }
  
  if (SPRITE->eating) return;
  
  double dx=WALK_SPEED*elapsed;
  if (sprite->xform&EGG_XFORM_XREV) dx=-dx;
  if (!sprite_move(sprite,dx,0.0)) {
    SPRITE->turnclock=0.500;
  }
  
  // Is there a tasty, tasty bird before us?
  if (g.scene.hero) {
    if ((g.scene.hero->y>sprite->y+sprite->pt)&&(g.scene.hero->y<sprite->y+sprite->pb)) {
      double dx=g.scene.hero->x-sprite->x;
      if (sprite->xform&EGG_XFORM_XREV) {
        if ((dx>-2.0)&&(dx<0.0)) bear_eat_bird(sprite,g.scene.hero);
      } else {
        if ((dx>0.0)&&(dx<2.0)) bear_eat_bird(sprite,g.scene.hero);
      }
    }
  }
}

static int _bear_strike(struct sprite *sprite,struct sprite *assailant) {
  if (!assailant) return 0;
  
  // Only the butt is vulnerable.
  if (sprite->xform==EGG_XFORM_XREV) {
    if (assailant->x<sprite->x) return 0;
  } else {
    if (assailant->x>sprite->x) return 0;
  }
  
  scene_spawn_sprite(&g.scene,sprite->x,sprite->y,RID_sprite_explosion,0);
  sprite->defunct=1;
  return 1;
}

// The sprite's nominal position is the center of the bottom-middle tile.
static void _bear_render(struct sprite *sprite) {
  int dstx=(int)(sprite->x*NS_sys_tilesize);
  int dsty=(int)(sprite->y*NS_sys_tilesize);
  uint8_t tileid=0x40;
  if (SPRITE->eating) {
    if (SPRITE->animframe&1) tileid+=12;
    else tileid+=9;
  } else {
    switch (SPRITE->animframe) {
      case 0: case 2: break;
      case 1: tileid+=3; break;
      case 3: tileid+=6; break;
    }
  }
  #define TILE(dx,dy,dt) graf_tile(&g.graf,dstx+((dx)*NS_sys_tilesize),dsty+((dy)*NS_sys_tilesize),tileid+(dt),sprite->xform);
  if (sprite->xform&EGG_XFORM_XREV) {
    TILE(-1,-1,0x02)
    TILE( 0,-1,0x01)
    TILE( 1,-1,0x00)
    TILE(-1, 0,0x12)
    TILE( 0, 0,0x11)
    TILE( 1, 0,0x10)
  } else {
    TILE(-1,-1,0x00)
    TILE( 0,-1,0x01)
    TILE( 1,-1,0x02)
    TILE(-1, 0,0x10)
    TILE( 0, 0,0x11)
    TILE( 1, 0,0x12)
  }
  #undef TILE
}

const struct sprite_type sprite_type_bear={
  .name="bear",
  .objlen=sizeof(struct sprite_bear),
  .init=_bear_init,
  .update=_bear_update,
  .strike=_bear_strike,
  .render=_bear_render,
};
