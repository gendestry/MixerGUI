#pragma once

#include "FixtureCube.h"
#include "Shader.h"
#include "Window.h"

#include "LightEngine/Engine/Engine.h"

#include <glm/glm.hpp>

#include <memory>
#include <optional>
#include <string>
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
  // When `asGroup` is set, the newly patched fixtures are stored as a new group.
  void patchFixtures(uint16_t universe, uint16_t amount, bool asGroup = false);
  // Repositions all cubes into a row along X.
  void layoutFixtures();

  void renderUI();
  void renderToolbar();
  // 3D manipulation: selection position + camera controls.
  void renderTransformWindow();
  // Popup (opened from the toolbar) holding the patch options + universe view.
  void renderPatchPopup();
  // Renders the multi-universe channel grid (no window Begin/End of its own).
  void renderUniverseView();
  void renderFixtureListWindow();
  void renderGroupWindow();
  void renderColorPresetWindow();
  void renderDimmerPresetWindow();
  void renderCommandWindow();
  void runCommand(const char *line);

  // Global keyboard shortcuts (only when not typing in a field).
  void handleHotkeys();
  // Clear the programmer (selection + edits) and reflect it everywhere.
  void clearProgrammer();

  // Selection is unified: the engine's programmer selection is the single
  // source of truth. Push cube flags -> engine, or pull engine -> cube flags.
  void syncEngineFromCubes();
  void syncCubesFromEngine();
  void update(float dt);
  void render();

  // Rubber-band selection: track the drag rectangle and, on release, select
  // every cube whose screen-space center falls inside it.
  void handleSelection();
  // Move tool: drag the selected fixtures across the ground (XZ) plane.
  void handleMove();
  // Camera zoom (scroll) and pan (right-drag).
  void handleCamera();

  // Intersect the mouse ray with the ground plane (y = 0). Screen coords in
  // ImGui logical points.
  std::optional<glm::vec3> groundHit(const glm::vec2 &mouse) const;

  Window m_window;
  std::unique_ptr<Shader> m_shader;
  LightEngine::Engine::Engine m_engine;
  std::vector<FixtureCube> m_fixtures;

  glm::vec3 m_clearColor{0.10f, 0.12f, 0.15f};
  bool m_spinning = false;
  float m_angle = 0.0f;
  float m_rowWidth = 0.0f;

  // Editing tool.
  enum class Tool { Select, Move };
  Tool m_tool = Tool::Select;

  // Per-axis single/range toggle for the Transform window's position editor.
  bool m_posRange[3] = {false, false, false};
  // Group rotation (euler degrees) applied rigidly around the selection
  // centroid. Edits are applied as the delta from the previous frame's value.
  glm::vec3 m_groupRot{0.0f};
  glm::vec3 m_groupRotPrev{0.0f};

  // Patch-window inputs.
  int m_patchUniverse = 1;
  int m_patchAmount = 10;
  // When set, patching also stores the new fixtures as a group.
  bool m_patchAsGroup = false;
  // Set by the toolbar button to open the patch popup on the next frame.
  bool m_openPatchPopup = false;

  // Camera matrices from the last render(), used for screen-space projection.
  glm::mat4 m_view{1.0f};
  glm::mat4 m_proj{1.0f};
  int m_fbWidth = 1;
  int m_fbHeight = 1;

  // Orbit/pan camera. Yaw/pitch (radians) orbit the target; middle-drag edits
  // them. Defaults match the old fixed heading (~21.8 deg above +Z).
  glm::vec3 m_camTarget{0.0f};
  float m_camDist = 15.0f;
  float m_camYaw = 0.0f;
  float m_camPitch = 0.381f;

  // Rubber-band drag state (screen pixels).
  bool m_dragging = false;
  glm::vec2 m_dragStart{0.0f};

  // Move-tool drag state.
  bool m_moving = false;
  glm::vec3 m_prevGround{0.0f};

  // Command console.
  char m_cmdInput[256] = {0};
  std::vector<std::string> m_cmdLog;
  bool m_commandsLoaded = false;
};
