#include "game/hummfu.h"

#define GRAVITY_START 0.000 /* m/s */
#define GRAVITY_LIMIT 8.000 /* m/s */
#define GRAVITY_ACCEL 20.000 /* m/s**2 */

struct sprite_hero {
  struct sprite hdr;
  double gravity;
  int seated;
  int flying;
  int walking; // Moving horizontally, whether in air or on ground.
  double wanimclock;
  int wanimframe;
  double fanimclock;
  int fanimframe;
  double swingclock; // Counts up. Set ever so slighly positive to start the animation. All effects happen at the start.
  int qx,qy;
  int input,pvinput;
};

#define SPRITE ((struct sprite_hero*)sprite)

/* Delete.
 */
 
static void _hero_del(struct sprite *sprite) {
}

/* Init.
 */
 
static int _hero_init(struct sprite *sprite) {
  sprite->pl=-0.300;
  sprite->pr= 0.200;
  sprite->pt=-0.250;
  sprite->pb= 0.300;
  sprite->solid=1;
  SPRITE->gravity=GRAVITY_START;
  SPRITE->seated=1;
  SPRITE->qx=-1;
  SPRITE->qy=-1;
  sprite->y+=0.500-sprite->pb;
  return 0;
}

/* Die.
 */
 
static void hero_die(struct sprite *sprite) {
  if (g.scene.winclock>0.0) return;
  egg_play_sound(RID_sound_deadbird,0.5,0.0);
  scene_begin_death(&g.scene);
  sprite->defunct=1;
  
  scene_spawn_sprite(&g.scene,sprite->x,sprite->y,RID_sprite_soulball,0x00000000);
  scene_spawn_sprite(&g.scene,sprite->x,sprite->y,RID_sprite_soulball,0x00000040);
  scene_spawn_sprite(&g.scene,sprite->x,sprite->y,RID_sprite_soulball,0x00000080);
  scene_spawn_sprite(&g.scene,sprite->x,sprite->y,RID_sprite_soulball,0x000000c0);
  scene_spawn_sprite(&g.scene,sprite->x,sprite->y,RID_sprite_soulball,0x00000120);
  scene_spawn_sprite(&g.scene,sprite->x,sprite->y,RID_sprite_soulball,0x00000160);
  scene_spawn_sprite(&g.scene,sprite->x,sprite->y,RID_sprite_soulball,0x000001a0);
  scene_spawn_sprite(&g.scene,sprite->x,sprite->y,RID_sprite_soulball,0x000001e0);
  scene_spawn_sprite(&g.scene,sprite->x,sprite->y,RID_sprite_soulball,0x00000210);
  scene_spawn_sprite(&g.scene,sprite->x,sprite->y,RID_sprite_soulball,0x00000230);
  scene_spawn_sprite(&g.scene,sprite->x,sprite->y,RID_sprite_soulball,0x00000250);
  scene_spawn_sprite(&g.scene,sprite->x,sprite->y,RID_sprite_soulball,0x00000270);
  scene_spawn_sprite(&g.scene,sprite->x,sprite->y,RID_sprite_soulball,0x00000290);
  scene_spawn_sprite(&g.scene,sprite->x,sprite->y,RID_sprite_soulball,0x000002b0);
  scene_spawn_sprite(&g.scene,sprite->x,sprite->y,RID_sprite_soulball,0x000002d0);
  scene_spawn_sprite(&g.scene,sprite->x,sprite->y,RID_sprite_soulball,0x000002f0);
}

/* Swing.
 */
 
static int hero_can_swing(const struct sprite *sprite) {
  //if (SPRITE->flying) return 0;
  if (SPRITE->swingclock>0.0) return 0;
  return 1;
}

static int hero_strike_foes(struct sprite *sprite) {
  const double width=0.750; // Extension from sprite's forward physical edge.
  double t=sprite->y+sprite->pt;
  double b=sprite->y+sprite->pb;
  double l,r;
  if (sprite->xform&EGG_XFORM_XREV) { // Facing left.
    r=sprite->x;
    l=r-width;
  } else {
    l=sprite->x;
    r=l+width;
  }
  int victimc=0;
  int i=g.scene.spritec;
  struct sprite **p=g.scene.spritev;
  for (;i-->0;p++) {
    struct sprite *victim=*p;
    if (!victim->type->strike) continue;
    if (victim->defunct) continue;
    if (victim==sprite) continue;
    double vl=victim->x+victim->pl; if (vl>=r) continue;
    double vr=victim->x+victim->pr; if (vr<=l) continue;
    double vt=victim->y+victim->pt; if (vt>=b) continue;
    double vb=victim->y+victim->pb; if (vb<=t) continue;
    if (victim->type->strike(victim,sprite)) {
      victimc++;
    }
  }
  return victimc;
}

/* Input.
 */
 
void sprite_hero_input(struct sprite *sprite,int input,int pvinput) {
  SPRITE->input=input;
  SPRITE->pvinput=pvinput;
  if ((input&EGG_BTN_WEST)&&!(pvinput&EGG_BTN_WEST)) {
    if (hero_can_swing(sprite)) {
      SPRITE->swingclock=0.001; // Start animation.
      scene_highlight_strike(&g.scene,sprite->x+((sprite->xform&EGG_XFORM_XREV)?-0.250:0.250),sprite->y);
      if (hero_strike_foes(sprite)) {
        egg_play_sound(RID_sound_swinghit,0.5,0.0);
      } else {
        egg_play_sound(RID_sound_swingmiss,0.5,0.0);
      }
    } else {
      egg_play_sound(RID_sound_swingreject,0.5,0.0);
    }
  }
}

