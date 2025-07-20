#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <array>

// Forward declaration
class Particle;

// Barnes-Hut Octree Node for 2D simulation (Quadtree)
class QuadTreeNode {
public:
    // Boundaries of this node
    struct Boundary {
        sf::Vector2f center;
        float halfSize;
        
        bool contains(const sf::Vector2f& point) const {
            return (point.x >= center.x - halfSize && point.x <= center.x + halfSize &&
                    point.y >= center.y - halfSize && point.y <= center.y + halfSize);
        }
        
        bool intersects(const Boundary& other) const {
            return !(other.center.x - other.halfSize > center.x + halfSize ||
                    other.center.x + other.halfSize < center.x - halfSize ||
                    other.center.y - other.halfSize > center.y + halfSize ||
                    other.center.y + other.halfSize < center.y - halfSize);
        }
    };
    
private:
    static constexpr int MAX_PARTICLES = 1;  // Maximum particles per leaf node
    static constexpr int NUM_CHILDREN = 4;   // Quadtree has 4 children
    
    Boundary boundary;
    std::vector<const Particle*> particles;
    std::array<std::unique_ptr<QuadTreeNode>, NUM_CHILDREN> children;
    
    // Center of mass and total mass for this node
    sf::Vector2f centerOfMass;
    float totalMass = 0.0f;
    
    bool isLeaf = true;
    
    void subdivide() {
        float newHalfSize = boundary.halfSize * 0.5f;
        
        // Create four children (NE, NW, SE, SW)
        children[0] = std::make_unique<QuadTreeNode>(
            Boundary{boundary.center + sf::Vector2f(newHalfSize, -newHalfSize), newHalfSize});
        children[1] = std::make_unique<QuadTreeNode>(
            Boundary{boundary.center + sf::Vector2f(-newHalfSize, -newHalfSize), newHalfSize});
        children[2] = std::make_unique<QuadTreeNode>(
            Boundary{boundary.center + sf::Vector2f(newHalfSize, newHalfSize), newHalfSize});
        children[3] = std::make_unique<QuadTreeNode>(
            Boundary{boundary.center + sf::Vector2f(-newHalfSize, newHalfSize), newHalfSize});
        
        isLeaf = false;
        
        // Redistribute existing particles
        for (const auto* p : particles) {
            for (auto& child : children) {
                if (child->insert(p)) {
                    break;
                }
            }
        }
        particles.clear();
    }
    
    void updateCenterOfMass() {
        if (isLeaf) {
            if (particles.empty()) {
                totalMass = 0.0f;
                centerOfMass = boundary.center;
            } else {
                totalMass = 0.0f;
                centerOfMass = sf::Vector2f(0, 0);
                for (const auto* p : particles) {
                    totalMass += p->mass;
                    centerOfMass += p->position * p->mass;
                }
                centerOfMass /= totalMass;
            }
        } else {
            totalMass = 0.0f;
            centerOfMass = sf::Vector2f(0, 0);
            for (const auto& child : children) {
                if (child && child->totalMass > 0) {
                    totalMass += child->totalMass;
                    centerOfMass += child->centerOfMass * child->totalMass;
                }
            }
            if (totalMass > 0) {
                centerOfMass /= totalMass;
            }
        }
    }
    
public:
    explicit QuadTreeNode(const Boundary& b) : boundary(b) {}
    
    bool insert(const Particle* particle) {
        if (!boundary.contains(particle->position)) {
            return false;
        }
        
        if (isLeaf) {
            if (particles.size() < MAX_PARTICLES) {
                particles.push_back(particle);
                return true;
            } else {
                subdivide();
                return insert(particle);
            }
        } else {
            for (auto& child : children) {
                if (child->insert(particle)) {
                    return true;
                }
            }
        }
        return false;
    }
    
    void computeForce(const Particle& particle, sf::Vector2f& force, float theta, float G, float softening) const {
        if (totalMass == 0 || (&particle == particles.front() && particles.size() == 1)) {
            return;
        }
        
        sf::Vector2f r = centerOfMass - particle.position;
        float distance = std::sqrt(r.x * r.x + r.y * r.y + softening * softening);
        
        if (isLeaf) {
            // Direct calculation for leaf nodes
            for (const auto* p : particles) {
                if (p != &particle) {
                    sf::Vector2f r_ij = p->position - particle.position;
                    float r2 = r_ij.x * r_ij.x + r_ij.y * r_ij.y + softening * softening;
                    float r3 = r2 * std::sqrt(r2);
                    force += r_ij * (G * particle.mass * p->mass / r3);
                }
            }
        } else {
            // Check if we can use the node as a single body
            float s = boundary.halfSize * 2.0f;  // Size of the node
            if (s / distance < theta) {
                // Use center of mass approximation
                float r2 = r.x * r.x + r.y * r.y + softening * softening;
                float r3 = r2 * std::sqrt(r2);
                force += r * (G * particle.mass * totalMass / r3);
            } else {
                // Recurse into children
                for (const auto& child : children) {
                    if (child) {
                        child->computeForce(particle, force, theta, G, softening);
                    }
                }
            }
        }
    }
    
    void build(const std::vector<Particle>& particles) {
        for (const auto& p : particles) {
            insert(&p);
        }
        updateCenterOfMass();
    }
    
    // Visualization helper
    void draw(sf::RenderWindow& window, int depth = 0) const {
        sf::RectangleShape rect(sf::Vector2f(boundary.halfSize * 2, boundary.halfSize * 2));
        rect.setPosition(boundary.center - sf::Vector2f(boundary.halfSize, boundary.halfSize));
        rect.setFillColor(sf::Color::Transparent);
        rect.setOutlineColor(sf::Color(100, 100, 100, 50));
        rect.setOutlineThickness(1.0f);
        window.draw(rect);
        
        if (!isLeaf) {
            for (const auto& child : children) {
                if (child) {
                    child->draw(window, depth + 1);
                }
            }
        }
    }
};

// Barnes-Hut Force Calculator
class BarnesHutForceCalculator {
private:
    float theta = 0.5f;  // Opening angle parameter (lower = more accurate)
    
public:
    void setTheta(float t) { theta = t; }
    
    std::vector<sf::Vector2f> computeForces(const std::vector<Particle>& particles, 
                                           float G, float softening) {
        if (particles.empty()) return {};
        
        // Find bounds
        sf::Vector2f min = particles[0].position;
        sf::Vector2f max = particles[0].position;
        
        for (const auto& p : particles) {
            min.x = std::min(min.x, p.position.x);
            min.y = std::min(min.y, p.position.y);
            max.x = std::max(max.x, p.position.x);
            max.y = std::max(max.y, p.position.y);
        }
        
        // Create root node
        sf::Vector2f center = (min + max) * 0.5f;
        float size = std::max(max.x - min.x, max.y - min.y) * 0.6f;
        
        QuadTreeNode root(QuadTreeNode::Boundary{center, size});
        root.build(particles);
        
        // Compute forces
        std::vector<sf::Vector2f> forces(particles.size(), sf::Vector2f(0, 0));
        
        #pragma omp parallel for
        for (size_t i = 0; i < particles.size(); ++i) {
            root.computeForce(particles[i], forces[i], theta, G, softening);
        }
        
        return forces;
    }
};g
