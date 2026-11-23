# Final Project — *Seven Star Rising* (Playtest 1 build)

A top-down action-adventure. You're a young martial artist racing an
invading warlord to find the seven star orbs. This build is the first
playable slice: the menu and the **Mountain Valley** area.

## Build & run

From inside this `final-project/` folder (raylib must be installed):

```bash
make run    # build and launch the game
```

## Controls

| Key | Action |
| --- | --- |
| `W A S D` / arrow keys | Move (four directions, diagonals allowed) |
| `J` / `Space` | Fire a ki blast in the direction you face (costs 12 ki) |
| Hold `K` | Charge ki. You can't move, and hits do double damage while charging |
| `Enter` | Start / continue on menu screens |
| `Q` / `Esc` | Quit |

## What's in Playtest 1

- **Menu scene:** title, "Press ENTER to start", the controls and the goal.
- **Top-down movement:** no gravity or jumping. The hero collides with bushes,
  rocks and the river, using the lecture's collision probes.
- **Animated hero:** walks in 4 directions, punches when firing a ki blast and
  raises both arms while charging.
- **Ki meter:** blasts spend ki, and holding `K` refills it with an aura. A tick
  on the bar marks the cost of one blast. With too little ki the bar flashes red.
- **Enemies (course `AIType` / `AIState`):**
  - **Soldiers (`WANDERER`):** patrol back and forth (`WALKING`), then chase
    you (`FOLLOWING`) when you get close. They take 5 blasts.
  - **Scouts (`FOLLOWER`):** hover (`IDLE`) until you're in range, then fly at
    you (`FOLLOWING`). They fly over the river and bushes. They take 4 blasts.
- **Health & lives:** 5 hearts per life and 3 lives. Enemies hurt on contact,
  with knockback and a short invincibility blink. Losing a life respawns you at
  the start. Losing the last life shows the **Game Over** screen.
- **Goal for this build:** find the valley's 3 star orbs (the HUD counts to 7).
  That opens the stone gate on the east side. Walk out through it to reach the
  end-of-playtest screen.
- **Ki blasts:** a fixed set of 12 `Entity` objects that are switched
  `ACTIVE` / `INACTIVE` and reused.

Coming in Playtest 2: melee combo, dash, flight over water/chasms, gunner
enemies, the wasteland and fortress areas, music and sound effects.

## Project layout

The code follows the structure of the course lectures (scenes, `Entity`,
`Map`, `GameState`):

```
final-project/
├── main.cpp              window, scenes, camera, fixed-timestep loop
├── CS3113/
│   ├── cs3113.h/.cpp     helpers from the lectures, scene IDs
│   ├── Map.h/.cpp        the lecture's tile map
│   ├── Entity.h/.cpp     top-down entity: movement, collision, animation, AI
│   ├── Scene.h/.cpp      scene base class and GameState
│   ├── Menu.h/.cpp       title screen
│   ├── Valley.h/.cpp     the Mountain Valley area (map data, enemies, HUD)
│   └── EndScreen.h/.cpp  "Valley Cleared" and "Game Over" screens
└── assets/sprites/       the pack PNGs the game uses
```

The valley map is two `unsigned int` arrays in `Valley.h`, drawn as two
`Map`s: a ground layer that is only drawn, and a solid layer (bushes, rocks,
water and the gate) that blocks movement. Opening the gate sets its tiles
in the solid layer to 0.

## Credits

All art comes from free asset packs. No lecture assets and no Dragon Ball
sprites are used.

- **[Ninja Adventure - Asset Pack](https://pixel-boy.itch.io/ninja-adventure-asset-pack)**
  by Pixel-boy and AAA, released under **CC0**. Used for:
  - **Hero:** FighterWhite
  - **Soldiers:** Knight, GladiatorBlue
  - **Scouts:** NinjaMageBlack
  - **Tiles:** `valley_tiles.png` is 8 cells copied out of the pack's
    TilesetFloor, TilesetNature, TilesetWater and TilesetRelief, side by side,
    so the course `Map` can use one texture
  - **Ki blast:** EnergyBall
  - **Star orbs:** GemYellow
  - **HUD:** hearts and the hero's portrait

Only the files above are in `assets/sprites/`. The full downloaded packs sit
in `assets/` but are git-ignored to keep the repo small. Those packs are
Ninja Adventure, the Super Pixel Effects Gigapack (free version, which needs
credit if used) and Evil Wizard 2 (planned for the boss).

## Generative-AI disclosure

Written with help from Claude (Anthropic), used with instructor permission for
this project.
