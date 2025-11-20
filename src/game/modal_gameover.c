#include "hummfu.h"
#include <limits.h>

#define BG_COLOR 0x106620ff

/* Update.
 */

void gameover_update(struct gameover *gameover,double elapsed,int input,int pvinput) {
  if ((input&EGG_BTN_SOUTH)&&!(pvinput&EGG_BTN_SOUTH)) {
    scene_begin(&g.scene,1);
  }
}

/* Render.
 */
 
void gameover_render(struct gameover *gameover) {
  graf_fill_rect(&g.graf,0,0,FBW,FBH,BG_COLOR);
  struct gameover_label *label=gameover->labelv;
  int i=gameover->labelc;
  for (;i-->0;label++) {
    if (label->blink&&(g.framec%40>30)) continue;
    graf_set_input(&g.graf,label->texid);
    graf_decal(&g.graf,label->x,label->y,0,0,label->w,label->h);
  }
}

/* Get a string by index.
 */
 
static int gameover_get_string(void *dstpp,const struct gameover *gameover,int index) {
  struct strings_reader reader;
  if (strings_reader_init(&reader,gameover->strings,gameover->stringsc)<0) return 0;
  struct strings_entry string;
  while (strings_reader_next(&string,&reader)>0) {
    if (string.index<index) continue;
    if (string.index>index) return 0;
    *(const void**)dstpp=string.v;
    return string.c;
  }
  return 0;
}

/* Add a label with no texture yet.
 */
 
static struct gameover_label *gameover_label_add(struct gameover *gameover) {
  if (gameover->labelc>=GAMEOVER_LABEL_LIMIT) return 0;
  struct gameover_label *label=gameover->labelv+gameover->labelc++;
  memset(label,0,sizeof(struct gameover_label));
  return label;
}

/* Generate the advice label.
 */
 
static int gameover_add_advice(struct gameover *gameover,int index,uint32_t rgba,int blink,int midy) {
  const char *text=0;
  int textc=gameover_get_string(&text,gameover,index);
  
  // Index 11 is the time advice. Replace '%' with the scorekeeper's best time.
  // All other strings are static.
  char tmp[256];
  if (index==11) {
    int tmpc=0,textp=0;
    for (;textp<textc;textp++) {
      if (text[textp]=='%') {
        int target=(int)score_best_time();
        if (target<0) target=0;
        else if (target>999) target=999;
        if (tmpc>sizeof(tmp)-3) return -1;
        if (target>=100) tmp[tmpc++]='0'+target/100;
        if (target>=10) tmp[tmpc++]='0'+(target/10)%10;
        tmp[tmpc++]='0'+target%10;
      } else {
        if (tmpc>=sizeof(tmp)) return -1;
        tmp[tmpc++]=text[textp];
      }
    }
    text=tmp;
    textc=tmpc;
  }
  
  struct gameover_label *label=gameover_label_add(gameover);
  if (!label) return -1;
  label->texid=font_render_to_texture(0,g.font,text,textc,FBW,FBH,rgba);
  egg_texture_get_size(&label->w,&label->h,label->texid);
  label->x=(FBW>>1)-(label->w>>1);
  label->y=midy-(label->h>>1);
  label->blink=blink;
  return 0;
}

/* Render one line into the temporary buffer.
 * Label goes right of (x) and value goes left of it.
 * (row) should increase monotonically from zero.
 */
 
static void gameover_render_report_line(
  struct gameover *gameover,
  uint32_t *rgbav, // FBW*FBH
  int x,int row,
  int kindex,
  int (*vrepr)(char *dst,int dsta,int *perfect,struct gameover *gameover)
) {
  int y=row*(font_get_line_height(g.font)+1);
  
  const char *k=0;
  int kc=gameover_get_string(&k,gameover,kindex);
  int kw=font_measure_string(g.font,k,kc);
  int kx=x-kw-5;
  font_render(rgbav+y*FBW+kx,FBW-kx,FBH-y,FBW<<2,g.font,k,kc,0xc0c0c0ff);
  
  int perfect=0;
  char v[256];
  int vc=vrepr(v,sizeof(v),&perfect,gameover);
  if ((vc<0)||(vc>sizeof(v))) vc=0;
  int vw=font_measure_string(g.font,v,vc);
  int vx=FBW-vw;
  font_render(rgbav+y*FBW+vx,FBW-vx,FBH-y,FBW<<2,g.font,v,vc,perfect?0xffff00ff:0xffffffff);
}

