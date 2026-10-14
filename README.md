# pacman

## Running on macOS

The source is in `Pacman/`. `pacman executable/` is a later, prebuilt-only Windows version
(no source). The original game uses Allegro 4 on Windows;
`Pacman/macos/` provides a small Allegro 4 compatibility layer built on SDL3.

```bash
brew install sdl3
cd Pacman && make run
```

Controls: arrow keys to move, `P` to pause, `Esc` to quit.
The experimental Winsock LAN code in `main.cpp` is only compiled on Windows.

## Pacman2: LAN edition (rebuilt)

`Pacman2/` rebuilds the later version that only survived as `pacman executable/pacman.exe`,
using that build's grid levels (`init/Grid/Grid_N.txt`), graphics and sounds. It shares the
SDL3 compatibility layer in `Pacman/macos/`.

```bash
cd Pacman2 && make run
```

To build a double-clickable `Pacman2/Pacman.app` (self-contained: SDL3 and all assets are bundled,
ad-hoc signed, Apple Silicon):

```bash
cd Pacman2 && make app
```

Hosting a LAN game makes macOS ask once whether Pacman may accept incoming connections.

- Menus: single player / host (server) / join (client), then difficulty and speed.
- Up to 4 players on a LAN (TCP port 10000). The maze is split into territories; your dots are
  yellow, teammates' are red. The team wins when every dot is eaten.
- Keys: arrows move, `P` pause (everyone pauses, each player rejoins with `P`), `Esc` quit.
  Multiplayer: `H`/`G` send/get a ghost, `D`/`F` send/get a dot, `C`/`V` send/get a power dot
  (to/from the next player).
- When a player loses or leaves, their dots are shared among the others; if the server goes
  away, clients finish the game on their own.
- Optional features (Options menu, all on by default; the host's choice applies to everyone):
  - moving obstacles: free-standing blocks of the maze slide as a whole to close one corridor
    and open another, then slide back; they wait rather than crush anyone (blocks, directions
    and timings in `init/moving_walls.txt`);
  - running dots: power dots flee from Pac-Man (pushing past other dots), and ordinary dots flee
    into corridors you have already cleared, so the last dots of a level are slippery;
  - terrains (LAN): the side tunnel leads into the next player's maze, where their ghosts
    hunt you; ghosts can wander over too;
  - cornered (LAN): when ghosts block every way out, the nearest one is sent to the next player.
- `make test` runs the game-logic tests (including multiplayer, over an in-memory network).
