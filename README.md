# C++-OpenGL-2D-Game
Simple 2d sandbox

# How to build (Windows):

conan install . --output-folder=build --build=missing

cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="build/conan_toolchain.cmake"

# build release:

cmake --build build --config Release

# build debug:

cmake --build build --config Debug
