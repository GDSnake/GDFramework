# SDL Framework Samples

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