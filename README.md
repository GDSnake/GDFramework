# SDL Boilerplate

This is a small SDL3 starting point. CMake downloads the SDL3 extensions and nlohmann/json when they are not already available under `DefaultGame/libs`.

Open the workspace in VS Code with CMake Tools installed, then configure and build the `DefaultGame` project. On Windows, the CMake setup supports MSVC and MinGW-compatible GCC or Clang toolchains.

The default `config.json` uses the existing flat window settings. Nested `window` settings are also accepted. Optional `images`, `fonts`, and `audio` objects can map resource names to paths; no resource sections are required for the starter to run.

`Game` provides an overridable event, update, and render loop. Add project code under `DefaultGame/source`; source files are picked up automatically by CMake.
