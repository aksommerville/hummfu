/* sprite_alien.c
 */
 
#include "game/hummfu.h"

#define HURTDX 3.000
#define HURTDY -2.000
#define WALK_SPEED 2.500

struct sprite_alien {
  struct sprite hdr;
  uint8_t tileid0;
  double targetx;
  double shootclock; // Counts down after shooting, while the muzzle flash is visible and we hold still.
  double hurtclock; // Counts up after injury.
  double shockclock; // Counts down upon seeing the hero. If he's still there when we expire, shoot him.
  double hurtdx;
  double animclock;
  int animframe;
  double stallclock;
  double random_shot_cooldown;
};

#define SPRITE ((struct sprite_alien*)sprite)

static int _alien_init(struct sprite *sprite) {
  SPRITE->tileid0=sprite->tileid;
  sprite->pl=-0.500;
  sprite->pr= 0.500;
  sprite->pt=-0.500;
  sprite->pb= 0.500;
  sprite->solid=1;
  SPRITE->targetx=sprite->x;
  return 0;
}

static void alien_fire_laser(struct sprite *sprite) {
  SPRITE->random_shot_cooldown=1.000;
  sprite->tileid=SPRITE->tileid0;
  SPRITE->shootclock=0.500;
  hummfu_sfx(RID_sound_laser);
  double laserx=sprite->x;
  if (sprite->xform&EGG_XFORM_XREV) laserx-=0.5; else laserx+=0.5;
  double lasery=sprite->y+0.200;
  struct sprite *laser=scene_spawn_sprite(&g.scene,laserx,lasery,RID_sprite_laser,sprite->xform);
}

/* Nonzero if there's something breakable in this rect.
 * We don't want the alien shooting toward boxes randomly, since that cheats the player out of a score-bearing play.
 * It's fine to shoot them when aiming for the hero; that's the hero's own fault.
 */
static int box_exists(double l,double t,double r,double b) {
  struct sprite **p=g.scene.spritev;
  int i=g.scene.spritec;
  for (;i-->0;p++) {
    struct sprite *box=*p;
    if (box->defunct) continue;
    if (box->y<t) continue;
    if (box->y>b) continue;
    if (box->x<l) continue;
    if (box->x>r) continue;
    if (score_type_for_spriteid(box->spriteid)=='b') return 1;
  }
  return 0;
}

static void alien_choose_next_move(struct sprite *sprite) {
  int col=(int)sprite->x;
  int row=(int)sprite->y;
  char optionv[4]; // [lrgs] = left, right, gun, stall
  int optionc=0;
  int colp=col;
  int colc=1,okl=0,okr=0;
  if ((col>=0)&&(col<NS_sys_mapw)&&(row>=0)&&(row<NS_sys_maph-1)) {
    const uint8_t *bodyrow=g.scene.map+row*NS_sys_mapw;
    const uint8_t *footrow=bodyrow+NS_sys_mapw;
    while ((colp>0)&&(g.physics[footrow[colp-1]]==NS_physics_solid)&&(g.physics[bodyrow[colp-1]]==NS_physics_vacant)) { colp--; colc++; }
    while ((colp+colc<NS_sys_mapw)&&(g.physics[footrow[colp+colc]]==NS_physics_solid)&&(g.physics[bodyrow[colp+colc]]==NS_physics_vacant)) colc++;
    okl=col-colp;
    okr=colc-okl;
    if (okl>=2) {
      optionv[optionc++]='l';
    }
    if (okr>=2) {
      optionv[optionc++]='r';
    }
    if (SPRITE->random_shot_cooldown<=0.0) {
      if (sprite->xform&EGG_XFORM_XREV) {
        if ((okl>=1)&&!box_exists(0.0,sprite->y+sprite->pt,sprite->x,sprite->y+sprite->pb)) optionv[optionc++]='g';
      } else {
        if ((okr>=1)&&!box_exists(sprite->x,sprite->y+sprite->pt,NS_sys_mapw,sprite->y+sprite->pb)) optionv[optionc++]='g';
      }
    }
  }
  if (!optionc) optionv[optionc++]='s';
  int optionp=rand()%optionc;
  switch (optionv[optionp]) {
    case 'l': {
        sprite->xform=EGG_XFORM_XREV;
        SPRITE->targetx=(colp+rand()%okl)+0.5;
        if (SPRITE->targetx>=sprite->x) SPRITE->targetx=sprite->x-0.5;
      } break;
    case 'r': {
        sprite->xform=0;
        SPRITE->targetx=(col+rand()%okr)+0.5;
        if (SPRITE->targetx<=sprite->x) SPRITE->targetx=sprite->x+0.5;
      } break;
    case 'g': {
        alien_fire_laser(sprite);
      } break;
    case 's': {
        SPRITE->stallclock=1.000;
        sprite->tileid=SPRITE->tileid0;
      } break;
  }
}

