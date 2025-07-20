// AstroDynamics Engine - Web Version
// N-body gravitational simulation with RK4 integration

class Particle {
    constructor(x, y, vx, vy, mass, color = '#ffffff', name = '') {
        this.position = { x, y };
        this.velocity = { x: vx, y: vy };
        this.acceleration = { x: 0, y: 0 };
        this.mass = mass;
        this.color = color;
        this.name = name;
        this.trail = [];
        this.fixed = false;
        this.radius = Math.min(5 + Math.log10(mass) * 2, 20);
    }
    
    updateTrail(maxLength) {
        this.trail.push({ x: this.position.x, y: this.position.y });
        if (this.trail.length > maxLength) {
            this.trail.shift();
        }
    }
    
    kineticEnergy() {
        const v2 = this.velocity.x ** 2 + this.velocity.y ** 2;
        return 0.5 * this.mass * v2;
    }
}

class RungeKuttaIntegrator {
    evaluate(particles, dt, derivatives = null) {
        const n = particles.length;
        const state = {
            positions: [],
            velocities: []
        };
        
        // Apply derivatives if provided
        for (let i = 0; i < n; i++) {
            if (derivatives) {
                state.positions.push({
                    x: particles[i].position.x + derivatives.dpos[i].x * dt,
                    y: particles[i].position.y + derivatives.dpos[i].y * dt
                });
                state.velocities.push({
                    x: particles[i].velocity.x + derivatives.dvel[i].x * dt,
                    y: particles[i].velocity.y + derivatives.dvel[i].y * dt
                });
            } else {
                state.positions.push({ ...particles[i].position });
                state.velocities.push({ ...particles[i].velocity });
            }
        }
        
        // Calculate accelerations
        const forces = this.computeForces(particles, state.positions);
        const output = {
            dpos: state.velocities,
            dvel: []
        };
        
        for (let i = 0; i < n; i++) {
            output.dvel.push({
                x: forces[i].x / particles[i].mass,
                y: forces[i].y / particles[i].mass
            });
        }
        
        return output;
    }
    
    computeForces(particles, positions) {
        const n = particles.length;
        const forces = Array(n).fill(null).map(() => ({ x: 0, y: 0 }));
        const G = simulation.constants.G * simulation.gravityMultiplier;
        const softening2 = simulation.constants.SOFTENING ** 2;
        
        for (let i = 0; i < n; i++) {
            for (let j = i + 1; j < n; j++) {
                const dx = positions[j].x - positions[i].x;
                const dy = positions[j].y - positions[i].y;
                const r2 = dx * dx + dy * dy + softening2;
                const r = Math.sqrt(r2);
                const F = G * particles[i].mass * particles[j].mass / r2;
                
                const fx = F * dx / r;
                const fy = F * dy / r;
                
                forces[i].x += fx;
                forces[i].y += fy;
                forces[j].x -= fx;
                forces[j].y -= fy;
            }
        }
        
        return forces;
    }
    
    integrate(particles, dt) {
        // RK4 integration
        const k1 = this.evaluate(particles, 0);
        const k2 = this.evaluate(particles, dt * 0.5, k1);
        const k3 = this.evaluate(particles, dt * 0.5, k2);
        const k4 = this.evaluate(particles, dt, k3);
        
        // Update particles
        for (let i = 0; i < particles.length; i++) {
            if (!particles[i].fixed) {
                const dxdt = {
                    x: (k1.dpos[i].x + 2 * k2.dpos[i].x + 2 * k3.dpos[i].x + k4.dpos[i].x) / 6,
                    y: (k1.dpos[i].y + 2 * k2.dpos[i].y + 2 * k3.dpos[i].y + k4.dpos[i].y) / 6
                };
                const dvdt = {
                    x: (k1.dvel[i].x + 2 * k2.dvel[i].x + 2 * k3.dvel[i].x + k4.dvel[i].x) / 6,
                    y: (k1.dvel[i].y + 2 * k2.dvel[i].y + 2 * k3.dvel[i].y + k4.dvel[i].y) / 6
                };
                
                particles[i].position.x += dxdt.x * dt;
                particles[i].position.y += dxdt.y * dt;
                particles[i].velocity.x += dvdt.x * dt;
                particles[i].velocity.y += dvdt.y * dt;
                particles[i].acceleration = dvdt;
            }
        }
    }
}

