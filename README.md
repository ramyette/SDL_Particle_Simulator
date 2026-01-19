# SDL Particle Simulator

A particle simulator program written in **C using SDL2** on Windows.

Features:
- Spawn thousands of individual particles
- Interact by dragging mouse

Plans:
- Particle interactions
- Color change based on velocity
- Particle spawning / lifespan

---

## Controls

- **Left Click + Drag**  
  Interact with particle field

- **Escape**
  Close program

---

## Build (MinGW + SDL2)

You can just download the latest release instead of building, but if you prefer building:

Example Windows make command:
mingw32-make all

Make sure `SDL2.dll` is in the same directory as `ParticleSimulator.exe` for the exe to run.

If you have SDL2 installed on another OS, compile with: 
**Linux:**
gcc -o ParticleSimulator main.c $(pkg-config --cflags --libs sdl2)
(have not tested on Linux)
**MacOS:**
clang main.c -o ParticleSimulator `sdl2-config --cflags --libs`

---

## Dependencies

- SDL2  
- MinGW-w64 (Windows)