static void _alien_update(struct sprite *sprite,double elapsed) {

  if (SPRITE->random_shot_cooldown>0.0) SPRITE->random_shot_cooldown-=elapsed;

  if (SPRITE->hurtclock>0.0) {
    SPRITE->hurtclock+=elapsed;
         if (SPRITE->hurtclock>0.700) sprite->defunct=1;
    else if (SPRITE->hurtclock>0.600) sprite->tileid=SPRITE->tileid0+0x14;
    else if (SPRITE->hurtclock>0.500) sprite->tileid=SPRITE->tileid0+0x13;
    else if (SPRITE->hurtclock>0.400) sprite->tileid=SPRITE->tileid0+0x12;
    else if (SPRITE->hurtclock>0.300) sprite->tileid=SPRITE->tileid0+0x11;
    else sprite->tileid=SPRITE->tileid0+0x10;
    sprite->x+=SPRITE->hurtdx*elapsed;
    sprite->y+=HURTDY*elapsed;
    return;
  }
  
  if (SPRITE->stallclock>0.0) {
    SPRITE->stallclock-=elapsed;
    return;
  }

  if (SPRITE->shootclock>0.0) {
    if ((SPRITE->shootclock-=elapsed)<=0.0) {
      alien_choose_next_move(sprite);
    } else {
      sprite->tileid=SPRITE->tileid0;
    }
    return;
  }
  
  // If the hero is in my line of sight, stop walking and start shooting.
  // Not bothering to check for intervening walls.
  // Aliens have X-Ray vision of course. But they keep forgetting that their lasers do stop at walls.
  if (g.scene.hero) {
    if ((g.scene.hero->y>=sprite->y+sprite->pt)&&(g.scene.hero->y<=sprite->y+sprite->pb)) {
      int los=0;
      if (sprite->xform&EGG_XFORM_XREV) los=g.scene.hero->x<sprite->x;
      else los=g.scene.hero->x>sprite->x;
      if (SPRITE->shockclock>0.0) {
        if ((SPRITE->shockclock-=elapsed)<=0.0) {
          sprite->tileid=SPRITE->tileid0;
          if (los) {
            alien_fire_laser(sprite);
          }
        }
        return;
      } else if (los) { // "A bird!"
        SPRITE->shockclock=0.250;
        sprite->tileid=SPRITE->tileid0+0x15;
        return;
      }
    }
  }
  
  if ((SPRITE->animclock-=elapsed)<=0.0) {
    SPRITE->animclock+=0.200;
    if (++(SPRITE->animframe)>=4) SPRITE->animframe=0;
    switch (SPRITE->animframe) {
      case 0: case 2: sprite->tileid=SPRITE->tileid0; break;
      case 1: sprite->tileid=SPRITE->tileid0+1; break;
      case 3: sprite->tileid=SPRITE->tileid0+2; break;
    }
  }
  
  if (sprite->xform&EGG_XFORM_XREV) { // walking left
    if (!sprite_move(sprite,-WALK_SPEED*elapsed,0.0)||(sprite->x<=SPRITE->targetx)) alien_choose_next_move(sprite);
  } else { // walking right
    if (!sprite_move(sprite,WALK_SPEED*elapsed,0.0)||(sprite->x>=SPRITE->targetx)) alien_choose_next_move(sprite);
  }
}

static int _alien_strike(struct sprite *sprite,struct sprite *assailant) {
  if (SPRITE->hurtclock>0.0) return 0;
  if (assailant) {
    if (assailant->x<sprite->x) SPRITE->hurtdx=HURTDX;
    else SPRITE->hurtdx=-HURTDX;
  }
  SPRITE->hurtclock=0.100;
  SPRITE->shootclock=0.0;
  return 1;
}

static void _alien_render(struct sprite *sprite) {
  int dstx=(int)(sprite->x*NS_sys_tilesize);
  int dsty=(int)(sprite->y*NS_sys_tilesize);
  graf_tile(&g.graf,dstx,dsty,sprite->tileid,sprite->xform);
  if (SPRITE->shootclock>0.250) { // Not >0; there's a little cooldown period before we start moving again.
    int flashdx=(sprite->xform&EGG_XFORM_XREV)?-NS_sys_tilesize:NS_sys_tilesize;
    graf_tile(&g.graf,dstx+flashdx,dsty,SPRITE->tileid0+3,sprite->xform);
  }
}

const struct sprite_type sprite_type_alien={
  .name="alien",
  .objlen=sizeof(struct sprite_alien),
  .init=_alien_init,
  .update=_alien_update,
  .strike=_alien_strike,
  .render=_alien_render,
};
