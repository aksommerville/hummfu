/* scene.h
 * The game's model world.
 * Responsible for everything except the two modals.
 */
 
#ifndef SCENE_H
#define SCENE_H

struct sprite;

struct scene {
  int active;
  int mapid;
  int bgtexid;
  struct sprite **spritev;
  int spritec,spritea;
  struct sprite *hero; // WEAK, OPTIONAL
  const uint8_t *map; // (NS_sys_mapw*NS_sys_maph)
};

void scene_update(struct scene *scene,double elapsed,int input,int pvinput);
void scene_render(struct scene *scene);

int scene_begin(struct scene *scene,int mapid);

struct sprite *scene_spawn_sprite(struct scene *scene,double x,double y,int spriteid,uint32_t arg);

#endif
