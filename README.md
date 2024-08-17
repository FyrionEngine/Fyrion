# Fyrion Engine

[![The MIT License][license-image]][license-url]
[![Language](https://img.shields.io/badge/language-C++-blue.svg)](https://isocpp.org/)

Fyrion is an in development open-source MIT licensed multi-platform 2D and 3D game engine   

[license-image]: https://img.shields.io/badge/License-MIT-yellow.svg
[license-url]: https://opensource.org/licenses/MIT

![Scene](Docs/Images/Sponza.png)


## Important Note:

>Fyrion is under constant development and is not a stable tool, it lacks important features and the API can change drastically, so it should not be used for production.

### Principles
- Easy to use, simple and straightforward API, no hidden or complicated structures, no complicated configuration steps
- Small and lightweight, small footprint libraries are carefully selected to be used in the project.
- Easy to build, no extra steps required to build the engine, just clone and build, pre-built shared libraries are only allowed if dynamically loaded 
- Editable, features should be replaced without editing a single engine LoC.
- Full API control, external libraries and STL types are not recommended in public API header files.
- Use it as a full engine with editor, or framework, importing the library to the project.   

### Current Features
* Scene Object System
* Asset system
* Custom STL types
* 3D Renderer with PBR, Cascade Shadow Maps and HDR Sky Rendering
* Editor
* Multiplatform (Windows, Linux, macOS)
* Fully customizable Render Graph
* HLSL Shaders
* Vulkan RHI
* Runtime reflection system
* Asset and Shader hot-reload


### Working in progress
* C++ scripting using plugin pattern (no need to recompile the engine)
* Start/Stop Simulation
* 3D Physics using Jolt Physics
* Export game


### Planned features
* Animation system
* Mobile device support (Android, iOS).
* 2D Renderer 
* 2D Physics
* Audio
* Game UI
* And more...

### Building
Fyrion uses cmake to build, all external libraries are included in the build system and compiles together with the engine for each platform
but some platforms like linux might require some library installation like xorg-dev,  libglu1-mesa-dev and libgtk-3-dev

```
git clone https://github.com/FyrionEngine/Fyrion.git
cd Fyrion
mkdir Build
cd Build
cmake ..
cmake --build .
```

---
### License
Fyrion is licensed under MIT License.