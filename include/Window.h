#pragma once
#include <glad/gl.h>
// GLFW_INCLUDE_NONE stops glfw3.h from pulling in the system GL header, so the
// include order of these two no longer matters (clang-format may reorder them).
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <cstdio>

inline void glfw_error_callback(int error, const char *description) {
  std::fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

class Window {
  // GLFWwindow* window;
public:
  GLFWwindow *window;

  ~Window() {
    glfwDestroyWindow(window);
    glfwTerminate();
  }

  bool init() {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
      std::fprintf(stderr, "Failed to initialize GLFW\n");
      return false;
    }

    const char *glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window = glfwCreateWindow(1280, 720, "guitest", nullptr, nullptr);
    if (!window) {
      std::fprintf(stderr, "Failed to create GLFW window\n");
      glfwTerminate();
      return false;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGL(glfwGetProcAddress)) {
      std::fprintf(stderr, "Failed to initialize GLAD\n");
      return false;
    }
    std::printf("OpenGL %s\n", glGetString(GL_VERSION));

    glEnable(GL_DEPTH_TEST);
    return true;
  }

  bool shouldClose() const { return glfwWindowShouldClose(window); }
};
