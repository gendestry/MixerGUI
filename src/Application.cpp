#include "Application.h"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>

#include <glm/gtc/matrix_transform.hpp>

#include "LightEngine/Engine/FixtureBuilder.h"
#include "LightEngine/Fixture/Fixture.h"
#include "LightEngine/GDTF/LogicalChannel.h"

#include <algorithm>
#include <cmath>

namespace {
// h in [0,1], s/v in [0,1] -> RGB.
glm::vec3 hsv2rgb(float h, float s, float v) {
  float r = std::fabs(h * 6.0f - 3.0f) - 1.0f;
  float g = 2.0f - std::fabs(h * 6.0f - 2.0f);
  float b = 2.0f - std::fabs(h * 6.0f - 4.0f);
  glm::vec3 c = glm::clamp(glm::vec3(r, g, b), 0.0f, 1.0f);
  return v * glm::mix(glm::vec3(1.0f), c, s);
}
} // namespace

bool Application::init() {
  if (!m_window.init())
    return false;

  initImGui();
  m_shader = std::make_unique<Shader>("data/cube.vert", "data/cube.frag");

#ifdef LE_DATA_DIR
  m_engine.loadCommands(LE_DATA_DIR "/commands.txt",
                        LE_DATA_DIR "/commands.syn");
  m_commandsLoaded = true;
  m_cmdLog.emplace_back("Commands loaded.");
#else
  m_cmdLog.emplace_back("LE_DATA_DIR not defined; commands unavailable.");
#endif

  // Start with an empty scene; fixtures are added via the Patch window.
  return true;
}

void Application::initImGui() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
  ImGui::StyleColorsDark();

  // When viewports are enabled, tweak WindowRounding/WindowBg so platform
  // windows (dragged outside the main window) look identical to internal ones.
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
  }
  ImGui_ImplGlfw_InitForOpenGL(m_window.window, true);
  ImGui_ImplOpenGL3_Init("#version 330");
}

void Application::patchFixtures(uint16_t universe, uint16_t amount) {
  using namespace LightEngine;
  using GDTF::Attribute;

  if (amount == 0)
    return;

  Fixtures::Fixture rgb =
      Engine::FixtureBuilder(
          "RGB", {Attribute::COLOR_R, Attribute::COLOR_G, Attribute::COLOR_B})
          .Get();

  auto fids = m_engine.patch(rgb, universe, amount);

  m_fixtures.reserve(m_fixtures.size() + fids.size());
  for (uint16_t fid : fids) {
    FixtureCube fc({fid}, universe);
    fc.cube().setColor(hsv2rgb(0.6f, 0.7f, 0.9f));
    m_fixtures.push_back(std::move(fc));
  }
  layoutFixtures();
  m_camDist = m_rowWidth * 0.6f + 8.0f;
}

void Application::layoutFixtures() {
  const float spacing = 2.0f;
  const size_t n = m_fixtures.size();
  m_rowWidth = spacing * (n > 0 ? n - 1 : 0);
  for (size_t i = 0; i < n; ++i) {
    // Color is driven by the engine each frame (see update()); only lay out
    // the row position here.
    m_fixtures[i].cube().setPosition(
        glm::vec3(spacing * float(i) - m_rowWidth * 0.5f, 0.0f, 0.0f));
  }
}

void Application::run() {
  while (!m_window.shouldClose()) {
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Full-viewport dockspace: windows can be docked/snapped to edges, split
    // and tabbed. PassthruCentralNode keeps the 3D scene visible in the middle.
    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(),
                                 ImGuiDockNodeFlags_PassthruCentralNode);
    handleHotkeys();
    renderUI();
    handleCamera();
    if (m_tool == Tool::Select)
      handleSelection();
    else
      handleMove();
    update(ImGui::GetIO().DeltaTime);
    render();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Render any windows that were dragged outside the main window as their own
    // OS-level windows. Must restore our GL context afterwards.
    ImGuiIO &io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
      GLFWwindow *backup = glfwGetCurrentContext();
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
      glfwMakeContextCurrent(backup);
    }

    glfwSwapBuffers(m_window.window);
  }
}

void Application::renderUI() {
  // ImGui::Begin("Scene");
  // ImGui::Checkbox("spin", &m_spinning);
  // ImGui::SliderFloat("angle", &m_angle, 0.0f, 360.0f);
  // ImGui::ColorEdit3("clear color", &m_clearColor.x);
  // ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
  // ImGui::End();

  renderToolbar();
  renderPatchWindow();
  renderFixtureListWindow();
  renderGroupWindow();
  renderColorPresetWindow();
  renderDimmerPresetWindow();
  renderCommandWindow();
}

