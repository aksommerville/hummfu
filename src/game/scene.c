#include "hummfu.h"

#define STRIKE_HIGHLIGHT_TIME 0.100

/* Update.
 */
 
void scene_update(struct scene *scene,double elapsed,int input,int pvinput) {

  // When input_blackout in play, ignore those bits until they go false.
  // This is how you can press Fly to restart after dying, and the new bird doesn't spawn flying.
  if (scene->input_blackout) {
    int bit=0x8000;
    for (;bit;bit>>=1) {
      if ((scene->input_blackout&bit)&&!(input&bit)) {
        scene->input_blackout&=~bit;
      }
    }
    input=pvinput=0;
  }

  if (scene->strikeclock>0.0) {
    scene->strikeclock-=elapsed;
  }
  if (scene->deathclock>0.0) {
    scene->deathclock+=elapsed;
  }

  if (input!=pvinput) {
    if (scene->hero) sprite_hero_input(scene->hero,input,pvinput);
    else if (scene->deathclock>=0.500) {
      if ((input&EGG_BTN_SOUTH)&&!(pvinput&EGG_BTN_SOUTH)) {
        scene_begin(scene,scene->mapid);
      }
    }
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

/* Blend RGBA.
 * It's private so we can assume alpha is always one.
 */
 
static uint32_t rgba_blend(uint32_t a,uint32_t z,double p) {
  if (p<=0.0) return a;
  if (p>=1.0) return z;
  double inv=1.0-p;
  uint8_t ar=a>>24,ag=a>>16,ab=a>>8;
  uint8_t zr=z>>24,zg=z>>16,zb=z>>8;
  int r=(int)(ar*inv+zr*p); if (r<0) r=0; else if (r>0xff) r=0xff;
  int g=(int)(ag*inv+zg*p); if (g<0) g=0; else if (g>0xff) g=0xff;
  int b=(int)(ab*inv+zb*p); if (b<0) b=0; else if (b>0xff) b=0xff;
  return (r<<24)|(g<<16)|(b<<8)|0xff;
}

/* Render.
 */
 
void scene_render(struct scene *scene) {

  /* Sky.
   */
  uint32_t skycolor=0x96d8eeff;
  if (scene->deathclock>0.0) {
    skycolor=rgba_blend(skycolor,0x801020ff,scene->deathclock/2.000);
  } else if (scene->strikeclock>0.0) {
    skycolor=rgba_blend(skycolor,0x204060ff,scene->strikeclock/STRIKE_HIGHLIGHT_TIME);
  }
  graf_fill_rect(&g.graf,0,0,FBW,FBH,skycolor);
  if (scene->strikeclock>0.0) {
    int alpha=(scene->strikeclock*255.0)/STRIKE_HIGHLIGHT_TIME;
    if (alpha>0) {
      graf_set_input(&g.graf,g.texid_sprites);
      if (alpha<0xff) graf_set_alpha(&g.graf,alpha);
      graf_decal(&g.graf,scene->strikex,scene->strikey,0,32,32,32);
      graf_set_alpha(&g.graf,0xff);
    }
  }
  
  /* Terrain.
   */
  graf_set_input(&g.graf,scene->bgtexid);
  graf_decal(&g.graf,0,0,0,0,FBW,FBH);
  
  /* Sprites.
   */
  graf_set_input(&g.graf,g.texid_sprites);
  int i=0;
  for (;i<scene->spritec;i++) {
    struct sprite *sprite=scene->spritev[i];
    if (sprite->defunct) continue;
    int dstx=(int)(sprite->x*NS_sys_tilesize);
    int dsty=(int)(sprite->y*NS_sys_tilesize);
    graf_tile(&g.graf,dstx,dsty,sprite->tileid,sprite->xform);
  }
  
  /* When the death clock has advanced pretty far (bg already red), fade in another red overlay to make the foreground appear to fade out.
   */
  if (scene->deathclock>2.000) {
    int alpha=(scene->deathclock-2.0)*64.0;
    if (alpha<0) alpha=0; else if (alpha>0xc0) alpha=0xc0;
    graf_fill_rect(&g.graf,0,0,FBW,FBH,0x80102000|alpha);
  }
  
  /* TODO Overlay?
   */
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

  scene->input_blackout=g.pvinput&~EGG_BTN_CD;
  
  // Acquire the resource and render background.
  const void *serial=0;
  int serialc=hummfu_get_res(&serial,EGG_TID_map,mapid);
  struct map_res res={0};
  if (map_res_decode(&res,serial,serialc)<0) return -1;
  if ((res.w!=NS_sys_mapw)||(res.h!=NS_sys_maph)) {
    fprintf(stderr,"map:%d dimensions %dx%d, expected %dx%d\n",mapid,res.w,res.h,NS_sys_mapw,NS_sys_maph);
    return -1;
  }
  scene->map=res.v;
  if (scene_render_bgtex(scene,&res)<0) return -1;
  scene->mapid=mapid;
  
  // Drop volatile things.
  scene->hero=0;
  while (scene->spritec>0) {
    scene->spritec--;
    sprite_del(scene->spritev[scene->spritec]);
  }
  scene->strikeclock=0.0;
  scene->deathclock=0.0;
  
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

/* Highlight strike.
 */
 
void scene_highlight_strike(struct scene *scene,double x,double y) {
  scene->strikeclock=STRIKE_HIGHLIGHT_TIME;
  scene->strikex=(int)(x*NS_sys_tilesize)-NS_sys_tilesize;
  scene->strikey=(int)(y*NS_sys_tilesize)-NS_sys_tilesize;
}

/* Begin death.
 */
 
void scene_begin_death(struct scene *scene) {
  scene->deathclock=0.001;
}
