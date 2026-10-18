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

The square uses SDL rendering rather than an OpenGL context.

## Controls

### SampleSquare

- `Escape` quits the sample.

### SampleImageViewer

- `1`-`9` or keypad `1`-`9` selects one of the nine blocks in matching order.
- `W`/`A`/`S`/`D` or the arrow keys move the selected block.
- `Escape` quits the sample.