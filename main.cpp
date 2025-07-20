#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>
#include <vector>
#include <cmath>
#include <iostream>
#include <fstream>
#include <memory>
#include <deque>
#include <algorithm>
#include <execution>
#include <chrono>
#include <iomanip>
#include <sstream>

using json = nlohmann::json;

// Constants
struct SimulationConstants {
    float G = 6.67430e-2f;        // Gravitational constant (scaled for visualization)
    float DT = 0.01f;             // Time step
    float SOFTENING = 1.0f;       // Softening parameter
    float MIN_DT = 0.0001f;       // Minimum time step for adaptive stepping
    float MAX_DT = 0.1f;          // Maximum time step
    float TRAIL_LENGTH = 100;     // Number of trail points
    bool ADAPTIVE_TIMESTEP = false;
};

// Particle class with enhanced features
class Particle {
public:
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Vector2f acceleration;
    float mass;
    sf::Color color;
    std::string name;
    std::deque<sf::Vector2f> trail;
    bool fixed = false;  // For fixed bodies like black holes
    
    Particle(sf::Vector2f pos, sf::Vector2f vel, float m, sf::Color c = sf::Color::White, const std::string& n = "")
        : position(pos), velocity(vel), acceleration(0, 0), mass(m), color(c), name(n) {}
    
    void updateTrail(size_t maxLength) {
        trail.push_back(position);
        if (trail.size() > maxLength) {
            trail.pop_front();
        }
    }
    
    float kineticEnergy() const {
        float v2 = velocity.x * velocity.x + velocity.y * velocity.y;
        return 0.5f * mass * v2;
    }
};

// Abstract Integrator class
class Integrator {
public:
    virtual ~Integrator() = default;
    virtual void integrate(std::vector<Particle>& particles, 
                         const std::function<std::vector<sf::Vector2f>(const std::vector<Particle>&)>& forceFunc,
                         float dt) = 0;
};

// Runge-Kutta 4th order integrator
class RungeKuttaIntegrator : public Integrator {
private:
    struct State {
        std::vector<sf::Vector2f> positions;
        std::vector<sf::Vector2f> velocities;
    };
    
    struct Derivative {
        std::vector<sf::Vector2f> dpos;  // velocity
        std::vector<sf::Vector2f> dvel;  // acceleration
    };
    
    Derivative evaluate(const State& initial, float dt, const Derivative& d,
                       const std::function<std::vector<sf::Vector2f>(const std::vector<Particle>&)>& forceFunc,
                       const std::vector<Particle>& particles) {
        State state;
        state.positions.resize(initial.positions.size());
        state.velocities.resize(initial.velocities.size());
        
        // Create temporary particles for force calculation
        std::vector<Particle> tempParticles = particles;
        
        for (size_t i = 0; i < initial.positions.size(); ++i) {
            state.positions[i] = initial.positions[i] + d.dpos[i] * dt;
            state.velocities[i] = initial.velocities[i] + d.dvel[i] * dt;
            tempParticles[i].position = state.positions[i];
            tempParticles[i].velocity = state.velocities[i];
        }
        
        Derivative output;
        output.dpos = state.velocities;
        
        // Calculate accelerations
        auto forces = forceFunc(tempParticles);
        output.dvel.resize(forces.size());
        for (size_t i = 0; i < forces.size(); ++i) {
            output.dvel[i] = forces[i] / tempParticles[i].mass;
        }
        
        return output;
    }
    
public:
    void integrate(std::vector<Particle>& particles,
                  const std::function<std::vector<sf::Vector2f>(const std::vector<Particle>&)>& forceFunc,
                  float dt) override {
        State initial;
        for (const auto& p : particles) {
            initial.positions.push_back(p.position);
            initial.velocities.push_back(p.velocity);
        }
        
        Derivative k1, k2, k3, k4;
        
        // Zero derivative for k1
        k1.dpos.resize(particles.size(), sf::Vector2f(0, 0));
        k1.dvel.resize(particles.size(), sf::Vector2f(0, 0));
        
        k1 = evaluate(initial, 0.0f, k1, forceFunc, particles);
        k2 = evaluate(initial, dt * 0.5f, k1, forceFunc, particles);
        k3 = evaluate(initial, dt * 0.5f, k2, forceFunc, particles);
        k4 = evaluate(initial, dt, k3, forceFunc, particles);
        
        // Update particles
        for (size_t i = 0; i < particles.size(); ++i) {
            if (!particles[i].fixed) {
                sf::Vector2f dxdt = (k1.dpos[i] + 2.0f * k2.dpos[i] + 2.0f * k3.dpos[i] + k4.dpos[i]) / 6.0f;
                sf::Vector2f dvdt = (k1.dvel[i] + 2.0f * k2.dvel[i] + 2.0f * k3.dvel[i] + k4.dvel[i]) / 6.0f;
                
                particles[i].position += dxdt * dt;
                particles[i].velocity += dvdt * dt;
                particles[i].acceleration = dvdt;
            }
        }
    }
};

