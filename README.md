AstroDynamicsEngine
A C++ simulation of gravitational interactions between multiple bodies, visualized in real-time using the SFML library. This project models the dynamics of celestial systems with numerical methods, offering an educational and interactive tool for exploring gravitational physics and computational modeling.
Table of Contents

Description
Installation
Usage
Physics and Math
Contributing
License

Description
AstroDynamicsEngine is an n-body simulation that calculates and visualizes the motion of bodies under gravitational forces, based on Newton's Law of Universal Gravitation. It uses numerical integration to update positions and velocities over time, displaying the results in a 2D real-time graphical interface powered by SFML. Whether you're a student of physics, a programming enthusiast, or a hobbyist, this project offers a hands-on way to explore complex systems.
Key Features

Real-time visualization of gravitational interactions in 2D.
Modular design for easy experimentation with initial conditions.
Planned support for interactive controls (e.g., adding bodies during runtime).

Installation
To set up and run the simulation, you'll need a C++ compiler and the SFML library installed on your system.
Prerequisites

C++ Compiler: Recommended options include g++ (Linux/macOS) or MSVC (Windows).
SFML: Simple and Fast Multimedia Library for graphics and window management.

Installing SFML

Ubuntu/Linux:
sudo apt-get update
sudo apt-get install libsfml-dev


macOS (using Homebrew):
brew install sfml


Windows:

Download SFML from the official website.
Extract the archive and link it with your compiler (e.g., MinGW or Visual Studio).
Refer to the SFML Windows tutorial for detailed setup.



Building the Project
Clone the repository and compile the source code:
git clone https://github.com/yourusername/AstroDynamicsEngine.git
cd AstroDynamicsEngine
g++ nbody.cpp -o nbody -lsfml-graphics -lsfml-window -lsfml-system

For Windows or other compilers, adjust the build command to include appropriate library paths (e.g., -I, -L flags).
Usage
Run the compiled executable to start the simulation:
./nbody

A window will open displaying the n-body system in motion. The default configuration includes a few bodies with predefined masses, positions, and velocities.
Example Scenarios

Solar System Lite: One heavy central body (e.g., a star) with lighter bodies orbiting it.
Binary System: Two bodies of equal mass orbiting each other.

To experiment, modify the initial conditions in nbody.cpp (e.g., mass, position, velocity) and recompile.
Planned Features

Interactive Mode: Click to add bodies or adjust parameters during runtime.
Config Files: Load initial conditions from a JSON or text file.

Physics and Math
The simulation is grounded in classical mechanics and numerical computation:
Gravitational Force
The force between two bodies is computed using Newton's Law of Universal Gravitation:
[F = G \frac{m_1 m_2}{r^2}]

( F ): Gravitational force magnitude.
( G ): Gravitational constant (approximated for simulation scale).
( m_1, m_2 ): Masses of the two bodies.
( r ): Distance between the bodies.

This force is applied as a vector, influencing acceleration in 2D space.
Numerical Integration
Positions and velocities are updated using the Euler Method:
[\mathbf{v}_{t+1} = \mathbf{v}_t + \mathbf{a}_t \cdot \Delta t]
[\mathbf{p}_{t+1} = \mathbf{p}_t + \mathbf{v}_t \cdot \Delta t]

( \mathbf{v} ): Velocity vector.
( \mathbf{a} ): Acceleration vector (from gravitational forces).
( \mathbf{p} ): Position vector.
( \Delta t ): Time step.

For improved accuracy, future versions could adopt the Runge-Kutta 4 (RK4) method.
Technical Notes

Time Step: A small ( \Delta t ) ensures stability but may slow the simulation.
Vector Operations: All calculations use 2D vectors for simplicity and performance.

Contributing
We welcome contributions to enhance AstroDynamicsEngine! To get started:

Fork the repository on GitHub.
Create a feature branch (git checkout -b feature-name).
Commit your changes with clear messages (git commit -m "Add feature X").
Push to your fork (git push origin feature-name).
Submit a pull request with a description of your changes.

Ideas for Contributions

Upgrade to RK4 integration for more accurate trajectories.
Implement the Barnes-Hut algorithm to optimize performance with many bodies.
Add 3D support using OpenGL alongside SFML.
Create a GUI for adjusting simulation parameters.

License
This project is licensed under the MIT License. Feel free to use, modify, and distribute it as you see fit.
