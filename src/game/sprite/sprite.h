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
  
  int solid; // Nonzero to block other participating sprites.
  double pl,pr,pt,pb; // Hitbox, distance to each edge from (x,y). (pl,pt) are normally negative.
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
  
  /* If implemented, nothing generic happens.
   * If you can be rendered as a single tile from image:sprites, don't use this!
   */
  void (*render)(struct sprite *sprite);
  
  void (*update)(struct sprite *sprite,double elapsed);
  
  /* Return nonzero if the strike is successful, zero if inert eg already dead.
   */
  int (*strike)(struct sprite *sprite,struct sprite *assailant);
};

void sprite_del(struct sprite *sprite);

struct sprite *sprite_new(const struct sprite_type *type,double x,double y,uint32_t arg,const void *serial,int serialc);

/* Sprite types.
 ***************************************************************************************/

#define _(tag) extern const struct sprite_type sprite_type_##tag;
FOR_EACH_SPRTYPE
#undef _

const struct sprite_type *sprite_type_by_id(int sprtype);

void sprite_hero_input(struct sprite *sprite,int input,int pvinput);

/* Physics.
 ***************************************************************************************/

/* For physics-enabled sprites, prefer using this to move.
 * Resolves all collisions immediately.
 * Returns nonzero if moved at all.
 */
int sprite_move(struct sprite *sprite,double dx,double dy);

/* Adjust to the nearest legal position, no preference of direction.
 * Returns nonzero if moved.
 */
int sprite_rectify(struct sprite *sprite);

/* Nonzero if we're in an unstable position, ie sprite_rectify would move it.
 */
int sprite_test_position(const struct sprite *sprite);

#endif
