
# Renderer

**Renderer** is a C++ application for rendering 3D models using either a **ray tracer** or a **rasterizer**, a project to learn the graphics pipeline and Windows API. It supports rendering OBJ files, with basic lighting and includes features for visualization and debugging.

This project is inspired by Gabriel Gambetta's *Computer Graphics From Scratch* and includes custom implementations.

## Features

### Rendering Modes
- **Ray Tracing**: Provides high-quality rendering but is slow for high-poly models.
- **Rasterization**: Optimized for real-time rendering with support for various visual debugging features.

### Supported File Formats
- **OBJ**: 3D models (Material/MTL support planned for future updates).

### Visual Features
- **Lighting**: Simple light rendering.
- **Wireframe Mode**: Toggle for edge-based rendering.
- **Anti-Aliasing**: FXAA for smoother edges.
- **Bounding Boxes**: Visualize object bounds.
- **Backface Culling**: Toggle to improve performance by discarding non-visible polygons.

### Controls
- **W, A, S, D**: Move the camera.
- **Space/CTRL**: Move the camera up/down.
- **SHIFT**: Increase moving speed.
- **Mouse**: Rotate the camera.
- **Key Bindings**:
  - `R`: Toggle ray tracing/rasterization.
  - `G`: Lock/unlock the mouse.
  - `V`: Toggle wireframe mode.
  - `B`: Show/hide bounding boxes.
  - `C`: Toggle backface culling.
  - `P`: Export the current frame to a PPM file.
  - `Q`: Reset camera position and rotation.
  - `L`: Display model details.
  - `F`: Toggle FXAA anti-aliasing.
  - `ESC`: Exit the application.

## Installation

### Prerequisites
- **Operating System**: Windows (other platforms not yet supported).
- **Dependencies**:
  - C++ compiler and/or CMake

### Build Instructions
Clone the repository:
```batch
git clone https://github.com/fahad-cpp/Renderer.git
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

### Running the Application
After building, run the executable from the `bin` directory:
```bash
cd bin
Renderer.exe

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


## Planned Features
- **Material and Texture Support**: Import and render MTL files.
- **Performance Optimizations**: Acceleration structures (e.g., BVH) for ray tracing.
- **Cross-Platform Support**: Expand compatibility to Linux and macOS.

## Contributing
Contributions are welcome! To get started:
1. Fork the repository.
2. Create a new branch (`git checkout -b feature-name`).
3. Commit changes (`git commit -m "Description"`).
4. Push to the branch (`git push origin feature-name`).
5. Open a pull request.

## Acknowledgments
- **Gabriel Gambetta**: For inspiration through *Computer Graphics From Scratch*.
