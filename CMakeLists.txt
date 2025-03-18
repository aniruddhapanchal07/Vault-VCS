add_definitions(-D_CRT_SECURE_NO_WARNINGS)

cmake_minimum_required(VERSION 3.20)  # Require CMake 3.20 or higher
project(Vault VERSION 1.0 LANGUAGES CXX)  # Set project name and version

# Set C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find OpenSSL (required for SHA-1 hashing)
find_package(OpenSSL REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# List all source files
set(SOURCES
    src/vault.cpp
    src/init.cpp
    src/add.cpp
    src/commit.cpp
    src/log.cpp
    src/CommitNode.cpp
    src/main.cpp
)

# Add the executable
add_executable(vault ${SOURCES})

# Link OpenSSL
target_link_libraries(vault OpenSSL::Crypto)

# Enable warnings
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(vault PRIVATE -Wall -Wextra -pedantic)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(vault PRIVATE /W4 /WX)
endif()

# Configure CMake to generate minimal build files
set(CMAKE_VERBOSE_MAKEFILE OFF)  # Disable verbose output
set(CMAKE_SUPPRESS_REGENERATION ON)  # Suppress unnecessary regeneration
