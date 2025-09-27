/* sprite_toast.c
 */
 
#include "game/hummfu.h"

#define SPEED 1.000

struct sprite_toast {
  struct sprite hdr;
  double ttl;
};

#define SPRITE ((struct sprite_toast*)sprite)

static int _toast_init(struct sprite *sprite) {
  SPRITE->ttl=1.000;
  return 0;
}

static void _toast_update(struct sprite *sprite,double elapsed) {
  if ((SPRITE->ttl-=elapsed)<0.0) sprite->defunct=1;
  sprite->y-=SPEED*elapsed;
}

const struct sprite_type sprite_type_toast={
  .name="toast",
  .objlen=sizeof(struct sprite_toast),
  .init=_toast_init,
  .update=_toast_update,
};