// HUD for displaying simulation information
class HUD {
private:
    sf::Font font;
    sf::Text fpsText;
    sf::Text particleCountText;
    sf::Text energyText;
    sf::Text zoomText;
    bool visible = true;
    
public:
    HUD() {
        // In production, load from file. Using default font for simplicity
        if (!font.loadFromFile("assets/fonts/arial.ttf")) {
            std::cerr << "Warning: Could not load font, HUD disabled" << std::endl;
            visible = false;
            return;
        }
        
        fpsText.setFont(font);
        fpsText.setCharacterSize(14);
        fpsText.setFillColor(sf::Color::White);
        fpsText.setPosition(10, 10);
        
        particleCountText.setFont(font);
        particleCountText.setCharacterSize(14);
        particleCountText.setFillColor(sf::Color::White);
        particleCountText.setPosition(10, 30);
        
        energyText.setFont(font);
        energyText.setCharacterSize(14);
        energyText.setFillColor(sf::Color::White);
        energyText.setPosition(10, 50);
        
        zoomText.setFont(font);
        zoomText.setCharacterSize(14);
        zoomText.setFillColor(sf::Color::White);
        zoomText.setPosition(10, 70);
    }
    
    void update(float fps, size_t particleCount, float totalEnergy, float zoom) {
        if (!visible) return;
        
        std::stringstream ss;
        ss << "FPS: " << std::fixed << std::setprecision(1) << fps;
        fpsText.setString(ss.str());
        
        ss.str("");
        ss << "Particles: " << particleCount;
        particleCountText.setString(ss.str());
        
        ss.str("");
        ss << "Total KE: " << std::scientific << std::setprecision(2) << totalEnergy;
        energyText.setString(ss.str());
        
        ss.str("");
        ss << "Zoom: " << std::fixed << std::setprecision(2) << zoom << "x";
        zoomText.setString(ss.str());
    }
    
    void draw(sf::RenderWindow& window) {
        if (!visible) return;
        window.draw(fpsText);
        window.draw(particleCountText);
        window.draw(energyText);
        window.draw(zoomText);
    }
};

// Enhanced N-Body Simulation
class NBodySimulation {
private:
    std::vector<Particle> particles;
    sf::RenderWindow window;
    std::unique_ptr<Integrator> integrator;
    SimulationConstants constants;
    HUD hud;
    
    // Camera controls
    sf::View camera;
    float zoomLevel = 1.0f;
    sf::Vector2f cameraOffset;
    bool isPanning = false;
    sf::Vector2i lastMousePos;
    
    // Simulation state
    bool isPaused = false;
    bool showTrails = true;
    bool showVelocityVectors = false;
    
    // Performance tracking
    sf::Clock fpsClock;
    float frameTime = 0.0f;
    
    // Force calculation
    std::vector<sf::Vector2f> computeForces(const std::vector<Particle>& particles) {
        std::vector<sf::Vector2f> forces(particles.size(), sf::Vector2f(0, 0));
        
        // Parallel force calculation for better performance
        std::for_each(std::execution::par_unseq, 
                     forces.begin(), forces.end(),
                     [&](sf::Vector2f& force) {
            size_t i = &force - &forces[0];
            for (size_t j = 0; j < particles.size(); ++j) {
                if (i != j) {
                    sf::Vector2f r = particles[j].position - particles[i].position;
                    float r2 = r.x * r.x + r.y * r.y + constants.SOFTENING * constants.SOFTENING;
                    float r3 = r2 * std::sqrt(r2);
                    float F = constants.G * particles[i].mass * particles[j].mass / r3;
                    forces[i] += r * F;
                }
            }
        });
        
        return forces;
    }
    