/* Represent values for gameover_render_report_line.
 */
 
static int repr_time(char *dst,int dsta,int *perfect,struct gameover *gameover) {
  if (g.score.time<score_best_time()) *perfect=1;
  if (dsta<9) return 9;
  int ms=(int)(g.score.time*1000.0);
  if (ms<0) ms=0;
  int sec=ms/1000; ms%=1000;
  int min=sec/60; sec%=60;
  if (min>99) { min=sec=99; ms=999; }
  dst[0]='0'+min/10;
  dst[1]='0'+min%10;
  dst[2]=':';
  dst[3]='0'+sec/10;
  dst[4]='0'+sec%10;
  dst[5]='.';
  dst[6]='0'+ms/100;
  dst[7]='0'+(ms/10)%10;
  dst[8]='0'+ms%10;
  return 9;
}

// Variable-legnth unsigned integer.
static int repr_uint(char *dst,int dsta,int v) {
  if (v<0) v=0;
  int dstc=1,limit=10;
  while (v>=limit) { dstc++; if (limit>UINT_MAX/10) break; limit*=10; }
  if (dstc>dsta) return dstc;
  int i=dstc;
  for (;i-->0;v/=10) dst[i]='0'+v%10;
  return dstc;
}

static int repr_deathc(char *dst,int dsta,int *perfect,struct gameover *gameover) {
  if (!g.score.deathc) *perfect=1;
  return repr_uint(dst,dsta,g.score.deathc);
}

static int repr_killc(char *dst,int dsta,int *perfect,struct gameover *gameover) {
  if (g.score.killc>=g.killc_max) *perfect=1;
  int dstc=repr_uint(dst,dsta,g.score.killc);
  if (dstc<dsta) dst[dstc]='/'; dstc++;
  dstc+=repr_uint(dst+dstc,dsta-dstc,g.killc_max);
  return dstc;
}

static int repr_breakc(char *dst,int dsta,int *perfect,struct gameover *gameover) {
  if (g.score.breakc>=g.breakc_max) *perfect=1;
  int dstc=repr_uint(dst,dsta,g.score.breakc);
  if (dstc<dsta) dst[dstc]='/'; dstc++;
  dstc+=repr_uint(dst+dstc,dsta-dstc,g.breakc_max);
  return dstc;
}

static int repr_total(char *dst,int dsta,int *perfect,struct gameover *gameover) {
  if (score_is_perfect(gameover->score)) *perfect=1;
  if (dsta<SCORE_LENGTH) return SCORE_LENGTH;
  memcpy(dst,gameover->score,SCORE_LENGTH);
  return SCORE_LENGTH;
}

static int repr_hiscore(char *dst,int dsta,int *perfect,struct gameover *gameover) {
  if (dsta<SCORE_LENGTH) return SCORE_LENGTH;
  if (memcmp(gameover->score,g.hiscore,SCORE_LENGTH)>=0) {
    *perfect=1;
    memcpy(dst,gameover->score,SCORE_LENGTH);
  } else {
    memcpy(dst,g.hiscore,SCORE_LENGTH);
  }
  return SCORE_LENGTH;
}

/* Nonzero if this framebuffer-strided buffer contains only full zeroes.
 */
 
static int pixels_zero(const uint32_t *v,int w,int h) {
  for (;h-->0;v+=FBW) {
    const uint32_t *p=v;
    int xi=w;
    for (;xi-->0;p++) if (*p) return 0;
  }
  return 1;
}

/* Generate the report:
 *  - Header, static text.
 *  - Table, line items from score.
 *  - Advice.
 * We also clear (g.score) and commit the new high score if warranted, since we're in a position to.
 */
 
