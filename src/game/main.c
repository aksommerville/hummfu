#include "hummfu.h"

struct g g={0};

void egg_client_quit(int status) {
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
  }
  
  if (egg_texture_load_image(g.texid_sprites=egg_texture_new(),RID_image_sprites)<0) return -1;
  if (egg_texture_load_image(g.texid_terrain=egg_texture_new(),RID_image_terrain)<0) return -1;

  // XXX Should be hello ultimately.
  //if (hello_begin(&g.hello)<0) return -1;
  if (scene_begin(&g.scene,1)<0) return -1;

  return 0;
}

/* Update.
 */

void egg_client_update(double elapsed) {

  int input=egg_input_get_one(0);
  int pvinput=g.pvinput;
  if (input!=g.pvinput) {
    g.pvinput=input;
  }
  
  if (g.scene.active) scene_update(&g.scene,elapsed,input,pvinput);
  else if (g.hello.active) hello_update(&g.hello,elapsed,input,pvinput);
  else if (g.gameover.active) gameover_update(&g.gameover,elapsed,input,pvinput);
  else if (hello_begin(&g.hello)<0) egg_terminate(1);
}

/* Render.
 */

void egg_client_render() {
  g.framec++;
  graf_reset(&g.graf);
  if (g.scene.active) scene_render(&g.scene);
  else if (g.hello.active) hello_render(&g.hello);
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
