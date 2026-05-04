# Battle Royale (C++ / OpenGL)

5-step build of a 100-player battle royale.

This repo is currently at **Step 1: Core 3D Environment & Player Controls**.

## Dependencies (Debian / Raspberry Pi OS)

```sh
sudo apt-get install -y build-essential cmake \
    libglfw3-dev libglew-dev libglm-dev
```

(GLFW 3.3, GLEW 2.2, GLM 0.9.9 — all in the default Bookworm repos.)

## Build

```sh
cmake -S . -B build
cmake --build build -j
```

Produces `build/battle_royale`.

## Run

```sh
./build/battle_royale
```

## Controls (Step 1)

| Key       | Action                                |
|-----------|---------------------------------------|
| WASD      | Walk                                  |
| Shift     | Sprint                                |
| Space     | Jump                                  |
| Mouse     | Look                                  |
| V         | Toggle FPS / TPS camera               |
| F5        | Debug: take 10 damage (wraps at 0)    |
| Esc       | Quit                                  |

## Project layout

```
fps_game/
├── CMakeLists.txt
├── shaders/        world.{vert,frag}, ui.{vert,frag}
└── src/
    ├── main.cpp
    ├── Game.{h,cpp}      window + main loop
    ├── Shader.{h,cpp}    GL program wrapper
    ├── Camera.{h,cpp}    FPS/TPS camera
    ├── Player.{h,cpp}    movement + gravity
    ├── World.{h,cpp}     ground + obstacle boxes
    └── UI.{h,cpp}        placeholder HUD (quads only)
```
