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

- Menus: single player / host (server) / join (client), then difficulty and speed.
- Up to 4 players on a LAN (TCP port 10000). The maze is split into territories; your dots are
  yellow, teammates' are red. The team wins when every dot is eaten.
- Keys: arrows move, `P` pause (everyone pauses, each player rejoins with `P`), `Esc` quit.
  Multiplayer: `H`/`G` send/get a ghost, `D`/`F` send/get a dot, `C`/`V` send/get a power dot
  (to/from the next player).
- When a player loses or leaves, their dots are shared among the others; if the server goes
  away, clients finish the game on their own.
- `make test` runs the game-logic tests (including multiplayer, over an in-memory network).
