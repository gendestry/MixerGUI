cmake_minimum_required(VERSION 3.16)
project(guitest VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# --------------------------------------------------
# Third-party libraries (lib/)
# --------------------------------------------------

# GLFW -- build only the library
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
add_subdirectory(lib/glfw)

# GLM -- header-only math library (target: glm::glm)
add_subdirectory(lib/glm)

# GLAD (Dav1dde/glad) -- generates a loader target: glad_gl_core_33
add_subdirectory(lib/glad/cmake glad_cmake)
glad_add_library(glad_gl_core_33 STATIC API gl:core=3.3)

# Dear ImGui -- no CMake shipped, build as a static lib ourselves
add_library(imgui STATIC
    lib/imgui/imgui.cpp
    lib/imgui/imgui_draw.cpp
    lib/imgui/imgui_tables.cpp
    lib/imgui/imgui_widgets.cpp
    lib/imgui/imgui_demo.cpp
    lib/imgui/backends/imgui_impl_glfw.cpp
    lib/imgui/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
    lib/imgui
    lib/imgui/backends
)
target_link_libraries(imgui PUBLIC glfw)

# LightEngine (github.com/gendestry/LightEngine) -- has its own CMake, but it
# pulls nested submodules (Utils, NewSyntaxParser). Only add it if those are
# present so a fresh checkout without --recursive still configures.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/LightEngine/lib/Utils/CMakeLists.txt")
    add_subdirectory(lib/LightEngine)
    set(HAVE_LIGHTENGINE ON)
else()
    message(WARNING
        "LightEngine submodules not initialized -- skipping. Run:\n"
        "  git submodule update --init --recursive lib/LightEngine")
    set(HAVE_LIGHTENGINE OFF)
endif()

# --------------------------------------------------
# Example executable (src/)
# --------------------------------------------------
file(GLOB GUITEST_SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_executable(guitest ${GUITEST_SOURCES})
target_include_directories(guitest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(guitest PRIVATE
    glfw
    glad_gl_core_33
    glm::glm
    imgui
)

if(HAVE_LIGHTENGINE)
    target_link_libraries(guitest PRIVATE LightEngine::LightEngine)
    target_compile_definitions(guitest PRIVATE HAVE_LIGHTENGINE
        LE_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/lib/LightEngine/include/LightEngine/Commands/data")
endif()

# OpenGL is needed on some platforms for linking
find_package(OpenGL REQUIRED)
target_link_libraries(guitest PRIVATE OpenGL::GL)

# Copy shader/data files next to the executable after each build
add_custom_command(TARGET guitest POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/data $<TARGET_FILE_DIR:guitest>/data)
