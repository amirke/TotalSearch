#!/bin/bash

echo "Building TotalSearch..."

# Create build directory if it doesn't exist
mkdir -p build
cd build

# Configure with CMake
echo "Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the project
echo "Building project..."
cmake --build . --config Release

# Check if build was successful
if [ $? -eq 0 ]; then
    echo "Build successful!"
    echo "Executable location: build/TotalSearch"
else
    echo "Build failed!"
    exit 1
fi

cd .. 