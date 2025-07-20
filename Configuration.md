# AstroDynamics Engine Configuration Guide

## Table of Contents
- [Scenario File Format](#scenario-file-format)
- [Particle Properties](#particle-properties)
- [Simulation Settings](#simulation-settings)
- [Camera Configuration](#camera-configuration)
- [Advanced Features](#advanced-features)
- [Performance Tuning](#performance-tuning)

## Scenario File Format

Scenarios are defined in JSON format and loaded from the `scenarios/` directory. Here's the basic structure:

```json
{
  "name": "Scenario Name",
  "description": "Brief description",
  "particles": [...],
  "settings": {...},
  "camera": {...}
}
```

## Particle Properties

Each particle in the `particles` array has the following properties:

| Property | Type | Description | Default |
|----------|------|-------------|---------|
| `position` | [float, float] | Initial position [x, y] | Required |
| `velocity` | [float, float] | Initial velocity [vx, vy] | Required |
| `mass` | float | Particle mass (affects gravity) | Required |
| `color` | [int, int, int] | RGB color values (0-255) | [255, 255, 255] |
| `name` | string | Display name for the particle | "" |
| `fixed` | boolean | If true, particle won't move | false |

### Example Particle:
```json
{
  "position": [400, 300],
  "velocity": [0, 20],
  "mass": 1000.0,
  "color": [255, 255, 0],
  "name": "Sun",
  "fixed": true
}
```

## Simulation Settings

The `settings` object controls the physics simulation:

| Setting | Type | Description | Default |
|---------|------|-------------|---------|
| `gravitational_constant` | float | Strength of gravity (G) | 6.67430e-2 |
| `time_step` | float | Integration time step (dt) | 0.01 |
| `softening` | float | Prevents singularities | 1.0 |
| `trail_length` | int | Number of trail points | 100 |
| `adaptive_timestep` | boolean | Enable adaptive time stepping | false |
| `min_dt` | float | Minimum time step (adaptive) | 0.0001 |
| `max_dt` | float | Maximum time step (adaptive) | 0.1 |

### Gravitational Constant Guidelines:
- **1e-3 to 1e-2**: Galaxy-scale simulations
- **1e-2 to 1e-1**: Solar system scale
- **1e-1 to 1**: Close binary systems
- **1 to 10**: Visualization-focused (less realistic)

### Time Step Guidelines:
- Smaller values = more accurate but slower
- Rule of thumb: `dt < 0.01 * (smallest orbit period)`
- For stable orbits: `dt < 0.01`
- For colliding systems: `dt < 0.001`

## Camera Configuration

The `camera` object sets the initial view:

```json
"camera": {
  "center": [400, 300],
  "zoom": 1.5
}
```

- `center`: [x, y] coordinates of camera center
- `zoom`: Zoom level (1.0 = default, <1 = zoomed in, >1 = zoomed out)

## Advanced Features

### 1. Creating Stable Orbits

For circular orbits, use: `v = sqrt(G * M / r)`

Where:
- `v` = orbital velocity
- `G` = gravitational constant
- `M` = mass of central body
- `r` = orbital radius

Example:
```json
{
  "position": [500, 300],
  "velocity": [0, 31.6],
  "mass": 10.0,
  "note": "For G=0.1, M=1000, r=100: v=sqrt(0.1*1000/100)=31.6"
}
```

### 2. Lagrange Points

Create a restricted three-body system:

```json
{
  "name": "Earth-Moon L4 Point",
  "particles": [
    {
      "position": [400, 300],
      "velocity": [0, 0],
      "mass": 5000.0,
      "name": "Earth",
      "fixed": true
    },
    {
      "position": [480, 300],
      "velocity": [0, 40],
      "mass": 50.0,
      "name": "Moon"
    },
    {
      "position": [440, 346.4],
      "velocity": [-20, 20],
      "mass": 0.1,
      "name": "L4 Object",
      "color": [255, 0, 0]
    }
  ]
}
```

### 3. Binary Systems

For equal-mass binaries orbiting their barycenter:

```json
{
  "particles": [
    {
      "position": [350, 300],
      "velocity": [0, -25],
      "mass": 1000.0
    },
    {
      "position": [450, 300],
      "velocity": [0, 25],
      "mass": 1000.0
    }
  ]
}
```

## Performance Tuning

### For Large N (>1000 particles):

1. **Reduce trail length**:
   ```json
   "trail_length": 20
   ```

2. **Increase time step** (carefully):
   ```json
   "time_step": 0.02
   ```

3. **Increase softening**:
   ```json
   "softening": 2.0
   ```

4. **Use adaptive time stepping**:
   ```json
   "adaptive_timestep": true,
   "min_dt": 0.001,
   "max_dt": 0.05
   ```

### Benchmarking Settings:

For performance testing, use this minimal configuration:

```json
{
  "settings": {
    "trail_length": 0,
    "softening": 5.0,
    "time_step": 0.05
  }
}
```

## Tips and Tricks

1. **Color Coding**: Use consistent colors for particle types
   - Yellow/Orange: Stars
   - Blue: Rocky planets
   - Red/Orange: Gas giants
   - White/Gray: Moons/asteroids

2. **Mass Ratios**: Keep realistic for stable systems
   - Sun : Earth ≈ 333,000 : 1
   - Earth : Moon ≈ 81 : 1
   - Binary stars: 0.5 : 1 to 2 : 1

3. **Initial Velocities**: 
   - For circular orbits: perpendicular to radius
   - For elliptical: vary angle and magnitude
   - For escape: v > sqrt(2GM/r)

4. **Debugging Unstable Systems**:
   - Check if particles are too close (increase softening)
   - Verify orbital velocities are reasonable
   - Reduce time step
   - Check mass values aren't extreme

## Example: Multi-Planet System

```json
{
  "name": "Kepler-like System",
  "particles": [
    {
      "position": [400, 300],
      "velocity": [0, 0],
      "mass": 8000.0,
      "color": [255, 240, 200],
      "name": "Star",
      "fixed": true
    },
    {
      "position": [450, 300],
      "velocity": [0, 63.2],
      "mass": 5.0,
      "color": [200, 100, 50],
      "name": "Hot Jupiter"
    },
    {
      "position": [520, 300],
      "velocity": [0, 49.2],
      "mass": 20.0,
      "color": [100, 150, 255],
      "name": "Super Earth"
    },
    {
      "position": [600, 300],
      "velocity": [0, 40.8],
      "mass": 15.0,
      "color": [150, 255, 150],
      "name": "Neptune-like"
    }
  ],
  "settings": {
    "gravitational_constant": 0.1,
    "time_step": 0.005,
    "trail_length": 300
  }
}
```

## Command Line Options

```bash
# Load specific scenario
./AstroDynamicsEngine --scenario scenarios/my_system.json

# Set particle count (for procedural generation - future feature)
./AstroDynamicsEngine --particles 10000

# Benchmark mode (future feature)
./AstroDynamicsEngine --benchmark

# Fullscreen mode (future feature)
./AstroDynamicsEngine --fullscreen
```