    float calculateTotalEnergy() {
        float totalKE = 0.0f;
        for (const auto& p : particles) {
            totalKE += p.kineticEnergy();
        }
        return totalKE;
    }
    
    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    // Convert mouse position to world coordinates
                    sf::Vector2f worldPos = window.mapPixelToCoords(
                        sf::Vector2i(event.mouseButton.x, event.mouseButton.y), camera);
                    
                    particles.emplace_back(worldPos, sf::Vector2f(0, 0), 10.0f, 
                                         sf::Color(rand() % 156 + 100, rand() % 156 + 100, rand() % 156 + 100));
                } else if (event.mouseButton.button == sf::Mouse::Middle) {
                    isPanning = true;
                    lastMousePos = sf::Mouse::getPosition(window);
                }
            } else if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Middle) {
                    isPanning = false;
                }
            } else if (event.type == sf::Event::MouseMoved) {
                if (isPanning) {
                    sf::Vector2i currentMousePos = sf::Mouse::getPosition(window);
                    sf::Vector2f delta(
                        (lastMousePos.x - currentMousePos.x) * zoomLevel,
                        (lastMousePos.y - currentMousePos.y) * zoomLevel
                    );
                    cameraOffset += delta;
                    lastMousePos = currentMousePos;
                    updateCamera();
                }
            } else if (event.type == sf::Event::MouseWheelScrolled) {
                if (event.mouseWheelScroll.delta > 0) {
                    zoomLevel *= 0.9f;
                } else {
                    zoomLevel *= 1.1f;
                }
                zoomLevel = std::clamp(zoomLevel, 0.1f, 10.0f);
                updateCamera();
            } else if (event.type == sf::Event::KeyPressed) {
                handleKeyPress(event.key.code);
            }
        }
    }
    
    void handleKeyPress(sf::Keyboard::Key key) {
        switch (key) {
            case sf::Keyboard::Space:
                if (particles.size() > 1) {
                    particles.erase(particles.begin() + 1, particles.end());
                }
                break;
            case sf::Keyboard::P:
                isPaused = !isPaused;
                break;
            case sf::Keyboard::T:
                showTrails = !showTrails;
                break;
            case sf::Keyboard::V:
                showVelocityVectors = !showVelocityVectors;
                break;
            case sf::Keyboard::R:
                loadDefaultScenario();
                break;
            case sf::Keyboard::Num1:
                loadScenarioFromFile("scenarios/solar_system.json");
                break;
            default:
                break;
        }
    }
    
    void updateCamera() {
        camera.setSize(800.0f * zoomLevel, 600.0f * zoomLevel);
        camera.setCenter(400.0f + cameraOffset.x, 300.0f + cameraOffset.y);
        window.setView(camera);
    }
    
    void loadDefaultScenario() {
        particles.clear();
        particles.emplace_back(sf::Vector2f(400, 300), sf::Vector2f(0, 0), 5000.0f, sf::Color::Yellow, "Sun");
        particles.emplace_back(sf::Vector2f(400, 200), sf::Vector2f(50, 0), 10.0f, sf::Color::Cyan, "Planet 1");
        particles.emplace_back(sf::Vector2f(550, 300), sf::Vector2f(0, 35), 20.0f, sf::Color::Red, "Planet 2");
        particles.emplace_back(sf::Vector2f(400, 450), sf::Vector2f(-30, 0), 15.0f, sf::Color::Green, "Planet 3");
    }
    
