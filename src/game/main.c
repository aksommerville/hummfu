#include "hummfu.h"

struct g g={0};

void egg_client_quit(int status) {
}

/* Load the terrain tilesheet to (g.physics).
 */
 
static void load_physics(const void *v,int c) {
  struct tilesheet_reader reader;
  if (tilesheet_reader_init(&reader,v,c)<0) return;
  struct tilesheet_entry entry;
  while (tilesheet_reader_next(&entry,&reader)>0) {
    if (entry.tableid!=NS_tilesheet_physics) continue;
    memcpy(g.physics+entry.tileid,entry.v,entry.c);
  }
}

/* Update (g.killc_max,g.breakc_max) in light of a map resource.
 */
 
static void count_map_static_features(const void *v,int c) {
  struct map_res map;
  if (map_res_decode(&map,v,c)<0) return;
  struct cmdlist_reader reader;
  if (cmdlist_reader_init(&reader,map.cmd,map.cmdc)<0) return;
  struct cmdlist_entry cmd;
  while (cmdlist_reader_next(&cmd,&reader)>0) {
    switch (cmd.opcode) {
      case CMD_map_sprite: {
          int spriteid=(cmd.arg[2]<<8)|cmd.arg[3];
          switch (score_type_for_spriteid(spriteid)) {
            case 'k': g.killc_max++; break;
            case 'b': g.breakc_max++; break;
          }
        } break;
    }
  }
}

/* Init.
 */

int egg_client_init() {

  int fbw=0,fbh=0;
  egg_texture_get_size(&fbw,&fbh,1);
  if ((fbw!=FBW)||(fbh!=FBH)) {
    fprintf(stderr,"Framebuffer size mismatch! metadata=%dx%d header=%dx%d\n",fbw,fbh,FBW,FBH);
    return -1;
  }

  g.romc=egg_rom_get(0,0);
  if (!(g.rom=malloc(g.romc))) return -1;
  egg_rom_get(g.rom,g.romc);
  struct rom_reader reader;
  if (rom_reader_init(&reader,g.rom,g.romc)<0) return -1;
  struct rom_entry res;
  while (rom_reader_next(&res,&reader)>0) {
    if (g.resc>=g.resa) {
      int na=g.resa+64;
      if (na>INT_MAX/sizeof(struct rom_entry)) return -1;
      void *nv=realloc(g.resv,sizeof(struct rom_entry)*na);
      if (!nv) return -1;
      g.resv=nv;
      g.resa=na;
    }
    g.resv[g.resc++]=res;
    // As long as we're iterating these, capture any global resources we need:
    if ((res.tid==EGG_TID_tilesheet)&&(res.rid==RID_tilesheet_terrain)) load_physics(res.v,res.c);
    else if (res.tid==EGG_TID_map) count_map_static_features(res.v,res.c);
  }
  
  if (egg_texture_load_image(g.texid_sprites=egg_texture_new(),RID_image_sprites)<0) return -1;
  if (egg_texture_load_image(g.texid_terrain=egg_texture_new(),RID_image_terrain)<0) return -1;
  
  if (!(g.font=font_new())) return -1;
  const char *msg;
  if (msg=font_add_image(g.font,RID_image_font9_0020,0x0020)) { fprintf(stderr,"ERROR: %s\n",msg); return -1; }
  
  egg_store_get(g.hiscore,sizeof(g.hiscore),"hiscore",7);
  score_force_valid(g.hiscore);
  
  srand_auto();

  if (scene_begin(&g.scene,1)<0) return -1;
  scene_no_fade_in(&g.scene);

  return 0;
}

/* Update.
 */

void egg_client_update(double elapsed) {
  g.sfxc=0;

  int input=egg_input_get_one(0);
  int pvinput=g.pvinput;
  if (input!=g.pvinput) {
    if ((input&EGG_BTN_AUX1)&&!(pvinput&EGG_BTN_AUX1)) { //XXX Highly temporary: AUX1 to reload map
      scene_begin(&g.scene,g.scene.mapid);
    }
    g.pvinput=input;
  }
  
  if (g.scene.active) scene_update(&g.scene,elapsed,input,pvinput);
  else if (g.gameover.active) gameover_update(&g.gameover,elapsed,input,pvinput);
  else if (scene_begin(&g.scene,1)<0) egg_terminate(1);
}

/* Render.
 */

void egg_client_render() {
  g.framec++;
  graf_reset(&g.graf);
  if (g.scene.active) scene_render(&g.scene);
  else if (g.gameover.active) gameover_render(&g.gameover);
  graf_flush(&g.graf);
}

/* Get resource.
 */
 
int hummfu_get_res(void *dstpp,int tid,int rid) {
  int lo=0,hi=g.resc;
  while (lo<hi) {
    int ck=(lo+hi)>>1;
    const struct rom_entry *q=g.resv+ck;
         if (tid<q->tid) hi=ck;
    else if (tid>q->tid) lo=ck+1;
    else if (rid<q->rid) hi=ck;
    else if (rid>q->rid) lo=ck+1;
    else {
      *(const void**)dstpp=q->v;
      return q->c;
    }
  }
  return 0;
}

/* Render text label.
 */

int hummfu_load_label(int *w,int *h,int ix) {
  const void *serial=0;
  int serialc=hummfu_get_res(&serial,EGG_TID_strings,(egg_prefs_get(EGG_PREF_LANG)<<6)|1);
  if (serialc<1) {
    serialc=hummfu_get_res(&serial,EGG_TID_strings,(EGG_LANG_FROM_STRING("en")<<6)|1);
    if (serialc<1) return 0;
  }
  struct strings_reader reader;
  if (strings_reader_init(&reader,serial,serialc)<0) return 0;
  struct strings_entry string;
  while (strings_reader_next(&string,&reader)>0) {
    if (string.index>ix) return 0;
    if (string.index<ix) continue;
    int texid=font_render_to_texture(0,g.font,string.v,string.c,FBW,FBH,0xffffffff);
    if (texid<1) return 0;
    egg_texture_get_size(w,h,texid);
    return texid;
  }
  return 0;
}

/* Sound effects.
 */
 
void hummfu_sfx(int soundid) {
  int *p=g.sfxv;
  int i=g.sfxc;
  for (;i-->0;p++) if (*p==soundid) return;
  if (g.sfxc<SFXV_LIMIT) g.sfxv[g.sfxc++]=soundid;
  egg_play_sound(soundid,1.0,0.0);
}

void hummfu_sfx_spatial(int soundid,double x) {
  int *p=g.sfxv;
  int i=g.sfxc;
  for (;i-->0;p++) if (*p==soundid) return;
  if (g.sfxc<SFXV_LIMIT) g.sfxv[g.sfxc++]=soundid;
  double pan=(x*2.0)/NS_sys_mapw-1.0;
  egg_play_sound(soundid,1.0,pan);
}
