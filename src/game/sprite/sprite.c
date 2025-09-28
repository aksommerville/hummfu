#include "game/hummfu.h"

/* Delete.
 */
 
void sprite_del(struct sprite *sprite) {
  if (!sprite) return;
  if (sprite->type->del) sprite->type->del(sprite);
  free(sprite);
}

/* New.
 */

struct sprite *sprite_new(const struct sprite_type *type,double x,double y,uint32_t arg,const void *serial,int serialc) {
  if (!type||(type->objlen<(int)sizeof(struct sprite))) return 0;
  struct sprite *sprite=calloc(1,type->objlen);
  if (!sprite) return 0;
  sprite->type=type;
  sprite->x=x;
  sprite->y=y;
  sprite->arg=arg;
  sprite->serial=serial;
  sprite->serialc=serialc;
  
  struct cmdlist_reader reader;
  if (sprite_reader_init(&reader,serial,serialc)>=0) {
    struct cmdlist_entry cmd;
    while (cmdlist_reader_next(&cmd,&reader)>0) {
      switch (cmd.opcode) {
      
        case CMD_sprite_tile: {
            sprite->tileid=cmd.arg[0];
            sprite->xform=cmd.arg[1];
          } break;
          
      }
    }
  }
  
  if (type->init&&(type->init(sprite)<0)) {
    sprite_del(sprite);
    return 0;
  }
  return sprite;
}

/* Sprite type by id.
 */
 
const struct sprite_type *sprite_type_by_id(int sprtype) {
  switch (sprtype) {
    #define _(tag) case NS_sprtype_##tag: return &sprite_type_##tag;
    FOR_EACH_SPRTYPE
    #undef _
  }
  return 0;
}

/* Registry of sprite resource ids and how they behave re scorekeeping.
 */
 
char score_type_for_spriteid(int spriteid) {
  /* A smart implementation would store these facts in the sprite resource.
   * Totally doable, but it turns out to be inconvenient during startup, when we tabulate how many killable and breakable sprites exist.
   * This is a jam game, don't overthink it.
   */
  if (spriteid>10) {
    fprintf(stderr,"%s:%d: sprite:%d not known. If it's killable or breakable, please update source.\n",__FILE__,__LINE__,spriteid);
  }
  switch (spriteid) {
    case RID_sprite_box:
      return 'b';
    case RID_sprite_alien:
    case RID_sprite_bear:
      return 'k';
  }
  return 0;
}
