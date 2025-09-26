#include "game/hummfu.h"

/* Distance from sprite to edge in a cardinal direction.
 */
 
static double nearest_edge_collision(const struct sprite *sprite,uint8_t dir) {
  switch (dir) {
    case 0x40: return sprite->y+sprite->pt;
    case 0x10: return sprite->x+sprite->pl;
    case 0x08: return NS_sys_mapw-(sprite->x+sprite->pr);
    case 0x02: return NS_sys_maph-(sprite->y+sprite->pb);
  }
  return 0.0;
}

/* Distance from sprite to solid grid cell in a cardinal direction.
 */
 
static int grid_row_is_solid(const uint8_t *v,int c) {
  for (;c-->0;v++) switch (g.physics[*v]) {
    case NS_physics_solid:
      return 1;
  }
  return 0;
}
 
static int grid_column_is_solid(const uint8_t *v,int c) {
  for (;c-->0;v+=NS_sys_mapw) switch (g.physics[*v]) {
    case NS_physics_solid:
      return 1;
  }
  return 0;
}
 
static double nearest_grid_collision(const struct sprite *sprite,uint8_t dir) {
  double l=sprite->x+sprite->pl;
  double r=sprite->x+sprite->pr;
  double t=sprite->y+sprite->pt;
  double b=sprite->y+sprite->pb;
  const double fudge=0.001;
  int cola=(int)l; if (cola<0) cola=0;
  int rowa=(int)t; if (rowa<0) rowa=0;
  int colz=(int)(r-fudge); if (colz>=NS_sys_mapw) colz=NS_sys_mapw-1;
  int rowz=(int)(b-fudge); if (rowz>=NS_sys_maph) rowz=NS_sys_maph-1;
  if ((cola>colz)||(rowa>rowz)) return 0.0;
  // Walk outward from the sprite until the edge, stopping at the first solid cell.
  // We're starting at the sprite's far edge. If we get something negativer than half a meter, ignore it.
  // That does happen, due to round-off error. Without this correction, you can get stuck in certain walls by walking into them, then turning around.
  #define RESULT(calc) { \
    double result=(calc); \
    if (result<-0.5) { \
      /* Too negative; ignore it. */ \
    } else { \
      return result; \
    } \
  }
  switch (dir) {
    case 0x40: { // rowz..0
        int colc=colz-cola+1;
        int row=rowz;
        const uint8_t *mapp=g.scene.map+row*NS_sys_mapw+cola;
        for (;row>=0;row--,mapp-=NS_sys_mapw) {
          if (grid_row_is_solid(mapp,colc)) RESULT(t-(row+1.0))
        }
      } break;
    case 0x10: { // colz..0
        int rowc=rowz-rowa+1;
        int col=colz;
        const uint8_t *mapp=g.scene.map+rowa*NS_sys_mapw+col;
        for (;col>=0;col--,mapp--) {
          if (grid_column_is_solid(mapp,rowc)) RESULT(l-(col+1.0))
        }
      } break;
    case 0x08: { // cola..mapw-1
        int rowc=rowz-rowa+1;
        int col=cola;
        const uint8_t *mapp=g.scene.map+rowa*NS_sys_mapw+col;
        for (;col<NS_sys_mapw;col++,mapp++) {
          if (grid_column_is_solid(mapp,rowc)) RESULT(col-r)
        }
      } break;
    case 0x02: { // rowa..maph-1
        int colc=colz-cola+1;
        int row=rowa;
        const uint8_t *mapp=g.scene.map+row*NS_sys_mapw+cola;
        for (;row<NS_sys_maph;row++,mapp+=NS_sys_mapw) {
          if (grid_row_is_solid(mapp,colc)) RESULT(row-b)
        }
      } break;
  }
  #undef RESULT
  return 999.999;
}

/* Distance from sprite to another solid sprite in a cardinal direction.
 */
 
