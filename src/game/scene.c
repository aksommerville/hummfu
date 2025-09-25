#include "hummfu.h"

/* Update.
 */
 
void scene_update(struct scene *scene,double elapsed,int input,int pvinput) {

  if (input!=pvinput) {
    if (scene->hero) sprite_hero_input(scene->hero,input,pvinput);
  }

  int i=0;
  for (;i<scene->spritec;i++) {
    struct sprite *sprite=scene->spritev[i];
    if (sprite->defunct) continue;
    if (sprite->type->update) sprite->type->update(sprite,elapsed);
  }
  
  // Remove defunct sprites.
  for (i=scene->spritec;i-->0;) {
    struct sprite *sprite=scene->spritev[i];
    if (!sprite->defunct) continue;
    if (sprite==scene->hero) scene->hero=0;
    scene->spritec--;
    memmove(scene->spritev+i,scene->spritev+i+1,sizeof(void*)*(scene->spritec-i));
    sprite_del(sprite);
  }
}

/* Render.
 */
 
void scene_render(struct scene *scene) {
  graf_fill_rect(&g.graf,0,0,FBW,FBH,0x808080ff);
  graf_set_input(&g.graf,scene->bgtexid);
  graf_decal(&g.graf,0,0,0,0,FBW,FBH);
  graf_set_input(&g.graf,g.texid_sprites);
  int i=0;
  for (;i<scene->spritec;i++) {
    struct sprite *sprite=scene->spritev[i];
    if (sprite->defunct) continue;
    int dstx=(int)(sprite->x*NS_sys_tilesize);
    int dsty=(int)(sprite->y*NS_sys_tilesize);
    graf_tile(&g.graf,dstx,dsty,sprite->tileid,sprite->xform);
  }
}

/* Render static map to bgtexid.
 */
 
static int scene_render_bgtex(struct scene *scene,const struct map_res *res) {
  if (scene->bgtexid<1) {
    if ((scene->bgtexid=egg_texture_new())<1) return -1;
    if (egg_texture_load_raw(scene->bgtexid,FBW,FBH,FBW<<2,0,0)<0) return -1;
  }
  struct egg_render_uniform un={
    .mode=EGG_RENDER_TILE,
    .dsttexid=scene->bgtexid,
    .srctexid=g.texid_terrain, // Regardless of what (res) says; there's just one terrain image.
    .tint=0,
    .alpha=0xff,
  };
  struct egg_render_tile vtxv[NS_sys_mapw*NS_sys_maph];
  struct egg_render_tile *vtx=vtxv;
  const uint8_t *tileid=res->v;
  int yi=NS_sys_maph,y=NS_sys_tilesize>>1;
  for (;yi-->0;y+=NS_sys_tilesize) {
    int xi=NS_sys_mapw,x=NS_sys_tilesize>>1;
    for (;xi-->0;vtx++,tileid++,x+=NS_sys_tilesize) {
      vtx->x=x;
      vtx->y=y;
      vtx->tileid=*tileid;
      vtx->xform=0;
    }
  }
  egg_render(&un,vtxv,sizeof(vtxv));
  return 0;
}

/* Spawn sprite.
 */
 
struct sprite *scene_spawn_sprite(struct scene *scene,double x,double y,int spriteid,uint32_t arg) {
  const void *serial=0;
  int serialc=hummfu_get_res(&serial,EGG_TID_sprite,spriteid);
  
  // Pre-run the command list to acquire sprtype.
  // Most processing of this list will happen in sprite_new(), not our problem.
  struct cmdlist_reader reader;
  if (sprite_reader_init(&reader,serial,serialc)<0) {
    fprintf(stderr,"sprite:%d missing or malformed\n",spriteid);
    return 0;
  }
  struct cmdlist_entry cmd;
  int sprtype=0;
  while (cmdlist_reader_next(&cmd,&reader)>0) {
    if (cmd.opcode==CMD_sprite_type) {
      sprtype=(cmd.arg[0]<<8)|cmd.arg[1];
    }
  }
  const struct sprite_type *type=sprite_type_by_id(sprtype);
  if (!type) {
    fprintf(stderr,"sprite:%d requests sprtype %d, not found\n",spriteid,sprtype);
    return 0;
  }
  
  if (scene->spritec>=scene->spritea) {
    int na=scene->spritea+32;
    if (na>INT_MAX/sizeof(void*)) return 0;
    void *nv=realloc(scene->spritev,sizeof(void*)*na);
    if (!nv) return 0;
    scene->spritev=nv;
    scene->spritea=na;
  }
  
  struct sprite *sprite=sprite_new(type,x,y,arg,serial,serialc);
  if (!sprite) return 0;
  scene->spritev[scene->spritec++]=sprite;
  if (sprite->type==&sprite_type_hero) scene->hero=sprite;
  return sprite;
}

/* Begin.
 */

int scene_begin(struct scene *scene,int mapid) {
  
  // Acquire the resource and render background.
  const void *serial=0;
  int serialc=hummfu_get_res(&serial,EGG_TID_map,mapid);
  struct map_res res={0};
  if (map_res_decode(&res,serial,serialc)<0) return -1;
  if ((res.w!=NS_sys_mapw)||(res.h!=NS_sys_maph)) {
    fprintf(stderr,"map:%d dimensions %dx%d, expected %dx%d\n",mapid,res.w,res.h,NS_sys_mapw,NS_sys_maph);
    return -1;
  }
  if (scene_render_bgtex(scene,&res)<0) return -1;
  scene->mapid=mapid;
  
  // Drop volatile things.
  scene->hero=0;
  while (scene->spritec>0) {
    scene->spritec--;
    sprite_del(scene->spritev[scene->spritec]);
  }
  
  struct cmdlist_reader reader;
  if (cmdlist_reader_init(&reader,res.cmd,res.cmdc)<0) return -1;
  struct cmdlist_entry cmd;
  while (cmdlist_reader_next(&cmd,&reader)>0) {
    switch (cmd.opcode) {
      case CMD_map_sprite: { /* u16:position, u16:spriteid, u32:arg */
          double x=cmd.arg[0]+0.5;
          double y=cmd.arg[1]+0.5;
          int spriteid=(cmd.arg[2]<<8)|cmd.arg[3];
          uint32_t arg=(cmd.arg[4]<<24)|(cmd.arg[5]<<16)|(cmd.arg[6]<<8)|cmd.arg[7];
          scene_spawn_sprite(scene,x,y,spriteid,arg);
        } break;
    }
  }
  
  g.hello.active=0;
  g.gameover.active=0;
  scene->active=1;
  return 0;
}