static int gameover_generate_report(struct gameover *gameover) {
  score_rate(gameover->score,sizeof(gameover->score),&g.score);
  
  /* Start with the itemized report.
   * Take a framebuffer-sized buffer, fill it, and crop it to only the needful size.
   * This report gets centered in the framebuffer and drives the position of the other labels.
   */
  uint32_t rgbav[FBW*FBH]={0};
  int kright=FBW-80; // Key and value both align right in their respective columns. The *left* side will be cropped.
  int row=0;
  gameover_render_report_line(gameover,rgbav,kright,row++,14,repr_time);
  gameover_render_report_line(gameover,rgbav,kright,row++,15,repr_deathc);
  gameover_render_report_line(gameover,rgbav,kright,row++,16,repr_killc);
  gameover_render_report_line(gameover,rgbav,kright,row++,17,repr_breakc);
  gameover_render_report_line(gameover,rgbav,kright,row++,18,repr_total);
  if (!score_is_zero(g.hiscore)) {
    row++;
    gameover_render_report_line(gameover,rgbav,kright,row++,19,repr_hiscore);
  }
  int x=0;
  while ((x<FBW)&&pixels_zero(rgbav+x,1,FBH)) x++;
  int w=FBW-x;
  int h=FBH;
  while (h&&pixels_zero(rgbav+(h-1)*FBW,FBW,1)) h--;
  if ((w<1)||(h<1)) return -1;
  struct gameover_label *report=gameover_label_add(gameover);
  if (!report) return -1;
  if ((report->texid=egg_texture_new())<1) return -1;
  if (egg_texture_load_raw(report->texid,w,h,FBW<<2,rgbav+x,sizeof(rgbav))<0) return -1;
  report->w=w;
  report->h=h;
  report->x=(FBW>>1)-(report->w>>1);
  report->y=(FBH>>1)-(report->h>>1);
  
  /* Advice label at the bottom. Various colors and content, and sometimes blinking.
   */
  int advicey=report->y+report->h+((FBH-report->y-report->h)>>1);
  if (memcmp(gameover->score,g.hiscore,SCORE_LENGTH)>0) {
    memcpy(g.hiscore,gameover->score,SCORE_LENGTH);
    egg_store_set("hiscore",7,gameover->score,SCORE_LENGTH);
    gameover_add_advice(gameover,7,0xffff00ff,1,advicey);
  } else if (score_is_perfect(gameover->score)) {
    gameover_add_advice(gameover,12,0xffff00ff,1,advicey);
  } else if (g.score.deathc) {
    gameover_add_advice(gameover,8,0xc0c0c0ff,0,advicey);
  } else if (g.score.killc<g.killc_max) {
    gameover_add_advice(gameover,9,0xc0c0c0ff,0,advicey);
  } else if (g.score.breakc<g.breakc_max) {
    gameover_add_advice(gameover,10,0xc0c0c0ff,0,advicey);
  } else if (score_rate_time(g.score.time)<1) {
    gameover_add_advice(gameover,11,0xc0c0c0ff,0,advicey);
  } else {
    // This should never happen, since if all the constituents are perfect the outcome should be perfect too (clause 12).
    fprintf(stderr,"%s:%d:WARNING: Failed to generate score advice.\n",__FILE__,__LINE__);
  }
  
  /* Static text greeting at the top.
   */
  struct gameover_label *greeting=gameover_label_add(gameover);
  if (!greeting) return -1;
  const char *text=0;
  int textc=gameover_get_string(&text,gameover,13);
  greeting->texid=font_render_to_texture(0,g.font,text,textc,FBW,FBH,0xffffffff);
  egg_texture_get_size(&greeting->w,&greeting->h,greeting->texid);
  greeting->x=(FBW>>1)-(greeting->w>>1);
  greeting->y=report->y>>1;
  
  memset(&g.score,0,sizeof(g.score));
  return 0;
}

/* Begin.
 */
 
int gameover_begin(struct gameover *gameover) {
  g.scene.active=0;
  gameover->active=1;
  hummfu_song(RID_song_peace_returns,0);
  
  if (!gameover->strings) {
    gameover->stringsc=hummfu_get_res(&gameover->strings,EGG_TID_strings,(egg_prefs_get(EGG_PREF_LANG)<<6)|1);
  }
  
  while (gameover->labelc>0) {
    gameover->labelc--;
    egg_texture_del(gameover->labelv[gameover->labelc].texid);
  }
  
  if (gameover_generate_report(gameover)<0) return -1;
  
  return 0;
}