static double nearest_sprite_collision(const struct sprite *sprite,uint8_t dir) {
  double l=sprite->x+sprite->pl;
  double r=sprite->x+sprite->pr;
  double t=sprite->y+sprite->pt;
  double b=sprite->y+sprite->pb;
  double nearest=999.999;
  struct sprite **p=g.scene.spritev;
  int i=g.scene.spritec;
  for (;i-->0;p++) {
    struct sprite *other=*p;
    if (other==sprite) continue;
    if (other->defunct) continue;
    if (!other->solid) continue;
    switch (dir) {
      case 0x40: {
          if (other->x+other->pr<=l) continue;
          if (other->x+other->pl>=r) continue;
          if (other->y+other->pt>=b) continue;
          double q=t-(other->y+other->pb);
          if (q<nearest) nearest=q;
        } break;
      case 0x10: {
          if (other->y+other->pb<=t) continue;
          if (other->y+other->pt>=b) continue;
          if (other->x+other->pl>=r) continue;
          double q=l-(other->x+other->pr);
          if (q<nearest) nearest=q;
        } break;
      case 0x08: {
          if (other->y+other->pb<=t) continue;
          if (other->y+other->pt>=b) continue;
          if (other->x+other->pr<=l) continue;
          double q=other->x+other->pl-r;
          if (q<nearest) nearest=q;
        } break;
      case 0x02: {
          if (other->x+other->pr<=l) continue;
          if (other->x+other->pl>=r) continue;
          if (other->y+other->pb<=t) continue;
          double q=other->y+other->pt-b;
          if (q<nearest) nearest=q;
        } break;
    }
    if (nearest<=0.0) break;
  }
  return nearest;
}

/* Move sprite.
 * This is strictly one-dimensional. If called with two nonzero deltas, we'll recur for each.
 * We will move only in the requested direction. Never backward and never on the off-axis.
 */
 
int sprite_move(struct sprite *sprite,double dx,double dy) {
  if (!sprite) return 0;
  if (!sprite->solid) {
    sprite->x+=dx;
    sprite->y+=dy;
    return 1;
  }
  if (
    ((dx<-0.0)||(dx>0.0))&&
    ((dy<-0.0)||(dy>0.0))
  ) {
    int x=sprite_move(sprite,dx,0.0);
    int y=sprite_move(sprite,0.0,dy);
    return x||y;
  }
  uint8_t dir=0; // 0x40,0x10,0x08,0x02, like map neighbor masks.
  double mag=0.0; // Absolute value of motion in the requested direction.
       if (dx<0.0) { dir=0x10; mag=-dx; }
  else if (dx>0.0) { dir=0x08; mag= dx; }
  else if (dy<0.0) { dir=0x40; mag=-dy; }
  else if (dy>0.0) { dir=0x02; mag= dy; }
  else return 0;
  
  /* There's three things we can collide against: Edge, Grid, and Sprites.
   * Call out for the nearest collision to each.
   * If any of the three <=0, stop immediately.
   * Otherwise, take the shortest and clamp to (mag).
   */
  double toe=nearest_edge_collision(sprite,dir); if (toe<=0.0) return 0;
  double tog=nearest_grid_collision(sprite,dir); if (tog<=0.0) return 0;
  double tos=nearest_sprite_collision(sprite,dir); if (tos<=0.0) return 0;
  double available;
  if ((toe<=tog)&&(toe<=tos)) available=toe;
  else if (tog<=tos) available=tog;
  else available=tos;
  if (available>mag) available=mag;
  
  // Commit it.
  switch (dir) {
    case 0x40: sprite->y-=available; break;
    case 0x10: sprite->x-=available; break;
    case 0x08: sprite->x+=available; break;
    case 0x02: sprite->y+=available; break;
  }
  return 1;
}

/* Rectify.
 */

