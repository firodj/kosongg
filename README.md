# KosongG


## Development

Run python script:

```
$ python main.py
```

Goto build folder:

```
$ mkdir -p output/build
$ cmake -S output -B output/build
$ cmake --build output/build
```
## Notes

Alternate CMake using FetchContent.

```
include(FetchContent)
# git clone --branch release-2.28.5 --depth=1 https://github.com/libsdl-org/SDL.git sdl2
FetchContent_Declare(
  sdl2
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG        15ead9a40d09a1eb9972215cceac2bf29c9b77f6 # release-2.28.5
)
# git clone --branch docking --depth=1 https://github.com/ocornut/imgui.git imgui-docking
FetchContent_Declare(
  imgui-docking
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        82eeafc1961557ab166fd588cee139db1f2a87cf # imgui-docking
)

set(SDL2_DIR ${sdl2_SOURCE_DIR})
set(IMGUI_DIR ${imgui-docking_SOURCE_DIR})

```
