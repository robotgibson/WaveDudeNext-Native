cmake_minimum_required(VERSION 3.16)
project(WaveGuideNative CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -ffast-math")

# Look inside local tracking subdirectory for nested module includes
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Find core layout platform packages
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(OpenMP QUIET)

# Pull in the URL and dependency targets from our isolated module file
include(Dependencies)

# Construct the custom TinyEXR interface header wrapper
# TINYEXR_USE_MINIZ=0 forces the single-header to use standard zlib definitions instead
add_library(tinyexr_interface INTERFACE)
target_include_directories(tinyexr_interface INTERFACE ${tinyexr_SOURCE_DIR})
target_compile_definitions(tinyexr_interface INTERFACE TINYEXR_USE_MINIZ=0)

# Automatically fetch local custom engine and layout logic objects
file(GLOB SOURCES "src/*.cpp" "src/engine/*.cpp")

# Define the final binary and tie the local ImGui backend objects down
add_executable(${PROJECT_NAME} ${SOURCES}
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

# Bind include lookups for nested engine components
target_include_directories(${PROJECT_NAME} PRIVATE 
    include
    ${SDL2_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIRS}
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

# Process cross-platform linkage blocks
target_link_libraries(${PROJECT_NAME} PRIVATE 
    ${SDL2_LIBRARIES}
    ${OPENGL_LIBRARIES}
    SDL2_image
    ZLIB::ZLIB
    tinyexr_interface
    m
)

# Evaluate and wire hardware parallelization metrics
if(OpenMP_CXX_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenMP::OpenMP_CXX)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAS_OPENMP)
    message(STATUS "OpenMP multi-threading acceleration enabled.")
else()
    message(STATUS "OpenMP not found. Compiling fallback single-threaded build.")
endif()
