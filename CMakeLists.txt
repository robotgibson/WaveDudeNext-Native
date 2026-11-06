cmake_minimum_required(VERSION 3.16)
project(WaveGuideNative CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -ffast-math")

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(OpenMP QUIET)

# --- Automated Dependency Fetching for Dear ImGui ---
include(FetchContent)
FetchContent_Declare(
    imgui
    URL "https://github.com/ocornut/imgui/archive/refs/tags/v1.91.5.tar.gz"
)
FetchContent_MakeAvailable(imgui)

# Gather our core local layout source files
file(GLOB SOURCES "src/*.cpp" "src/engine/*.cpp")

# Define the final compiled executable target block
add_executable(${PROJECT_NAME} ${SOURCES}
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE 
    include
    ${SDL2_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

# FIXED: Linked standard package dependencies including native SDL2_image configurations
target_link_libraries(${PROJECT_NAME} PRIVATE 
    ${SDL2_LIBRARIES}
    ${OPENGL_LIBRARIES}
    SDL2_image
    m
)

if(OpenMP_CXX_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenMP::OpenMP_CXX)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAS_OPENMP)
    message(STATUS "OpenMP multi-threading acceleration enabled.")
else()
    message(STATUS "OpenMP not found. Compiling fallback single-threaded build.")
endif()
