#!/bin/bash

# Build script for the C++ shell
# This script uses the Makefile to build the shell

echo "Building C++ shell using Makefile..."
make

if [ $? -eq 0 ]; then
    echo "✓ Build successful!"
    echo "Run with: ./bin/shell"
    echo "Or use: make run"
else
    echo "✗ Build failed"
    exit 1
fi
