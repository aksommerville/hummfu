/* sprite_title.c
 * The big "Humm Fu" banner and some lesser text labels.
 * All the extra stuff for the hello level.
 */
 
#include "game/hummfu.h"

#define LABEL_LIMIT 4

struct sprite_title {
  struct sprite hdr;
  struct label {
    int texid;
    int x,y,w,h;
  } labelv[LABEL_LIMIT];
  int labelc;
};

#define SPRITE ((struct sprite_title*)sprite)

static void _title_del(struct sprite *sprite) {
  struct label *label=SPRITE->labelv;
  int i=SPRITE->labelc;
  for (;i-->0;label++) {
    egg_texture_del(label->texid);
  }
}

static int title_add_label(struct sprite *sprite,const char *src,int srcc) {
  if (SPRITE->labelc>=LABEL_LIMIT) return -1;
  struct label *label=SPRITE->labelv+SPRITE->labelc++;
  if ((label->texid=font_render_to_texture(0,g.font,src,srcc,FBW,FBH,0x000000ff))<1) return -1;
  egg_texture_get_size(&label->w,&label->h,label->texid);
  return 0;
}

static int _title_init(struct sprite *sprite) {

  const char *hspfx=0;
  int hspfxc=0;
  const void *serial=0;
  int serialc=hummfu_get_res(&serial,EGG_TID_strings,(egg_prefs_get(EGG_PREF_LANG)<<6)|1);
  struct strings_reader reader;
  if (strings_reader_init(&reader,serial,serialc)>=0) {
    struct strings_entry string;
    while (strings_reader_next(&string,&reader)>0) {
      switch (string.index) {
        case 3: case 4: case 5: title_add_label(sprite,string.v,string.c); break;
        case 6: hspfx=string.v; hspfxc=string.c; break;
      }
    }
  }
  
  if (!score_is_zero(g.hiscore)) {
    char tmp[256];
    int tmpc=snprintf(tmp,sizeof(tmp),"%.*s %.*s",hspfxc,hspfx,SCORE_LENGTH,g.hiscore);
    if ((tmpc>0)&&(tmpc<sizeof(tmp))) {
      title_add_label(sprite,tmp,tmpc);
    }
  }
  
  int xz=260;
  int y=83;
  struct label *label=SPRITE->labelv;
  int i=SPRITE->labelc;
  for (;i-->0;label++,y+=10) {
    label->y=y;
    label->x=xz-label->w;
  }

  return 0;
}

static void _title_render(struct sprite *sprite) {
  graf_set_image(&g.graf,RID_image_title);
  graf_decal(&g.graf,60,1,0,0,199,97);
  struct label *label=SPRITE->labelv;
  int i=SPRITE->labelc;
  for (;i-->0;label++) {
    graf_set_input(&g.graf,label->texid);
    graf_decal(&g.graf,label->x,label->y,0,0,label->w,label->h);
  }
}

const struct sprite_type sprite_type_title={
  .name="title",
  .objlen=sizeof(struct sprite_title),
  .del=_title_del,
  .init=_title_init,
  .render=_title_render,
};
