
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

## Installation

### Prerequisites
- **Operating System**: Windows / Linux.
- **Installing Dependencies**:
  - clang, cmake, ninja and lld
  - for linux :
    ```bash
    sudo pacman -S cmake ninja clang lld
    ```
  - for Windows :
    ```batch
    winget install KitWare.CMake
    winget install Ninja-build.Ninja
    winget install LLVM.LLVM
    ```

### Build Instructions
Clone the repository recursively:
```batch
git clone --recursive https://github.com/fahad-cpp/Renderer.git
cd Renderer
```
#### Using Cmake
##### Generating build files
```batch
mkdir build
cd build
cmake ..

```
##### Building and running
```batch
cmake --build . --config Release
Release\Renderer.exe

```
#### Or run the script
```batch
build.bat 

```
OR
```bash
chmod +x build.sh
./build.sh
```

### Running the Application
After building, run the executable from the `bin` directory:
```bash
cd bin
./Renderer
```

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