void Application::clearProgrammer() {
  m_engine.clear();
  m_engine.update();
  syncCubesFromEngine();
}

void Application::handleHotkeys() {
  ImGuiIO &io = ImGui::GetIO();
  // Only act as a global shortcut when no text field is capturing input.
  if (!io.WantTextInput && ImGui::IsKeyPressed(ImGuiKey_C, false)) {
    clearProgrammer();
    // Swallow the 'c' so it doesn't also get typed into the command line.
    io.InputQueueCharacters.resize(0);
  }
}

void Application::syncEngineFromCubes() {
  std::vector<uint16_t> fids;
  for (auto &fc : m_fixtures)
    if (fc.cube().selected())
      fids.push_back(fc.fids().front());
  m_engine.programmer().select(fids);
}

void Application::syncCubesFromEngine() {
  std::vector<uint16_t> sel = m_engine.programmer().selection().fids();
  for (auto &fc : m_fixtures) {
    bool s = std::find(sel.begin(), sel.end(), fc.fids().front()) != sel.end();
    fc.cube().setSelected(s);
  }
}

std::optional<glm::vec3> Application::groundHit(const glm::vec2 &mouse) const {
  ImGuiIO &io = ImGui::GetIO();
  if (io.DisplaySize.x <= 0 || io.DisplaySize.y <= 0)
    return std::nullopt;

  // Mouse (logical points) -> NDC.
  float ndcX = 2.0f * mouse.x / io.DisplaySize.x - 1.0f;
  float ndcY = 1.0f - 2.0f * mouse.y / io.DisplaySize.y;

  glm::mat4 invVP = glm::inverse(m_proj * m_view);
  glm::vec4 pNear = invVP * glm::vec4(ndcX, ndcY, -1.0f, 1.0f);
  glm::vec4 pFar = invVP * glm::vec4(ndcX, ndcY, 1.0f, 1.0f);
  if (pNear.w == 0.0f || pFar.w == 0.0f)
    return std::nullopt;
  glm::vec3 origin = glm::vec3(pNear) / pNear.w;
  glm::vec3 dir = glm::normalize(glm::vec3(pFar) / pFar.w - origin);

  // Intersect y = 0.
  if (std::fabs(dir.y) < 1e-6f)
    return std::nullopt;
  float t = -origin.y / dir.y;
  if (t < 0.0f)
    return std::nullopt;
  return origin + dir * t;
}

void Application::handleCamera() {
  ImGuiIO &io = ImGui::GetIO();
  if (io.WantCaptureMouse)
    return;

  // Zoom with the scroll wheel (exponential feel).
  if (io.MouseWheel != 0.0f) {
    m_camDist *= std::pow(0.9f, io.MouseWheel);
    m_camDist = glm::clamp(m_camDist, 1.0f, 200.0f);
  }

  // Pan with the right mouse button: shift the target along the view plane.
  if (ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
    ImVec2 d = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
    ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);
    // Camera basis in world space (rows of the view rotation).
    glm::vec3 right(m_view[0][0], m_view[1][0], m_view[2][0]);
    glm::vec3 up(m_view[0][1], m_view[1][1], m_view[2][1]);
    float scale = m_camDist * 0.0015f;
    m_camTarget += (-d.x * right + d.y * up) * scale;
  }
}

void Application::handleMove() {
  ImGuiIO &io = ImGui::GetIO();
  if (io.WantCaptureMouse && !m_moving)
    return;

  // With multi-viewport enabled, io.MousePos is in OS desktop coordinates;
  // the ground/projection math wants coordinates relative to the main window.
  const ImVec2 vp = ImGui::GetMainViewport()->Pos;
  const glm::vec2 mouse(io.MousePos.x - vp.x, io.MousePos.y - vp.y);

  if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
    if (auto hit = groundHit(mouse)) {
      m_moving = true;
      m_prevGround = *hit;
    }
  }

  if (m_moving) {
    if (auto hit = groundHit(mouse)) {
      glm::vec3 delta = *hit - m_prevGround;
      m_prevGround = *hit;
      for (auto &fc : m_fixtures)
        if (fc.cube().selected())
          fc.cube().setPosition(fc.cube().position() + delta);
    }
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
      m_moving = false;
  }
}

