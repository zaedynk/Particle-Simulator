# Particle Simulator

A real-time GPU particle simulator written in C++ and OpenGL. It pushes a million
particles through a compute shader every frame and lets you fly a camera through them
and pull them around with the mouse.

![Particles](https://img.shields.io/badge/particles-1%2C000%2C000-blue) ![OpenGL](https://img.shields.io/badge/OpenGL-4.3-5586A4) ![License](https://img.shields.io/badge/license-MIT-green)

## How it works

Every particle's position, velocity, and color lives in a [shader storage buffer](https://www.khronos.org/opengl/wiki/Shader_Storage_Buffer_Object)
on the GPU. Each frame:

1. A **compute shader** updates every particle in parallel. While you hold the mouse
   button, particles are pulled toward a point in space using Newton's law of gravity,
   with a little drag so the motion settles instead of exploding.
2. The same buffers are then drawn directly as points by the **render shader**, colored
   by how fast each particle is moving (slow = blue, fast = green/red).

Because the data never leaves the GPU, the simulation stays fast even with a million particles.

## Controls

| Input | Action |
| --- | --- |
| **Hold left click** | Pull particles toward the cursor |
| **W A S D** | Move the camera |
| **Space / Ctrl** | Move the camera up / down |
| **Shift** | Move faster |
| **T** | Pause / resume the simulation |
| **Esc** | Toggle mouse look (turn the camera with the mouse) |

## Requirements

- A GPU and drivers supporting **OpenGL 4.3** (for compute shaders)
- **CMake** 3.10+
- A **C++20** compiler (Visual Studio 2019/2022, or GCC/Clang)
- [**vcpkg**](https://github.com/microsoft/vcpkg) for dependencies

## Build & run

### 1. Install the dependencies with vcpkg

```bash
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh      # Windows: .\bootstrap-vcpkg.bat
./vcpkg install glew glfw3 glm
```

### 2. Build the project

```bash
git clone https://github.com/zaedynk/Particle-Simulator
cd Particle-Simulator/ParticleSimulator

cmake -B build -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
```

Replace `/path/to/vcpkg` with the location of your vcpkg checkout.

### 3. Run

```bash
./build/bin/ParticleSimulator        # Windows: .\build\bin\Release\ParticleSimulator.exe
```

> **Tip:** A million particles is demanding. If it runs slowly, lower `PARTICLE_COUNT`
> near the top of [`main.cpp`](ParticleSimulator/main.cpp) and rebuild.

> **Visual Studio users:** You don't need a separate solution file — open the
> `ParticleSimulator` folder with *File → Open → Folder* and Visual Studio will
> configure the CMake project automatically (set the vcpkg toolchain in CMake settings).

## Project layout

| File | Purpose |
| --- | --- |
| `main.cpp` | Window setup, buffer creation, and the main render loop |
| `input.h` / `input.cpp` | Camera movement and mouse interaction |
| `shaders.h` / `shaders.cpp` | Shader compilation plus the GLSL source (compute / vertex / fragment) |
| `stb_easy_font.h` | Tiny public-domain header used to draw the on-screen hints |
| `CMakeLists.txt` | Cross-platform build configuration |

## Dependencies

| Library | Role |
| --- | --- |
| [OpenGL](https://www.opengl.org/) | Core rendering and compute API |
| [GLEW](https://glew.sourceforge.net/) | Loads modern OpenGL functions |
| [GLFW](https://www.glfw.org/) | Window creation and input |
| [GLM](https://github.com/g-truc/glm) | Vector and matrix math |
| [stb_easy_font](https://github.com/nothings/stb) | On-screen text (vendored, no install needed) |

## License

Released under the MIT License — see [LICENSE](LICENSE).
