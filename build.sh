#!/bin/bash

# Build script for the C++ shell

echo "Building C++ shell..."
g++ -std=c++17 -o shell shell.cpp -lreadline

if [ $? -eq 0 ]; then
    echo "✓ Build successful!"
    echo "Run with: ./shell"
else
    echo "✗ Build failed"
    exit 1
fi
