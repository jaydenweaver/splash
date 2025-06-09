# Splash

A terminal-based fluid simulation powered by **Smoothed Particle Hydrodynamics (SPH)** and rendered using ASCII art via `ncurses`. Interact with the fluid in real-time by clicking to trigger dynamic explosions within the simulation grid.

---

## Build Instructions

**Requirements:**

- C++17-compatible compiler  
- `ncurses` development library (e.g., `libncurses-dev` on Ubuntu/Debian)

**Compile with:**

`g++ -Isrc/include -o build/splash src/splash.cpp src/renderer.cpp src/phys.cpp -lncurses`

---

## Usage

Run the compiled binary:

`./build/splash`

**Controls:**

- **Left mouse click**: triggers an explosion at the clicked position  
- **q**: quits the simulation

---

## Project Structure

splash/
├── build/ # Output directory for binaries
├── src/
│ ├── splash.cpp # Main loop and input handling
│ ├── phys.cpp # SPH simulation logic
│ ├── renderer.cpp # ASCII renderer using ncurses
├── include/
│ └── phys.h # Shared constants and structures
└── README.md

---

## Notes

- The simulation operates on a 500×200 high-resolution grid. Terminal output is automatically downsampled to fit your terminal window.
- Rendering is pure ASCII; higher terminal resolution = higher visual fidelity.
- Entire simulation runs on the CPU with no hardware acceleration.

---

## Physics Model

The simulation implements a simplified 2D **Smoothed Particle Hydrodynamics** (SPH) framework:

- **Density Estimation**: Poly6 smoothing kernel based on particle distance squared  
- **Pressure Force**: Derived from a pressure gradient using the SPH pressure formula  
- **Viscosity**: Models fluid internal friction with a simple linear viscosity kernel  
- **Gravity**: Constant downward force applied to all particles  
- **Integration**: Explicit Euler integration for velocity and position updates  
- **Boundaries**: Reflective edge collisions with dampening  
- **Explosions**: Radial force fields applied to particles within a blast radius when clicked

---

## Troubleshooting

- If explosion effects don't align with clicks, try adjusting your terminal size or tweaking scaling calculations.
- If it's laggy, reduce `PARTICLE_COUNT` or the grid resolution in `phys.h`.
- Ensure your terminal supports mouse input and that you’ve installed `ncurses`.

---

## License

MIT License — free to use, modify, and distribute.

---

## Author

Developed by Jayden Weaver
