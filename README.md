# Indifference

A 2D game written from scratch in C++ with SDL3.

> A game me and my friend built just for fun and to challenge myself with my new coding
> skills in c++. Simple story game on various different lessons

![Title screen](docs/title.png)

---

## Features

- Hand-rolled game loop with delta-time movement — no engine, no framework
- Two-pass axis-separated collision (resolve X, then resolve Y) with a float epsilon
  so grazing contacts don't register as hits
- Held-button jumping, so bunny hopping chains naturally and air control stays responsive
- Enemies that track the player, with a death screen and a full level reset
- Parallax scrolling background that wraps seamlessly in both directions
- Camera that follows the player and clamps to the level bounds
- Tile-based levels loaded from an integer map, switchable at runtime
- NPC dialogue with per-character lines, word wrap, and line advancing
- Title screen, options menu with a draggable volume slider, and a pause menu that
  keeps the world visible behind it
- Save and load — level and player position persist between sessions
- Scrolling end credits with a skip delay
- Epilepsy warning screen, and a timed screen-flash effect that grows more frequent
  as the player progresses
- Debug overlay drawing collider rectangles and state text

---

## Controls

| Input | Action |
|---|---|
| `A` / `D` or `←` / `→` | Move |
| `Space` (hold) | Jump — keep it held to bunny hop |
| Mouse | Menus, buttons, volume slider |

> Fill in the rest from your `handleKeyInput` — the debug toggle, the dialogue advance
> key, and the pause key. Anyone cloning this won't be able to guess them.

---

## Building

### Dependencies

- A C++17 compiler (developed with **MinGW-w64 / w64devkit** on Windows)
- **SDL3**
- **SDL3_image** — PNG loading
- **SDL3_mixer** (3.x) — note that 3.x is a full API redesign from 2.x; the 2.x
  tutorials and snippets you find online will not compile against it
- **GLM** — header only, used for `glm::vec2`

### Compiling

```bash
g++ -std=c++17 -Wall gametest.cpp -o indifference.exe \
    -I"path/to/SDL3/include" \
    -I"path/to/glm" \
    -L"path/to/SDL3/lib" \
    -lSDL3 -lSDL3_image -lSDL3_mixer
```

Adjust the include and library paths to wherever you unpacked SDL3. `-Wall` is worth
keeping on — it catches uninitialized members and `=` vs `==` mistakes before they
turn into an afternoon of debugging.

The built executable needs the SDL3 DLLs beside it, plus the `assets/` folder.

---

## Project structure

```
.
├── gametest.cpp        main loop, update, render, collision, menus
├── gameobject.h        GameObject, ObjectType, the ObjectData union, Button, Slider
├── animation.h         Animation — wraps a timer, reports the current frame
├── timer.h             timer — length, elapsed, timeout
├── assets/
│   ├── texture/        sprites and tiles (PNG, loaded with nearest-neighbour scaling)
│   └── audio/          music and sound effects
└── CREDITS.txt         required attribution — see Assets below
```

### Key constants

| Constant | Value | Meaning |
|---|---|---|
| `TILE_SIZE` | 32 | Tile edge length in pixels |
| `MAP_ROWS` | 20 | Level height in tiles |
| `MAP_COLS` | 40 | Level width in tiles |
| `logW` / `logH` | 640 × 360 | Logical resolution — the window scales to this |

Rendering runs through `SDL_SetRenderLogicalPresentation`, so the game letterboxes
cleanly at any window size. Mouse input is converted with
`SDL_ConvertEventToRenderCoordinates` before hit-testing, otherwise buttons stop
lining up the moment the window is resized.

---

## Level format

Levels are 2D integer arrays, read tile by tile in `createTiles`:

| Value | Tile |
|---|---|
| 0 | Empty |
| 1 | Grass — solid |
| 2 | Dirt — solid |
| 3 | Background tile — drawn behind everything, no collider |
| 4 | Player spawn |
| 5 | Event block — triggers a cutscene / level change |
| 6 | NPC, id 0 |
| 7 | NPC, id 1 |
| 8 | End block — rolls the credits |

To add a level, add a `case` to the `createTiles` switch and point it at the new map
array. Every value needs its own case — a missing one falls through to `default` and
silently loads level 1 instead.

---

## Save data

Saves are written to the platform preferences directory via `SDL_GetPrefPath`, not
next to the executable. The file stores the level number, player state, and the
player's X and Y position, one value per line.

Reading it back uses `std::stoi` / `std::stof`, which throw on an empty line — so
the read is wrapped in a `try` / `catch` and the save is loaded once at startup
rather than on every call to `createTiles`.

---

## Assets and licensing

**This matters if you distribute the game.** The art and audio come from several
sources under different terms:

| Source | Terms |
|---|---|
| CraftPix freebies | Attribution required. **Raw art files may not be redistributed** — do not commit `assets/` to a public repo |
| Various sound effects | CC-BY 3.0 and CC-BY-SA 3.0 — attribution required |
| Forest parallax pack | Public domain |

Two rules follow from that:

1. **`CREDITS.txt` must ship with the game**, filled in with the real entries. A
   missing or empty credits file breaks the CC-BY terms.
2. **CC-BY-SA assets stay in their own files.** Merging them into a combined
   spritesheet with differently-licensed art would force the share-alike terms onto
   everything in that file.

Anything original — the code, the tile art, the character sprites, the menu
screens — is yours.

---

## Known issues / TODO

- `CREDITS.txt` is still empty and needs real attribution entries before any build
  goes out
- The screen-flash effect is wired up but `gs.flash` is never set to `true`, so it
  never fires — hook it to the end of the intro
- Top-down sprite set exists for up and down; left and right still need cutting