/* Walking (or flying, whatever, Horizontal Motion).
 * Exactly one of these must be called each update.
 */
 
static void hero_walk(struct sprite *sprite,double d,double elapsed) {
  // Moving by a constant instead of multiplying by elapsed time helps us avoid stuttering.
  const double speed=0.125;
  if (sprite_move(sprite,d*speed,0.0)) {
    // Moved (possible partial collision).
  } else {
    // Collision.
  }
  SPRITE->walking=1;
  // Natural orientation is rightward.
  if (d<0.0) {
    sprite->xform=EGG_XFORM_XREV;
  } else {
    sprite->xform=0;
  }
  if ((SPRITE->wanimclock-=elapsed)<=0.0) {
    SPRITE->wanimclock+=0.200;
    if (++(SPRITE->wanimframe)>=4) SPRITE->wanimframe=0;
  }
}

static void hero_walk_none(struct sprite *sprite,double elapsed) {
  SPRITE->walking=0;
  SPRITE->wanimclock=0.0;
  SPRITE->wanimframe=0;
}

/* Flight or gravity.
 * As with walk, call one per cycle.
 */
 
static void hero_fly(struct sprite *sprite,double elapsed) {
  SPRITE->seated=0;
  SPRITE->gravity=GRAVITY_START;
  sprite_move(sprite,0.0,-0.125);
  SPRITE->flying=1;
  if ((SPRITE->fanimclock-=elapsed)<=0.0) {
    SPRITE->fanimclock+=0.050;
    if (++(SPRITE->fanimframe)>=6) SPRITE->fanimframe=0;
  }
}

static void hero_fly_none(struct sprite *sprite,double elapsed) {
  SPRITE->flying=0;
  SPRITE->gravity+=GRAVITY_ACCEL*elapsed;
  if (SPRITE->gravity>=GRAVITY_LIMIT) SPRITE->gravity=GRAVITY_LIMIT;
  if (!sprite_move(sprite,0.0,SPRITE->gravity*elapsed)) {
    // Hit floor.
    SPRITE->seated=1;
    SPRITE->gravity=GRAVITY_START;
  } else {
    // Falling.
    SPRITE->seated=0;
  }
  SPRITE->fanimclock=0.0;
  SPRITE->fanimframe=0;
}

/* Update animation, select face.
 */
 
static void hero_animate(struct sprite *sprite,double elapsed) {

  if (SPRITE->swingclock>0.0) {
    SPRITE->swingclock+=elapsed;
    if (SPRITE->swingclock>0.240) {
      SPRITE->swingclock=0.0;
    } else if (SPRITE->swingclock>0.160) {
      sprite->tileid=0x05;
    } else if (SPRITE->swingclock>0.080) {
      sprite->tileid=0x04;
    } else {
      sprite->tileid=0x03;
    }
  
  } else if (SPRITE->flying) {
    switch (SPRITE->fanimframe) {
      case 0: sprite->tileid=0x06; break;
      case 1: sprite->tileid=0x07; break;
      case 2: sprite->tileid=0x08; break;
      case 3: sprite->tileid=0x09; break;
      case 4: sprite->tileid=0x08; break;
      case 5: sprite->tileid=0x07; break;
    }
  
  } else if (SPRITE->walking) {
    switch (SPRITE->wanimframe) {
      case 1: sprite->tileid=0x01; break;
      case 3: sprite->tileid=0x02; break;
      default: sprite->tileid=0x00; break;
    }
  
  } else {
    sprite->tileid=0x00;
  }
}

/* Update quantized position, and trigger actions if we change cell.
 */
 
static void hero_check_qpos(struct sprite *sprite) {
  int x=(int)sprite->x;
  int y=(int)sprite->y;
  if ((x==SPRITE->qx)&&(y==SPRITE->qy)) return;
  SPRITE->qx=x;
  SPRITE->qy=y;
  if ((x>=0)&&(y>=0)&&(x<NS_sys_mapw)&&(y<NS_sys_maph)) {
    uint8_t physics=g.physics[g.scene.map[y*NS_sys_mapw+x]];
    switch (physics) {
      case NS_physics_hazard: hero_die(sprite); break;
    }
  }
}

/* Update.
 */
 
static void _hero_update(struct sprite *sprite,double elapsed) {
  switch (SPRITE->input&(EGG_BTN_LEFT|EGG_BTN_RIGHT)) {
    case EGG_BTN_LEFT: hero_walk(sprite,-1.0,elapsed); break;
    case EGG_BTN_RIGHT: hero_walk(sprite,1.0,elapsed); break;
    default: hero_walk_none(sprite,elapsed); break;
  }
  if (SPRITE->input&EGG_BTN_SOUTH) {
    hero_fly(sprite,elapsed);
  } else {
    hero_fly_none(sprite,elapsed);
  }
  hero_check_qpos(sprite);
  hero_animate(sprite,elapsed);
}

/* Get struck.
 */
 
static int _hero_strike(struct sprite *sprite,struct sprite *assailant) {
  hero_die(sprite);
  return 1;
}

/* Type definition.
 */
 
const struct sprite_type sprite_type_hero={
  .name="hero",
  .objlen=sizeof(struct sprite_hero),
  .del=_hero_del,
  .init=_hero_init,
  .update=_hero_update,
  .strike=_hero_strike,
};