int sprite_rectify(struct sprite *sprite) {
  if (!sprite||!sprite->solid) return 0;
  double l=sprite->x+sprite->pl;
  double t=sprite->y+sprite->pt;
  double r=sprite->x+sprite->pr;
  double b=sprite->y+sprite->pb;
  const double fudge=0.001;
  
  #define AABB_LIMIT 8
  struct aabb { double l,r,t,b; } aabbv[AABB_LIMIT];
  int aabbc=0;
  
  /* Grid.
   */
  int cola=(int)l; if (cola<0) cola=0;
  int rowa=(int)t; if (rowa<0) rowa=0;
  int colz=(int)(r-fudge); if (colz>=NS_sys_mapw) colz=NS_sys_mapw-1;
  int rowz=(int)(b-fudge); if (rowz>=NS_sys_maph) rowz=NS_sys_maph-1;
  if ((cola<=colz)&&(rowa<=rowz)) {
    const uint8_t *mrow=g.scene.map+rowa*NS_sys_mapw+cola;
    int row=rowa;
    for (;row<=rowz;row++,mrow+=NS_sys_mapw) {
      const uint8_t *mp=mrow;
      int col=cola;
      for (;col<=colz;col++,mp++) {
        uint8_t physics=g.physics[*mp];
        if (physics==NS_physics_solid) {
          struct aabb *aabb=aabbv+aabbc++;
          aabb->l=col;
          aabb->r=col+1.0;
          aabb->t=row;
          aabb->b=row+1.0;
          if (aabbc>=AABB_LIMIT) goto _done_collecting_;
        }
      }
    }
  }
  
  /* Other sprites.
   */
  int i=g.scene.spritec;
  struct sprite **p=g.scene.spritev;
  for (;i-->0;p++) {
    struct sprite *other=*p;
    if (other==sprite) continue;
    if (!other->solid) continue;
    if (other->defunct) continue;
    if (other->x+other->pl>=r) continue;
    if (other->x+other->pr<=l) continue;
    if (other->y+other->pt>=b) continue;
    if (other->y+other->pb<=t) continue;
    struct aabb *aabb=aabbv+aabbc++;
    aabb->l=other->x+other->pl;
    aabb->r=other->x+other->pr;
    aabb->t=other->y+other->pt;
    aabb->b=other->y+other->pb;
    if (aabbc>=AABB_LIMIT) goto _done_collecting_;
  }
  
  /* If we've identified any collisions, find a position that escapes all of them.
   * Calculate the four cardinal escapements for each collision, and reduce those to the max per direction.
   * Also determine a valid escapement for each of the four diagonals, updating one axis of each at each blockage.
   * Whichever per-direction max is smallest, that's the direction we'll go.
   * 
   * In quick initial experiments, this seems to work nicely.
   * It elegantly* solves the "concave corner" problem that I've been stuggling with for years.
   * [*] If you think the mess below is elegant:
   */
 _done_collecting_:;
  if (aabbc<1) return 0;
  struct aabb esc={0.0,0.0,0.0,0.0}; // Absolute values, sprite's escape distance in the given cardinal direction.
  struct diagonal { double x,y; } diagonalv[4]={0}; // NW,NE,SW,SE. Absolute values of diagonal escapes.
  struct aabb *box=aabbv;
  for (i=aabbc;i-->0;box++) {
    // Calculate and record cardinal escapes.
    double el=r-box->l; if (el>esc.l) esc.l=el;
    double er=box->r-l; if (er>esc.r) esc.r=er;
    double et=b-box->t; if (et>esc.t) esc.t=et;
    double eb=box->b-t; if (eb>esc.b) esc.b=eb;
    // Update one axis of each diagonal, whichever of these cardinals is shorter.
    if (el<=et) { if (el>diagonalv[0].x) diagonalv[0].x=el; }
           else { if (et>diagonalv[0].y) diagonalv[0].y=et; }
    if (er<=et) { if (er>diagonalv[1].x) diagonalv[1].x=er; }
           else { if (et>diagonalv[1].y) diagonalv[1].y=et; }
    if (el<=eb) { if (el>diagonalv[2].x) diagonalv[2].x=el; }
           else { if (eb>diagonalv[2].y) diagonalv[2].y=eb; }
    if (er<=eb) { if (er>diagonalv[3].x) diagonalv[3].x=er; }
           else { if (eb>diagonalv[3].y) diagonalv[3].y=eb; }
  }
  int bestp=0; // 0..7: 0..3 for cardinals, 4..7 for diagonals
  double bestscore=esc.l;
  if (esc.r<bestscore) { bestp=1; bestscore=esc.r; }
  if (esc.t<bestscore) { bestp=2; bestscore=esc.t; }
  if (esc.b<bestscore) { bestp=3; bestscore=esc.b; }
  if (diagonalv[0].x+diagonalv[0].y<bestscore) { bestp=4; bestscore=diagonalv[0].x+diagonalv[0].y; }
  if (diagonalv[1].x+diagonalv[1].y<bestscore) { bestp=5; bestscore=diagonalv[1].x+diagonalv[1].y; }
  if (diagonalv[2].x+diagonalv[2].y<bestscore) { bestp=6; bestscore=diagonalv[2].x+diagonalv[2].y; }
  if (diagonalv[3].x+diagonalv[3].y<bestscore) { bestp=7; bestscore=diagonalv[3].x+diagonalv[3].y; }
  switch (bestp) {
    case 0: sprite->x-=esc.l; break;
    case 1: sprite->x+=esc.r; break;
    case 2: sprite->y-=esc.t; break;
    case 3: sprite->y+=esc.b; break;
    case 4: sprite->x-=diagonalv[0].x; sprite->y-=diagonalv[0].y; break;
    case 5: sprite->x+=diagonalv[1].x; sprite->y-=diagonalv[1].y; break;
    case 6: sprite->x-=diagonalv[2].x; sprite->y+=diagonalv[2].y; break;
    case 7: sprite->x+=diagonalv[3].x; sprite->y+=diagonalv[3].y; break;
  }
  
  #undef AABB_LIMIT
  return 1;
}