public:
    NBodySimulation() : window(sf::VideoMode(800, 600), "AstroDynamics Engine v2.0") {
        window.setFramerateLimit(60);
        integrator = std::make_unique<RungeKuttaIntegrator>();
        
        camera = window.getDefaultView();
        updateCamera();
        
        loadDefaultScenario();
    }
    
    void loadScenarioFromFile(const std::string& filename) {
        try {
            std::ifstream file(filename);
            if (!file.is_open()) {
                std::cerr << "Could not open scenario file: " << filename << std::endl;
                return;
            }
            
            json j;
            file >> j;
            
            particles.clear();
            
            // Load particles
            for (const auto& p : j["particles"]) {
                sf::Vector2f pos(p["position"][0], p["position"][1]);
                sf::Vector2f vel(p["velocity"][0], p["velocity"][1]);
                float mass = p["mass"];
                sf::Color color(p["color"][0], p["color"][1], p["color"][2]);
                std::string name = p.value("name", "");
                
                particles.emplace_back(pos, vel, mass, color, name);
                
                if (p.contains("fixed")) {
                    particles.back().fixed = p["fixed"];
                }
            }
            
            // Load settings if present
            if (j.contains("settings")) {
                const auto& settings = j["settings"];
                constants.G = settings.value("gravitational_constant", constants.G);
                constants.DT = settings.value("time_step", constants.DT);
                constants.SOFTENING = settings.value("softening", constants.SOFTENING);
            }
            
            std::cout << "Loaded scenario: " << j.value("name", "Unknown") << std::endl;
            
        } catch (const std::exception& e) {
            std::cerr << "Error loading scenario: " << e.what() << std::endl;
        }
    }
    
    void run() {
        while (window.isOpen()) {
            frameTime = fpsClock.restart().asSeconds();
            float fps = 1.0f / frameTime;
            
            handleEvents();
            
            if (!isPaused) {
                // Update physics
                integrator->integrate(particles, 
                    [this](const std::vector<Particle>& p) { return computeForces(p); },
                    constants.DT);
                
                // Update trails
                if (showTrails) {
                    for (auto& p : particles) {
                        p.updateTrail(constants.TRAIL_LENGTH);
                    }
                }
            }
            
            // Update HUD
            hud.update(fps, particles.size(), calculateTotalEnergy(), zoomLevel);
            
            // Render
            window.clear(sf::Color::Black);
            
            // Draw trails
            if (showTrails) {
                for (const auto& p : particles) {
                    if (!p.trail.empty()) {
                        sf::VertexArray trail(sf::LineStrip, p.trail.size());
                        for (size_t i = 0; i < p.trail.size(); ++i) {
                            trail[i].position = p.trail[i];
                            sf::Color trailColor = p.color;
                            trailColor.a = static_cast<sf::Uint8>(255 * (i / float(p.trail.size())) * 0.5f);
                            trail[i].color = trailColor;
                        }
                        window.draw(trail);
                    }
                }
            }
            
            // Draw particles
            for (const auto& p : particles) {
                float radius = std::min(5.0f + std::log10(p.mass), 20.0f);
                sf::CircleShape shape(radius);
                shape.setPosition(p.position - sf::Vector2f(radius, radius));
                shape.setFillColor(p.color);
                
                // Add glow effect for massive objects
                if (p.mass > 1000) {
                    sf::CircleShape glow(radius * 2);
                    glow.setPosition(p.position - sf::Vector2f(radius * 2, radius * 2));
                    sf::Color glowColor = p.color;
                    glowColor.a = 50;
                    glow.setFillColor(glowColor);
                    window.draw(glow);
                }
                
                window.draw(shape);
                
                // Draw velocity vectors
                if (showVelocityVectors) {
                    sf::VertexArray velocityLine(sf::Lines, 2);
                    velocityLine[0].position = p.position;
                    velocityLine[0].color = sf::Color::White;
                    velocityLine[1].position = p.position + p.velocity * 0.5f;
                    velocityLine[1].color = sf::Color(255, 255, 255, 100);
                    window.draw(velocityLine);
                }
            }
            
            // Draw HUD with default view
            window.setView(window.getDefaultView());
            hud.draw(window);
            window.setView(camera);
            
            window.display();
        }
    }
};

int main(int argc, char* argv[]) {
    NBodySimulation sim;
    
    // Load scenario from command line if provided
    if (argc > 2 && std::string(argv[1]) == "--scenario") {
        sim.loadScenarioFromFile(argv[2]);
    }
    
    sim.run();
    return 0;
}
