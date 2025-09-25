/* sprite.h
 */
 
#ifndef SPRITE_H
#define SPRITE_H

struct sprite;
struct sprite_type;

struct sprite {
  const struct sprite_type *type;
  double x,y; // Center in meters.
  uint8_t tileid,xform;
  uint32_t arg;
  const void *serial; // The entire resource we spawned from.
  int serialc;
  int defunct; // Aside from scene, this is the one and only way to delete a sprite.
};

struct sprite_type {
  const char *name;
  int objlen;
  void (*del)(struct sprite *sprite);
  
  /* Generic parts of (sprite) are already populated.
   * You may return <0 to gracefully abort construction; the game will proceed anyway.
   * DO NOT spawn new sprites from init.
   */
  int (*init)(struct sprite *sprite);
  
  void (*update)(struct sprite *sprite,double elapsed);
};

void sprite_del(struct sprite *sprite);

struct sprite *sprite_new(const struct sprite_type *type,double x,double y,uint32_t arg,const void *serial,int serialc);

#define _(tag) extern const struct sprite_type sprite_type_##tag;
FOR_EACH_SPRTYPE
#undef _

const struct sprite_type *sprite_type_by_id(int sprtype);

void sprite_hero_input(struct sprite *sprite,int input,int pvinput);

#endif
