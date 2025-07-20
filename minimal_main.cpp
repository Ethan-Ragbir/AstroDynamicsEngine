#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>

struct Particle {
    sf::Vector2f pos, vel;
    float mass;
    
    Particle(float x, float y, float vx, float vy, float m) 
        : pos(x, y), vel(vx, vy), mass(m) {}
};

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "AstroDynamics - Minimal");
    
    std::vector<Particle> particles;
    particles.emplace_back(400, 300, 0, 0, 5000);    // Sun
    particles.emplace_back(500, 300, 0, 40, 10);     // Planet
    
    const float G = 0.1f;
    const float dt = 0.01f;
    
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::MouseButtonPressed) {
                particles.emplace_back(event.mouseButton.x, event.mouseButton.y, 0, 0, 10);
            }
        }
        
        // Physics
        for (size_t i = 0; i < particles.size(); i++) {
            sf::Vector2f force(0, 0);
            for (size_t j = 0; j < particles.size(); j++) {
                if (i != j) {
                    sf::Vector2f r = particles[j].pos - particles[i].pos;
                    float dist = std::sqrt(r.x * r.x + r.y * r.y + 1.0f);
                    force += r * (G * particles[i].mass * particles[j].mass / (dist * dist * dist));
                }
            }
            particles[i].vel += force / particles[i].mass * dt;
            particles[i].pos += particles[i].vel * dt;
        }
        
        // Render
        window.clear();
        for (const auto& p : particles) {
            sf::CircleShape circle(5);
            circle.setPosition(p.pos - sf::Vector2f(5, 5));
            circle.setFillColor(sf::Color::White);
            window.draw(circle);
        }
        window.display();
    }
    
    return 0;
}