class AstroDynamicsSimulation {
    constructor() {
        this.canvas = document.getElementById('canvas');
        this.ctx = this.canvas.getContext('2d');
        this.starsCanvas = document.getElementById('stars');
        this.starsCtx = this.starsCanvas.getContext('2d');
        
        this.particles = [];
        this.integrator = new RungeKuttaIntegrator();
        
        this.constants = {
            G: 100,  // Scaled for visualization
            DT: 0.01,
            SOFTENING: 1.0,
            TRAIL_LENGTH: 100
        };
        
        this.camera = {
            x: 0,
            y: 0,
            zoom: 1
        };
        
        this.isPaused = false;
        this.showTrails = true;
        this.showGrid = false;
        this.gravityMultiplier = 1.0;
        this.speedMultiplier = 1.0;
        
        this.mouse = {
            x: 0,
            y: 0,
            down: false,
            button: 0,
            lastX: 0,
            lastY: 0
        };
        
        this.fps = 60;
        this.frameTime = 0;
        this.lastTime = performance.now();
        
        this.init();
    }
    
    init() {
        this.resize();
        window.addEventListener('resize', () => this.resize());
        
        this.setupEventListeners();
        this.drawStarfield();
        this.loadScenario('default');
        this.animate();
    }
    
    resize() {
        const container = document.getElementById('canvas-container');
        this.canvas.width = container.clientWidth;
        this.canvas.height = container.clientHeight;
        this.starsCanvas.width = container.clientWidth;
        this.starsCanvas.height = container.clientHeight;
        
        this.camera.x = this.canvas.width / 2;
        this.camera.y = this.canvas.height / 2;
        this.drawStarfield();
    }
    
    drawStarfield() {
        const ctx = this.starsCtx;
        ctx.fillStyle = 'rgba(0, 0, 0, 0.9)';
        ctx.fillRect(0, 0, this.starsCanvas.width, this.starsCanvas.height);
        
        // Draw stars
        for (let i = 0; i < 200; i++) {
            const x = Math.random() * this.starsCanvas.width;
            const y = Math.random() * this.starsCanvas.height;
            const radius = Math.random() * 1.5;
            const opacity = Math.random() * 0.8 + 0.2;
            
            ctx.beginPath();
            ctx.arc(x, y, radius, 0, Math.PI * 2);
            ctx.fillStyle = `rgba(255, 255, 255, ${opacity})`;
            ctx.fill();
        }
    }
    
    setupEventListeners() {
        // Mouse events
        this.canvas.addEventListener('mousedown', (e) => this.onMouseDown(e));
        this.canvas.addEventListener('mousemove', (e) => this.onMouseMove(e));
        this.canvas.addEventListener('mouseup', (e) => this.onMouseUp(e));
        this.canvas.addEventListener('wheel', (e) => this.onWheel(e));
        this.canvas.addEventListener('contextmenu', (e) => e.preventDefault());
        
        // Keyboard events
        document.addEventListener('keydown', (e) => this.onKeyDown(e));
        
        // Sliders
        document.getElementById('gravitySlider').addEventListener('input', (e) => {
            this.gravityMultiplier = parseFloat(e.target.value);
            document.getElementById('gravityValue').textContent = this.gravityMultiplier.toFixed(1);
        });
        
        document.getElementById('speedSlider').addEventListener('input', (e) => {
            this.speedMultiplier = parseFloat(e.target.value);
            document.getElementById('speedValue').textContent = this.speedMultiplier.toFixed(1);
        });
    }
    
    onMouseDown(e) {
        const rect = this.canvas.getBoundingClientRect();
        this.mouse.x = e.clientX - rect.left;
        this.mouse.y = e.clientY - rect.top;
        this.mouse.down = true;
        this.mouse.button = e.button;
        this.mouse.lastX = this.mouse.x;
        this.mouse.lastY = this.mouse.y;
        
        if (e.button === 0) { // Left click
            const worldPos = this.screenToWorld(this.mouse.x, this.mouse.y);
            const color = `hsl(${Math.random() * 360}, 70%, 60%)`;
            this.particles.push(new Particle(worldPos.x, worldPos.y, 0, 0, 10, color));
        }
    }
    
    onMouseMove(e) {
        const rect = this.canvas.getBoundingClientRect();
        this.mouse.x = e.clientX - rect.left;
        this.mouse.y = e.clientY - rect.top;
        
        if (this.mouse.down && this.mouse.button === 1) { // Middle mouse
            const dx = this.mouse.x - this.mouse.lastX;
            const dy = this.mouse.y - this.mouse.lastY;
            this.camera.x += dx;
            this.camera.y += dy;
            this.mouse.lastX = this.mouse.x;
            this.mouse.lastY = this.mouse.y;
        }
    }
    
    onMouseUp(e) {
        this.mouse.down = false;
    }
    
