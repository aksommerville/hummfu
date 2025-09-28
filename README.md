# Humm Fu

Requires [Egg v2](https://github.com/aksommerville/egg2) to build.

For [Code For A Cause Micro Jam 2](https://itch.io/jam/cfac-x-micro-2), theme "TINY BUT MIGHTY".

- Single-screen platformer. Avoid scrolling because I think offscreen point culling is not mitigated yet for web (MacOS clients).
- You're a tiny bird. Hold A to fly, and tap B to swing your blade of grass.
- Use one image each, for sprites and maps. Both tilesheets. 16x16px tiles, but the hero will use only a little of it.

## Timeline

- Thu 25 Sept 17:00: Begin work.
- Fri 26 Sept EOD: Hero, monsters, map loader.
- Sat 27 Sept EOD: Menus. Final maps.
- Sun 28 Sept EOD: Music. Itch page. Aim to be fully finished.
- Mon 29 Sept: Overflow.
- Tue 30 Sept 12:00: Deadline. No time to work today.

## TODO

- [x] General sprite physics.
- [x] Hero movement.
- [x] Swing blade.
- [x] Monsters. Bear and alien. Maybe that's enough? Make more if there's time.
- [x] Destroyable static items.
- [x] Static hazards.
- [x] Sound effects.
- [ ] Sticking to the wall here and there, feels like a rounding error. Easily exposed in map:2, near the SW corner.
- [ ] Flight training level, fly thru the rings or something.
- [ ] Combat training level, whack cardboard cutouts when they pop up. (but not the nun or the child, just the gangsters).
- [ ] Final levels. Probably like 20? I expect them to play really fast.
- [ ] Tune time-bonus times. (score.c)
- [x] Scorekeeping.
- [x] Consider nixing Hello modal, and instead make it the first level. Playable but all you can do is proceed.
- [x] Hello modal.
- [x] Gameover modal.
- [x] When aliens shoot a box they break it and you don't get the points. That's by design, but don't let it happen randomly.
- - So, when the alien is going to shoot randomly, check for boxes first. If they're shooting because the hero is present, let it happen.
