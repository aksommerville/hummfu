#include "game/hummfu.h"

struct sprite_box {
  struct sprite hdr;
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

/* Type definition.
 */
 
const struct sprite_type sprite_type_box={
  .name="box",
  .objlen=sizeof(struct sprite_box),
  .init=_box_init,
};