    onWheel(e) {
        e.preventDefault();
        const zoomSpeed = 0.1;
        if (e.deltaY < 0) {
            this.camera.zoom *= (1 + zoomSpeed);
        } else {
            this.camera.zoom *= (1 - zoomSpeed);
        }
        this.camera.zoom = Math.max(0.1, Math.min(10, this.camera.zoom));
    }
    
    onKeyDown(e) {
        switch(e.key.toLowerCase()) {
            case ' ':
                e.preventDefault();
                this.clearParticles();
                break;
            case 'p':
                this.togglePause();
                break;
            case 't':
                this.toggleTrails();
                break;
            case 'g':
                this.toggleGrid();
                break;
        }
    }
    
    screenToWorld(x, y) {
        return {
            x: (x - this.camera.x) / this.camera.zoom,
            y: (y - this.camera.y) / this.camera.zoom
        };
    }
    
    worldToScreen(x, y) {
        return {
            x: x * this.camera.zoom + this.camera.x,
            y: y * this.camera.zoom + this.camera.y
        };
    }
    
    togglePause() {
        this.isPaused = !this.isPaused;
    }
    
    toggleTrails() {
        this.showTrails = !this.showTrails;
        if (!this.showTrails) {
            this.particles.forEach(p => p.trail = []);
        }
    }
    
    toggleGrid() {
        this.showGrid = !this.showGrid;
    }
    
    clearParticles() {
        this.particles = this.particles.filter(p => p.name === 'Sun' || p.fixed);
    }
    
    loadScenario(scenario) {
        this.particles = [];
        
        switch(scenario) {
            case 'default':
                this.particles.push(new Particle(0, 0, 0, 0, 5000, '#ffff00', 'Sun'));
                this.particles[0].fixed = true;
                this.particles.push(new Particle(100, 0, 0, 50, 10, '#00ffff'));
                this.particles.push(new Particle(150, 0, 0, 40, 20, '#ff0000'));
                this.particles.push(new Particle(-120, 0, 0, -45, 15, '#00ff00'));
                break;
                
            case 'binary':
                this.particles.push(new Particle(-50, 0, 0, -25, 2000, '#ffff66'));
                this.particles.push(new Particle(50, 0, 0, 25, 2000, '#ffaa66'));
                break;
                
            case 'galaxy':
                // Central black holes
                this.particles.push(new Particle(-200, 0, 5, 0, 10000, '#ff00ff'));
                this.particles.push(new Particle(200, 0, -5, 0, 8000, '#00ffff'));
                
                // Add disk particles
                for (let i = 0; i < 50; i++) {
                    const angle = Math.random() * Math.PI * 2;
                    const r = 50 + Math.random() * 100;
                    const x = -200 + r * Math.cos(angle);
                    const y = r * Math.sin(angle);
                    const v = Math.sqrt(100 * 10000 / r) * 0.7;
                    const vx = 5 - v * Math.sin(angle);
                    const vy = v * Math.cos(angle);
                    this.particles.push(new Particle(x, y, vx, vy, 5, '#9999ff'));
                }
                
                for (let i = 0; i < 40; i++) {
                    const angle = Math.random() * Math.PI * 2;
                    const r = 40 + Math.random() * 80;
                    const x = 200 + r * Math.cos(angle);
                    const y = r * Math.sin(angle);
                    const v = Math.sqrt(100 * 8000 / r) * 0.7;
                    const vx = -5 - v * Math.sin(angle);
                    const vy = v * Math.cos(angle);
                    this.particles.push(new Particle(x, y, vx, vy, 5, '#ff9999'));
                }
                break;
                
            case 'lagrange':
                // Earth-Moon system with L4 point
                this.particles.push(new Particle(0, 0, 0, 0, 5000, '#0099ff', 'Earth'));
                this.particles[0].fixed = true;
                this.particles.push(new Particle(80, 0, 0, 56, 50, '#cccccc', 'Moon'));
                // L4 point (60 degrees ahead)
                const l4x = 40 * Math.cos(Math.PI / 3);
                const l4y = 40 * Math.sin(Math.PI / 3);
                this.particles.push(new Particle(l4x, l4y, -28, 28, 0.1, '#ff0000', 'L4'));
                break;
                
            case 'chaos':
                // Three-body chaos
                this.particles.push(new Particle(0, -100, 30, 0, 1000, '#ff0000'));
                this.particles.push(new Particle(86.6, 50, -15, 26, 1000, '#00ff00'));
                this.particles.push(new Particle(-86.6, 50, -15, -26, 1000, '#0000ff'));
                break;
        }
    }
    
