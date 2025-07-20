#!/bin/bash

# AstroDynamics Web Deployment Script
# This script sets up the web version for GitHub Pages

echo "🚀 AstroDynamics Web Deployment Script"
echo "======================================"

# Check if we're in the right directory
if [ ! -f "main.cpp" ]; then
    echo "❌ Error: main.cpp not found. Make sure you're in the AstroDynamicsEngine directory."
    exit 1
fi

# Get GitHub username
echo -n "Enter your GitHub username: "
read GITHUB_USERNAME

if [ -z "$GITHUB_USERNAME" ]; then
    echo "❌ Error: GitHub username cannot be empty"
    exit 1
fi

# Create docs directory
echo "📁 Creating docs directory..."
mkdir -p docs

# Create index.html
echo "📄 Creating index.html..."
cat > docs/index.html << 'EOF'
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>AstroDynamics Engine - Web Version</title>
    <style>
        @import url('https://fonts.googleapis.com/css2?family=Orbitron:wght@400;700&display=swap');
        
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Orbitron', monospace;
            background: #000;
            color: #fff;
            overflow: hidden;
            display: flex;
            flex-direction: column;
            height: 100vh;
        }
        
        #header {
            background: rgba(0, 0, 0, 0.8);
            padding: 10px 20px;
            border-bottom: 2px solid #333;
            z-index: 100;
            backdrop-filter: blur(10px);
        }
        
        .title {
            font-size: 24px;
            font-weight: 700;
            color: #00ffff;
            text-shadow: 0 0 10px #00ffff;
            display: inline-block;
            margin-right: 20px;
        }
        
        .controls {
            display: inline-block;
            font-size: 12px;
            color: #888;
        }
        
        #canvas-container {
            flex: 1;
            position: relative;
            background: radial-gradient(ellipse at center, #0a0a2a 0%, #000 100%);
        }
        
        canvas {
            display: block;
            cursor: crosshair;
        }
        
        #hud {
            position: absolute;
            top: 10px;
            left: 10px;
            background: rgba(0, 0, 0, 0.7);
            padding: 15px;
            border-radius: 5px;
            font-size: 12px;
            border: 1px solid #333;
            backdrop-filter: blur(5px);
        }
        
        #hud div {
            margin: 5px 0;
        }
        
        .hud-label {
            color: #888;
            display: inline-block;
            width: 100px;
        }
        
        .hud-value {
            color: #0f0;
            font-weight: bold;
        }
        
        #controls-panel {
            position: absolute;
            top: 10px;
            right: 10px;
            background: rgba(0, 0, 0, 0.7);
            padding: 15px;
            border-radius: 5px;
            border: 1px solid #333;
            backdrop-filter: blur(5px);
        }
        
        button {
            background: #1a1a1a;
            color: #fff;
            border: 1px solid #333;
            padding: 8px 15px;
            margin: 2px;
            cursor: pointer;
            border-radius: 3px;
            font-family: 'Orbitron', monospace;
            font-size: 11px;
            transition: all 0.3s;
        }
        
        button:hover {
            background: #333;
            border-color: #00ffff;
            box-shadow: 0 0 5px #00ffff;
        }
        
        button:active {
            transform: scale(0.95);
        }
        
        .scenario-buttons {
            margin-top: 10px;
            padding-top: 10px;
            border-top: 1px solid #333;
        }
        
        .slider-container {
            margin: 10px 0;
        }
        
        .slider-container label {
            display: block;
            margin-bottom: 5px;
            font-size: 11px;
            color: #888;
        }
        
        input[type="range"] {
            width: 100%;
            height: 5px;
            background: #333;
            outline: none;
            border-radius: 3px;
        }
        
        input[type="range"]::-webkit-slider-thumb {
            appearance: none;
            width: 15px;
            height: 15px;
            background: #00ffff;
            cursor: pointer;
            border-radius: 50%;
            box-shadow: 0 0 5px #00ffff;
        }
        
        #footer {
            background: rgba(0, 0, 0, 0.8);
            padding: 10px;
            text-align: center;
            font-size: 11px;
            color: #666;
            border-top: 1px solid #333;
        }
        
        #footer a {
            color: #00ffff;
            text-decoration: none;
        }
        
        #footer a:hover {
            text-shadow: 0 0 5px #00ffff;
        }
        
        @media (max-width: 768px) {
            .title {
                font-size: 18px;
            }
            
            .controls {
                display: none;
            }
            
            #controls-panel {
                right: auto;
                left: 10px;
                top: 150px;
            }
        }
        
        .glow {
            filter: drop-shadow(0 0 3px currentColor);
        }
        
        #stars {
            position: absolute;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            pointer-events: none;
        }
    </style>
</head>
<body>
    <div id="header">
        <h1 class="title">AstroDynamics Engine</h1>
        <span class="controls">
            <strong>Controls:</strong> Left Click: Add particle | Scroll: Zoom | Middle Drag: Pan | 
            Space: Clear | P: Pause | T: Trails | G: Grid
        </span>
    </div>
    
    <div id="canvas-container">
        <canvas id="stars"></canvas>
        <canvas id="canvas"></canvas>
        
        <div id="hud">
            <div><span class="hud-label">FPS:</span> <span class="hud-value" id="fps">60</span></div>
            <div><span class="hud-label">Particles:</span> <span class="hud-value" id="particleCount">0</span></div>
            <div><span class="hud-label">Total KE:</span> <span class="hud-value" id="energy">0</span></div>
            <div><span class="hud-label">Zoom:</span> <span class="hud-value" id="zoom">1.00x</span></div>
            <div><span class="hud-label">Time Step:</span> <span class="hud-value" id="timestep">0.01</span></div>
        </div>
        
        <div id="controls-panel">
            <button onclick="simulation.togglePause()">⏸ Pause (P)</button>
            <button onclick="simulation.toggleTrails()">〰️ Trails (T)</button>
            <button onclick="simulation.toggleGrid()">⊞ Grid (G)</button>
            <button onclick="simulation.clearParticles()
