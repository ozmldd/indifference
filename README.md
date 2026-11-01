# Indifference

A 2D game written from scratch in C++ with SDL3.

> A game me and my friend built just for fun and to challenge myself with my new coding
> skills in c++. Simple story game on various different lessons

## Features

- Hand-rolled game loop with delta-time movement — no engine, no framework
- Two-pass axis-separated collision (resolve X, then resolve Y) with a float epsilon
  so grazing contacts don't register as hits
- intro cut scene
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

arrows keys to move (left for left, right for right, you get the deal)
hold either shift keys to sprint
Z for interacting with characters
W for skipping dialog
S for skipping some of the credits at the end

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

## Project structure

```
.
├── indifference.cpp        main loop, update, render, collision, menus
├── indifference.exe        basic executable
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

## Level format

Levels are 2D integer arrays, read tile by tile in `createTiles`:

To add a level, add a `case` to the `createTiles` switch and point it at the new map
array. Every value needs its own case — a missing one falls through to `default` and
silently loads level 1 instead.

## Save data

Saves are written to the platform preferences directory via `SDL_GetPrefPath`, not
next to the executable. The file stores the level number, player state, and the
player's X and Y position, one value per line.

Reading it back uses `std::stoi` / `std::stof`, which throw on an empty line — so
the read is wrapped in a `try` / `catch` and the save is loaded once at startup
rather than on every call to `createTiles`.

## Assets and licensing

Shoutout to my friend Diego (@itsmista_dlg) for some of the drawing like the main character
Another shoutout to Ceebee(ceebee_do_thingies) made some incredible art for the cutscene seriously follow
him he came in clutch.
Rest of the tiles and NPC's were drawn (lazily) by me

##AI USAGE (very important)
my code was AI assisted some times just for ideas on structure, other times on the actual code.
Here are code that was 100& AI:
-The epilepsy warning 
-In-game Menu function
-Credit function
-some of the dialog system (mostly the framing of the function with the use of arrays)
-switch level transition
-intro cutscene transition
-all of the Name entry
-control screen
-the zoom in for the player
-slider and buttons
The rest was from me and an old game I built using a youtube tutorial (Shout out Constref)
