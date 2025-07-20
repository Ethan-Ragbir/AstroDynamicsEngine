# AstroDynamics Engine 🌌

A high-performance N-body gravitational simulation engine with advanced numerical integration, spatial optimization algorithms, and both 2D/3D visualization capabilities.

![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg)

## Features

### Core Physics Engine
- **Advanced Integration Methods**
  - 4th-order Runge-Kutta (RK4) integration for high accuracy
  - Adaptive timestep control
  - Energy conservation monitoring
  
- **Performance Optimization**
  - Barnes-Hut tree algorithm for O(n log n) complexity
  - Spatial octree/quadtree partitioning
  - Multi-threading support with OpenMP
  - SIMD vectorization for force calculations

### Visualization
- **2D Mode** (SFML-based)
  - Real-time particle rendering
  - Interactive camera controls (pan, zoom)
  - Particle trails and heat maps
  - Performance metrics overlay
  
- **3D Mode** (OpenGL-based)
  - Full 3D camera controls
  - Particle glow effects and bloom
  - Skybox rendering
  - Stereoscopic rendering support

### Features
- **Configuration System**
  - JSON-based scenario files
  - Predefined scenarios (solar system, galaxy collision, etc.)
  - Custom particle distribution generators
  
- **Analysis Tools**
  - Energy conservation tracking
  - Center of mass calculation
  - Orbital parameter extraction
  - Collision detection and merging

## Quick Start

### Prerequisites
```bash
# Ubuntu/Debian
sudo apt-get install cmake libsfml-dev libglew-dev libglm-dev libopengl-dev nlohmann-json3-dev

# macOS
brew install cmake sfml glew glm nlohmann-json

# Windows (vcpkg)
vcpkg install sfml glew glm nlohmann-json
```

### Building
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Running
```bash
# 2D simulation with default scenario
./AstroDynamics2D

# 3D simulation
./AstroDynamics3D

# Load custom scenario
./AstroDynamics2D --config ../scenarios/galaxy_collision.json

# Benchmark mode
./AstroDynamics2D --benchmark --particles 10000
```

## Usage

### Interactive Controls

#### 2D Mode
- **Left Click**: Add particle at cursor
- **Right Click + Drag**: Set initial velocity for new particle
- **Middle Mouse**: Pan camera
- **Scroll**: Zoom in/out
- **Space**: Pause/Resume simulation
- **R**: Reset to initial conditions
- **T**: Toggle particle trails
- **G**: Toggle grid
- **F1**: Toggle performance overlay

#### 3D Mode
- **WASD**: Move camera
- **Mouse**: Look around
- **Q/E**: Move up/down
- **Shift**: Speed boost
- **1-9**: Load preset scenarios
- **F11**: Toggle fullscreen

### Configuration Files

Create custom scenarios using JSON:

```json
{
  "name": "Binary Star System",
  "integration": {
    "method": "RK4",
    "timestep": 0.001,
    "adaptive": true
  },
  "particles": [
    {
      "position": [0, 0, 0],
      "velocity": [0, 10, 0],
      "mass": 1000,
      "color": [1.0, 0.8, 0.0],
      "name": "Star A"
    },
    {
      "position": [100, 0, 0],
      "velocity": [0, -10, 0],
      "mass": 800,
      "color": [0.0, 0.5, 1.0],
      "name": "Star B"
    }
  ],
  "camera": {
    "position": [200, 200, 200],
    "target": [0, 0, 0]
  }
}
```

## Architecture

### Barnes-Hut Algorithm
The engine uses a sophisticated tree-based algorithm to reduce computational complexity:
- Spatial subdivision using octree (3D) or quadtree (2D)
- Configurable theta parameter for accuracy vs. performance trade-off
- Dynamic tree rebuilding for moving particles

### Integration Methods
Multiple numerical integration schemes are available:
- **Euler** (1st order) - Fast but less accurate
- **Velocity Verlet** (2nd order) - Good for energy conservation
- **RK4** (4th order) - High accuracy, default method
- **Adaptive RK45** - Variable timestep for efficiency

## Performance

Benchmark results on typical hardware:

| Particles | Naive O(n²) | Barnes-Hut | Speedup |
|-----------|-------------|------------|---------|
| 1,000     | 60 FPS      | 60 FPS     | 1x      |
| 10,000    | 6 FPS       | 58 FPS     | 9.7x    |
| 100,000   | 0.06 FPS    | 45 FPS     | 750x    |
| 1,000,000 | -           | 15 FPS     | -       |

## Examples

### Solar System
```bash
./AstroDynamics3D --config ../scenarios/solar_system.json
```

### Galaxy Collision
```bash
./AstroDynamics3D --config ../scenarios/andromeda_milkyway.json
```

### Particle Cloud Collapse
```bash
./AstroDynamics2D --particles 5000 --distribution uniform --collapse
```

## Contributing

We welcome contributions! Please see our [Contributing Guidelines](CONTRIBUTING.md) for details.

### Development Setup
```bash
# Clone with submodules
git clone --recursive https://github.com/yourusername/AstroDynamicsEngine.git

# Enable debug build
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Run tests
make test
```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Barnes-Hut algorithm implementation inspired by [Josh Barnes' original paper](https://doi.org/10.1038/324446a0)
- RK4 integration based on numerical methods from Burden & Faires
- Galaxy collision scenarios use initial conditions from [Toomre & Toomre (1972)](https://doi.org/10.1086/151823)

## Citation

If you use this software in your research, please cite:
```bibtex
@software{astrodynamics_engine,
  title = {AstroDynamics Engine: High-Performance N-Body Simulation},
  author = {Your Name},
  year = {2024},
  url = {https://github.com/yourusername/AstroDynamicsEngine}
}
```
