#include "game/hummfu.h"

struct sprite_hero {
  struct sprite hdr;
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
  return 0;
}

/* Input.
 */
 
void sprite_hero_input(struct sprite *sprite,int input,int pvinput) {
}

/* Update.
 */
 
static void _hero_update(struct sprite *sprite,double elapsed) {
  // Moving by a constant instead of multiplying by elapsed time helps us avoid stuttering.
  const double d=0.125;
  switch (g.pvinput&(EGG_BTN_LEFT|EGG_BTN_RIGHT)) {
    case EGG_BTN_LEFT: sprite_move(sprite,-d,0.0); break;
    case EGG_BTN_RIGHT: sprite_move(sprite,d,0.0); break;
  }
  switch (g.pvinput&(EGG_BTN_UP|EGG_BTN_DOWN)) {
    case EGG_BTN_UP: sprite_move(sprite,0.0,-d); break;
    case EGG_BTN_DOWN: sprite_move(sprite,0.0,d); break;
  }
}

/* Type definition.
 */
 
const struct sprite_type sprite_type_hero={
  .name="hero",
  .objlen=sizeof(struct sprite_hero),
  .del=_hero_del,
  .init=_hero_init,
  .update=_hero_update,
};
