cmake_minimum_required(VERSION 3.16)
project(WaveDudeNative CXX)

# Force modern C++ standards
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Force aggressive native optimisations (Critical for stencil math performance)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -ffast-math")

# Find native SDL2 system packages on Ubuntu
find_package(SDL2 REQUIRED)

# Aggregate our internal source paths
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
)

# Define our native executable target
add_executable(${PROJECT_NAME} ${SOURCES})

# Point compiler to our clean include folders
target_include_directories(${PROJECT_NAME} PRIVATE 
    include
    ${SDL2_INCLUDE_DIRS}
)

# Link SDL2 to our binary application
target_link_libraries(${PROJECT_NAME} PRIVATE 
    ${SDL2_LIBRARIES}
    m # Math library for wave trigonometric operations
)

