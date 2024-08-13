# Particle Simulator

Particle Simulator is an interactive OpenGL application designed to simulate and render a large number of particles in real-time. The program allows users to interact with the particles using both mouse and keyboard inputs.

## Features

- Real-time particle simulation and rendering
- Interactive control over particle movement and simulation state
- Camera movement for exploring the simulation space

## Dependencies

This project relies on the following libraries, managed using [vcpkg](https://github.com/microsoft/vcpkg):

- **OpenGL**: Core rendering API
- **GLEW**: OpenGL Extension Wrangler Library
- **GLFW**: Library for handling window and input events
- **GLM**: OpenGL Mathematics library
- **GLUT**: For rendering bitmap text

## Getting Started

### Prerequisites

- **CMake** 3.10 or higher
- **vcpkg**: Package manager for installing dependencies
- **Visual Studio 2019/2022** or a similar C++20 compliant compiler

### Installing Dependencies

1. **Clone the vcpkg repository**:

   ```bash
   git clone https://github.com/microsoft/vcpkg.git
   cd vcpkg
   ```

2. **Bootstrap vcpkg**:

   - **On Windows**:

     ```bash
     .\bootstrap-vcpkg.bat
     ```

   - **On Unix**:

     ```bash
     ./bootstrap-vcpkg.sh
     ```

3. **Install required packages**:

   ```bash
   .\vcpkg install glew:x64-windows glfw3:x64-windows glm:x64-windows
   ```

### Building the Project

1. **Clone the Particle Simulator repository**:

   ```bash
   git clone https://github.com/zaedynk/Particle-Simulator
   cd ParticleSimulator/ParticleSimulator
   ```

2. **Create a build directory**:

   ```bash
   mkdir build
   cd build
   ```

3. **Configure the project with CMake**:

   ```bash
   cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
   ```

   Replace `C:/path/to/vcpkg` with the actual path to your vcpkg installation.

4. **Build the project**:

   ```bash
   cmake --build .
   ```

### Running the Application

After building, navigate to the `bin` directory inside your build folder and run the `ParticleSimulator.exe`:

```bash
cd bin
ParticleSimulator.exe
```

## Instructions

- **Hold Left Click**: Move particles toward the mouse position.
- **Hit T**: Pause or resume the simulation.
- **WASD**: Move the camera around the scene.
- **Escape**: Lock/hide the mouse for immersive camera movement.

## Project Structure

- `Main.cpp`: Entry point for the application
- `InputManager.cpp`: Handles user input
- `Shader.cpp`: Manages shader programs
- `ComputeShader.glsl`: Computes particle dynamics
- `CMakeLists.txt`: Build configuration file

## Contributing

Contributions are welcome! Please fork the repository and submit a pull request for any features or bug fixes you would like to add.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

This project uses open-source libraries provided by the community. Special thanks to the contributors of GLEW, GLFW, GLM, and FreeGLUT.
