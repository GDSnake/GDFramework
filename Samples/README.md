# GDFramework Samples

These standalone samples use the framework's `Renderer`, `Sprite`, and `SpriteManager` without modifying the `DefaultGame` sources or build file.

## Build

From this directory, configure and build with CMake and Ninja:

```powershell
cmake --preset samples-ninja
cmake --build --preset debug
```

The executables and their configs are placed in separate runtime directories under `out/build/samples-ninja/bin`.

## Samples

- `SampleSquare` draws a static square using SDL's renderer.
- `SampleImageViewer` loads a named image through `SpriteManager` and displays it.
- `SampleSceneChange` provides a starter scene for implementing scene transitions.
- `SampleScreenScrolling` demonstrates a scrolling car-racing scene with sprite sheets.
- `SampleParticlesDemo` follows the mouse with a 16x16 flame frame, textured sparks, and pulsing yellow-orange squares. Its entity holds 128 stable handles while `ParticleSystem` owns and advances the particle data.

The square uses SDL rendering rather than an OpenGL context.

## Controls

### SampleSquare

- `Escape` quits the sample.

### SampleImageViewer

- `1`-`9` or keypad `1`-`9` selects one of the nine blocks in matching order.
- `W`/`A`/`S`/`D` or the arrow keys move the selected block.
- `Escape` quits the sample.

### SampleSceneChange

- `Escape` quits the sample.

### SampleParticlesDemo

- Move the mouse to move the flame emitter and leave a particle trail.
- `Escape` quits the sample.
- Build target: `SampleParticlesDemo`. Run it from its output directory, `out/build/samples-ninja/bin/SampleParticlesDemo/Debug`, so the copied config and assets are found.
- `--smoke-test <output-prefix>` runs three mouse positions, checks the particle pool and rendered pixels, saves three PNG captures, and exits after checking cleanup.

## Asset Credits

- Road tileset: nodeturtle, [Free Basic Car Pack and Tilemap](https://nodeturtle.itch.io/free-basic-car-pack-and-tilemap).
- Car sprites: marcusvh, [2D Cars](https://marcusvh.itch.io/2d-cars).
- Flame and spark sprites in `SampleParticlesDemo`: [bdragon1727](https://bdragon1727.itch.io).