    update(deltaTime) {
        if (this.isPaused) return;
        
        const dt = this.constants.DT * this.speedMultiplier;
        const steps = Math.ceil(deltaTime / 16); // Multiple steps for stability
        
        for (let i = 0; i < steps; i++) {
            this.integrator.integrate(this.particles, dt);
        }
        
        // Update trails
        if (this.showTrails) {
            this.particles.forEach(p => {
                p.updateTrail(this.constants.TRAIL_LENGTH);
            });
        }
    }
    
    render() {
        const ctx = this.ctx;
        
        // Clear canvas
        ctx.fillStyle = 'rgba(0, 0, 0, 0.1)';
        ctx.fillRect(0, 0, this.canvas.width, this.canvas.height);
        
        ctx.save();
        
        // Draw grid
        if (this.showGrid) {
            this.drawGrid();
        }
        
        // Draw trails
        if (this.showTrails) {
            this.particles.forEach(p => {
                if (p.trail.length > 1) {
                    ctx.strokeStyle = p.color;
                    ctx.globalAlpha = 0.5;
                    ctx.beginPath();
                    
                    for (let i = 0; i < p.trail.length; i++) {
                        const pos = this.worldToScreen(p.trail[i].x, p.trail[i].y);
                        if (i === 0) {
                            ctx.moveTo(pos.x, pos.y);
                        } else {
                            ctx.lineTo(pos.x, pos.y);
                        }
                    }
                    
                    ctx.lineWidth = 1;
                    ctx.stroke();
                }
            });
        }
        
        // Draw particles
        ctx.globalAlpha = 1;
        this.particles.forEach(p => {
            const pos = this.worldToScreen(p.position.x, p.position.y);
            
            // Glow for massive objects
            if (p.mass > 1000) {
                const gradient = ctx.createRadialGradient(pos.x, pos.y, 0, pos.x, pos.y, p.radius * 3 * this.camera.zoom);
                gradient.addColorStop(0, p.color);
                gradient.addColorStop(1, 'transparent');
                ctx.fillStyle = gradient;
                ctx.globalAlpha = 0.3;
                ctx.beginPath();
                ctx.arc(pos.x, pos.y, p.radius * 3 * this.camera.zoom, 0, Math.PI * 2);
                ctx.fill();
            }
            
            // Main particle
            ctx.globalAlpha = 1;
            ctx.fillStyle = p.color;
            ctx.beginPath();
            ctx.arc(pos.x, pos.y, p.radius * this.camera.zoom, 0, Math.PI * 2);
            ctx.fill();
            
            // Name label for named objects
            if (p.name && this.camera.zoom > 0.5) {
                ctx.fillStyle = '#ffffff';
                ctx.font = '10px Arial';
                ctx.textAlign = 'center';
                ctx.fillText(p.name, pos.x, pos.y - p.radius * this.camera.zoom - 5);
            }
        });
        
        ctx.restore();
        
        // Update HUD
        this.updateHUD();
    }
    
    drawGrid() {
        const ctx = this.ctx;
        ctx.strokeStyle = 'rgba(255, 255, 255, 0.1)';
        ctx.lineWidth = 1;
        
        const gridSize = 50 * this.camera.zoom;
        const offsetX = this.camera.x % gridSize;
        const offsetY = this.camera.y % gridSize;
        
        for (let x = offsetX; x < this.canvas.width; x += gridSize) {
            ctx.beginPath();
            ctx.moveTo(x, 0);
            ctx.lineTo(x, this.canvas.height);
            ctx.stroke();
        }
        
        for (let y = offsetY; y < this.canvas.height; y += gridSize) {
            ctx.beginPath();
            ctx.moveTo(0, y);
            ctx.lineTo(this.canvas.width, y);
            ctx.stroke();
        }
    }
    
    updateHUD() {
        document.getElementById('fps').textContent = Math.round(this.fps);
        document.getElementById('particleCount').textContent = this.particles.length;
        
        const totalEnergy = this.particles.reduce((sum, p) => sum + p.kineticEnergy(), 0);
        document.getElementById('energy').textContent = totalEnergy.toExponential(2);
        
        document.getElementById('zoom').textContent = this.camera.zoom.toFixed(2) + 'x';
        document.getElementById('timestep').textContent = (this.constants.DT * this.speedMultiplier).toFixed(3);
    }
    
    animate() {
        const currentTime = performance.now();
        const deltaTime = currentTime - this.lastTime;
        this.lastTime = currentTime;
        
        // Update FPS
        this.frameTime += deltaTime;
        if (this.frameTime >= 1000) {
            this.fps = 1000 / deltaTime;
            this.frameTime = 0;
        }
        
        this.update(deltaTime);
        this.render();
        
        requestAnimationFrame(() => this.animate());
    }
}

// Initialize simulation
const simulation = new AstroDynamicsSimulation();
