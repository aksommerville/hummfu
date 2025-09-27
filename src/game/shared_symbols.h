/* shared_symbols.h
 * This file is consumed by eggdev and editor, in addition to compiling in with the game.
 */

#ifndef SHARED_SYMBOLS_H
#define SHARED_SYMBOLS_H

#define NS_sys_tilesize 16
#define NS_sys_mapw 20
#define NS_sys_maph 11

#define CMD_map_image     0x20 /* u16:imageid */
#define CMD_map_sprite    0x61 /* u16:position, u16:spriteid, u32:arg */

#define CMD_sprite_image 0x20 /* u16:imageid */
#define CMD_sprite_tile  0x21 /* u8:tileid, u8:xform */
#define CMD_sprite_type  0x22 /* u16:sprtype */

#define NS_tilesheet_physics 1
#define NS_tilesheet_family 0
#define NS_tilesheet_neighbors 0
#define NS_tilesheet_weight 0

#define NS_physics_vacant 0
#define NS_physics_solid 1
#define NS_physics_hazard 2

#define NS_sprtype_dummy 0
#define NS_sprtype_hero 1
#define NS_sprtype_box 2 /* Solid and destroyable. */
#define NS_sprtype_soulball 3
#define NS_sprtype_flower 4
#define NS_sprtype_toast 5
#define NS_sprtype_title 6
#define NS_sprtype_alien 7
#define NS_sprtype_laser 8

#define FOR_EACH_SPRTYPE \
  _(dummy) \
  _(hero) \
  _(box) \
  _(soulball) \
  _(flower) \
  _(toast) \
  _(title) \
  _(alien) \
  _(laser)

#endif
