#include "game/hummfu.h"

#define BOX_DAMAGE_SPEED 6.000 /* m/s */

struct sprite_box {
  struct sprite hdr;
  int damage;
  double damageclock;
  double damagedx;
};

#define SPRITE ((struct sprite_box*)sprite)

/* Init.
 */
 
static int _box_init(struct sprite *sprite) {
  sprite->solid=1;
  sprite->pl=-0.500;
  sprite->pr= 0.500;
  sprite->pt=-0.500;
  sprite->pb= 0.500;
  return 0;
}

/* Update.
 */
 
static void _box_update(struct sprite *sprite,double elapsed) {
  if (SPRITE->damage) {
    SPRITE->damageclock+=elapsed;
         if (SPRITE->damageclock>=0.300) sprite->defunct=1;
    else if (SPRITE->damageclock>=0.200) sprite->tileid=0x13;
    else if (SPRITE->damageclock>=0.100) sprite->tileid=0x12;
    else sprite->tileid=0x11;
    sprite->x+=SPRITE->damagedx*elapsed;
  } else {
    sprite_move(sprite,0.0,6.000*elapsed);
  }
}

/* Get whacked.
 */
 
static int _box_strike(struct sprite *sprite,struct sprite *assailant) {
  if (SPRITE->damage) return 0;
  if (assailant) {
    if (assailant->x<sprite->x) {
      SPRITE->damagedx=BOX_DAMAGE_SPEED;
    } else {
      SPRITE->damagedx=-BOX_DAMAGE_SPEED;
    }
  }
  SPRITE->damage=1;
  return 1;
}

/* Type definition.
 */
 
const struct sprite_type sprite_type_box={
  .name="box",
  .objlen=sizeof(struct sprite_box),
  .init=_box_init,
  .update=_box_update,
  .strike=_box_strike,
};