/* Test position.
 */

int sprite_test_position(const struct sprite *sprite) {
  if (!sprite||!sprite->solid) return 0;
  double l=sprite->x+sprite->pl;
  double t=sprite->y+sprite->pt;
  double r=sprite->x+sprite->pr;
  double b=sprite->y+sprite->pb;
  const double fudge=0.001; // mmm double fudge
  
  /* Grid.
   */
  int cola=(int)l; if (cola<0) cola=0;
  int rowa=(int)t; if (rowa<0) rowa=0;
  int colz=(int)(r-fudge); if (colz>=NS_sys_mapw) colz=NS_sys_mapw-1;
  int rowz=(int)(b-fudge); if (rowz>=NS_sys_maph) rowz=NS_sys_maph-1;
  if ((cola<=colz)&&(rowa<=rowz)) {
    const uint8_t *mrow=g.scene.map+rowa*NS_sys_mapw+cola;
    int row=rowa;
    for (;row<=rowz;row++,mrow+=NS_sys_mapw) {
      const uint8_t *mp=mrow;
      int col=cola;
      for (;col<=colz;col++,mp++) {
        uint8_t physics=g.physics[*mp];
        if (physics==NS_physics_solid) return 1;
      }
    }
  }
  
  /* Other sprites.
   */
  int i=g.scene.spritec;
  struct sprite **p=g.scene.spritev;
  for (;i-->0;p++) {
    struct sprite *other=*p;
    if (other==sprite) continue;
    if (!other->solid) continue;
    if (other->defunct) continue;
    if (other->x+other->pl>=r) continue;
    if (other->x+other->pr<=l) continue;
    if (other->y+other->pt>=b) continue;
    if (other->y+other->pb<=t) continue;
    return 1;
  }
  
  return 0;
}
