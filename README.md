
# Renderer

**Renderer** is a C++ application for rendering 3D models using either a **ray tracer** or a **rasterizer**, a project to learn the graphics pipeline and Windows API. It supports rendering OBJ files, with basic lighting and includes features for visualization and debugging.

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
- **Surface Normal Coloring**: Visualization of surface normals.
- ![DemonSkullDepth](https://github.com/user-attachments/assets/39b3cd52-3e36-4013-ad2f-3eac4b3d6e59)

- **Wireframe Mode**: A triangle-based wireframe view.
- ![DemonSkullTris](https://github.com/user-attachments/assets/cfa0e260-9c0b-42bb-b6ee-a1fb85195104)

- **Single Colour**: Same model rendered in single colour with multiple light soources.
-  ![DemonSkullLightBr](https://github.com/user-attachments/assets/8087e7b8-26d6-40c9-92ab-f3ff4738c997)

## Acknowledgments
- **Gabriel Gambetta**: For inspiration through *Computer Graphics From Scratch*.
