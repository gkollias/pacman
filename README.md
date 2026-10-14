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
