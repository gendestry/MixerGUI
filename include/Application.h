#pragma once

#include "FixtureCube.h"
#include "Shader.h"
#include "Window.h"

#include "LightEngine/Engine/Engine.h"

#include <glm/glm.hpp>

#include <memory>
#include <vector>

// Owns the window, GL resources and the main render loop.
class Application {
public:
  Application() = default;

  // Initializes the window, ImGui, shaders and scene. Returns false on failure.
  bool init();

  // Runs the main loop until the window is closed.
  void run();

  ~Application();

private:
  void initImGui();

  // Patches `amount` RGB fixtures onto `universe`, creating cubes for them.
  void patchFixtures(uint16_t universe, uint16_t amount);
  // Repositions all cubes into a row along X.
  void layoutFixtures();

  void renderUI();
  void renderPatchWindow();
  void renderFixtureListWindow();
  void update(float dt);
  void render();

  // Rubber-band selection: track the drag rectangle and, on release, select
  // every cube whose screen-space center falls inside it.
  void handleSelection();

  Window m_window;
  std::unique_ptr<Shader> m_shader;
  LightEngine::Engine::Engine m_engine;
  std::vector<FixtureCube> m_fixtures;

  glm::vec3 m_clearColor{0.10f, 0.12f, 0.15f};
  bool m_spinning = true;
  float m_angle = 0.0f;
  float m_rowWidth = 0.0f;

  // Patch-window inputs.
  int m_patchUniverse = 1;
  int m_patchAmount = 10;

  // Camera matrices from the last render(), used for screen-space projection.
  glm::mat4 m_view{1.0f};
  glm::mat4 m_proj{1.0f};
  int m_fbWidth = 1;
  int m_fbHeight = 1;

  // Rubber-band drag state (screen pixels).
  bool m_dragging = false;
  glm::vec2 m_dragStart{0.0f};
};
