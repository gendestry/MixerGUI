# Dev shell providing the native libraries GLFW/OpenGL need to build & run.
# Usage:  nix-shell --run 'cmake -S . -B build && cmake --build build'
{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  nativeBuildInputs = with pkgs; [
    cmake
    pkg-config
    gcc
    # GLAD's generator is a Python tool that needs jinja2
    (python3.withPackages (ps: [ ps.jinja2 ]))
  ];

  buildInputs = with pkgs; [
    # X11 backend
    libx11
    libxrandr
    libxinerama
    libxcursor
    libxi
    libxext
    # Wayland backend (optional; GLFW_BUILD_WAYLAND is OFF by default here)
    wayland
    wayland-protocols
    libxkbcommon
    # OpenGL
    libGL
    libGLU
  ];

  # So the built binary can find libGL at runtime inside the shell.
  LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath [ pkgs.libGL ];
}
