
# Renderer

This is a CPU Renderer that renders objects using the CPU , you can use either a **ray tracer** or a **rasterizer** to render models, i made this project to learn the graphics pipeline and Windows API , although now it uses FSWindow , a windowing library i made for linux and windows. This project supports rendering OBJ files, with basic lighting (blinn-phong) and includes features for visualization and debugging.

### Controls
- **W, A, S, D**: Move the camera.
- **Space/CTRL**: Move the camera up/down.
- **SHIFT**: Increase moving speed.
- **Mouse**: Rotate the camera.
- **Key Bindings**:
  - `R`: Toggle ray tracing/rasterization.
  - `X`: Change lighting modes
  - `G`: Lock/unlock the mouse.
  - `N`: set debugstate to normal mode
  - `T`: set debugstate to wireframe mode
  - `V`: disable debugstate.
  - `M`: set debugstate to depth mode
  - `B`: Show/hide bounding boxes.
  - `C`: Toggle backface culling.
  - `P`: Export the current frame to a PPM file.
  - `Q`: Reset camera position and rotation.
  - `L`: Display model details.
  - `F`: Toggle FXAA anti-aliasing.
  - `ESC`: Exit the application.

## Building from source
> **Note** : you can also use g++ & gcc  or Visual Studio but clang will have least problems as i use it for development 
### Windows
- install clang, cmake and ninja <br>
```batch
winget install cmake
winget install LLVM.LLVM
winget install Ninja-build.Ninja

```
- clone the repo. <br>
```batch
git clone --recursive https://github.com/fahad-cpp/Renderer Renderer
cd Renderer

```
- build using cmake (clang)
```batch
cmake --preset clang-release
cmake --build --preset clang-release
bin\Renderer.exe

```
(use clang-debug for debug build)

other presets that may be available on windows:
```
gcc-debug/gcc-release # gcc compiler with ninja
vs-debug/vs-release # MSVC compiler with Visual Studio
```
to check available presets, run:
```batch
cmake --list-presets
```

### Linux
- install clang, cmake and ninja using your package manager <br>
```bash
sudo pacman -S clang ninja cmake lld

```
- clone the repo
```bash
git clone --recursive https://github.com/fahad-cpp/Renderer Renderer
cd Renderer

```
- build using cmake (clang)
```batch
cmake --preset clang-release
cmake --build --preset clang-release
./bin/Renderer

```
(use clang-debug for debug build)

other presets that may be available on linux:
```
gcc-debug/gcc-release # gcc compiler with ninja
```
to check available presets, run:
```batch
cmake --list-presets
```
> **Note:** <br> on Wayland systems the mouse locking does not work so mouse will go out of window <br> disable mouse lock by pressing G.<br> you can also move camera using arrow keys

## Sample Outputs

### Ray Tracing
- **White King**: A ray-traced render of a chess piece.
![WhiteKing](https://github.com/user-attachments/assets/8c97e116-1da8-4f69-be0b-e1772792075a)

- **Spheres**: Reflection on spheres rendered using ray tracing.
![RayTracerReflection](https://github.com/user-attachments/assets/a848f865-3c41-4b84-8db5-720a3365d647)

### Rasterization
- **Demon Skull Normals**: Render time : ~8ms <br>
![DemonSkullDepth](res/showcase/DemonSkullNormal.png)

- **Demon Skull**: Render time : ~8ms <br>
![DemonSkullLightBr](res/showcase/DemonSkull.png)

- **Sponza**: Render time : ~40ms <br>
![DemonSkullLightBr](res/showcase/Sponza.png)