void Application::handleSelection() {
  ImGuiIO &io = ImGui::GetIO();
  // Ignore drags that start on / interact with an ImGui window.
  if (io.WantCaptureMouse && !m_dragging)
    return;

  // With multi-viewport enabled, io.MousePos is in OS desktop coordinates.
  // Keep a window-local copy for projection; the marquee is drawn back in
  // desktop coordinates (the foreground draw list expects those).
  const ImVec2 vp = ImGui::GetMainViewport()->Pos;
  const glm::vec2 mouse(io.MousePos.x - vp.x, io.MousePos.y - vp.y);

  if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
    m_dragging = true;
    m_dragStart = mouse;
  }

  if (!m_dragging)
    return;

  // Screen-space rectangle (window-local logical points).
  glm::vec2 lo = glm::min(m_dragStart, mouse);
  glm::vec2 hi = glm::max(m_dragStart, mouse);

  // Draw the marquee (offset back into desktop coordinates).
  ImGui::GetForegroundDrawList()->AddRectFilled(
      ImVec2(lo.x + vp.x, lo.y + vp.y), ImVec2(hi.x + vp.x, hi.y + vp.y),
      IM_COL32(255, 205, 25, 40));
  ImGui::GetForegroundDrawList()->AddRect(
      ImVec2(lo.x + vp.x, lo.y + vp.y), ImVec2(hi.x + vp.x, hi.y + vp.y),
      IM_COL32(255, 205, 25, 200));

  if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
    m_dragging = false;

    // ImGui coords are in logical points; convert to framebuffer pixels.
    ImVec2 dispSize = io.DisplaySize;
    float sx = dispSize.x > 0 ? float(m_fbWidth) / dispSize.x : 1.0f;
    float sy = dispSize.y > 0 ? float(m_fbHeight) / dispSize.y : 1.0f;

    const glm::mat4 vp = m_proj * m_view;
    for (auto &fc : m_fixtures) {
      glm::vec4 clip = vp * glm::vec4(fc.cube().position(), 1.0f);
      if (clip.w <= 0.0f) { // behind the camera
        fc.cube().setSelected(false);
        continue;
      }
      glm::vec3 ndc = glm::vec3(clip) / clip.w;
      // NDC -> framebuffer pixels -> logical points.
      float px = (ndc.x * 0.5f + 0.5f) * float(m_fbWidth) / sx;
      float py = (1.0f - (ndc.y * 0.5f + 0.5f)) * float(m_fbHeight) / sy;
      bool inside = px >= lo.x && px <= hi.x && py >= lo.y && py <= hi.y;
      fc.cube().setSelected(inside);
    }
    // Propagate the new selection to the engine so it's the same everywhere.
    syncEngineFromCubes();
  }
}

void Application::update(float dt) {
  if (m_spinning)
    m_angle += dt * 45.0f;
  if (m_angle >= 360.0f)
    m_angle -= 360.0f;

  // Compose the engine's layers into this frame's merged values.
  m_engine.update(dt);

  for (auto &fc : m_fixtures) {
    // Read the fixture's live color/intensity from the engine frame.
    const LightEngine::Engine::FixtureValues *v =
        m_engine.values(fc.fids().front());

    // Hue/sat come from the color contribution; brightness from the dimmer.
    // An unlit fixture (no intensity) renders black, mirroring the real state.
    float hue = 0.0f, sat = 0.0f;
    if (v && v->color) {
      hue = v->color->h / 360.0f;
      sat = v->color->s;
    }
    float intensity = (v && v->intensity) ? *v->intensity : 0.0f;
    fc.cube().setColor(hsv2rgb(hue, sat, 1.0f) * intensity);

    // Spin each cube in place; its row position was set in patchFixtures().
    fc.cube().setRotation(glm::vec3(0.0f, m_angle, 0.0f));
  }
}

void Application::render() {
  glfwGetFramebufferSize(m_window.window, &m_fbWidth, &m_fbHeight);
  glViewport(0, 0, m_fbWidth, m_fbHeight);
  glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Eye sits behind and above the target at m_camDist along a fixed heading.
  const glm::vec3 dir = glm::normalize(glm::vec3(0.0f, 0.4f, 1.0f));
  glm::vec3 eye = m_camTarget + dir * m_camDist;
  m_view = glm::lookAt(eye, m_camTarget, glm::vec3(0.0f, 1.0f, 0.0f));
  m_proj = glm::perspective(
      glm::radians(45.0f),
      m_fbHeight > 0 ? float(m_fbWidth) / float(m_fbHeight) : 1.0f, 0.1f,
      100.0f);

  m_shader->use();
  m_shader->setMat4("uView", m_view);
  m_shader->setMat4("uProj", m_proj);
  for (auto &fc : m_fixtures)
    fc.cube().draw(m_shader->id());
}

Application::~Application() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}
