#include "Application.h"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>

#include <glm/gtc/matrix_transform.hpp>

#include "LightEngine/Engine/FixtureBuilder.h"
#include "LightEngine/Fixture/Fixture.h"
#include "LightEngine/GDTF/LogicalChannel.h"

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

const char *attributeName(LightEngine::GDTF::Attribute a) {
  using LightEngine::GDTF::Attribute;
  switch (a) {
  case Attribute::DIMMER:
    return "Dimmer";
  case Attribute::VDIMMER:
    return "Virtual Dimmer";
  case Attribute::COLOR_R:
    return "Red";
  case Attribute::COLOR_G:
    return "Green";
  case Attribute::COLOR_B:
    return "Blue";
  case Attribute::COLOR_W:
    return "White";
  }
  return "Unknown";
}
} // namespace

bool Application::init() {
  if (!m_window.init())
    return false;

  initImGui();
  m_shader = std::make_unique<Shader>("data/cube.vert", "data/cube.frag");
  patchFixtures(1, 10);
  return true;
}

void Application::initImGui() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  ImGui::StyleColorsDark();
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
}

void Application::layoutFixtures() {
  const float spacing = 2.0f;
  const size_t n = m_fixtures.size();
  m_rowWidth = spacing * (n > 0 ? n - 1 : 0);
  for (size_t i = 0; i < n; ++i) {
    float hue = n > 0 ? float(i) / float(n) : 0.0f;
    m_fixtures[i].cube().setColor(hsv2rgb(hue, 0.7f, 0.9f));
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

    renderUI();
    handleSelection();
    update(ImGui::GetIO().DeltaTime);
    render();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(m_window.window);
  }
}

void Application::renderUI() {
  ImGui::Begin("Scene");
  ImGui::Checkbox("spin", &m_spinning);
  ImGui::SliderFloat("angle", &m_angle, 0.0f, 360.0f);
  ImGui::ColorEdit3("clear color", &m_clearColor.x);
  ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
  ImGui::End();

  renderPatchWindow();
  renderFixtureListWindow();
}

void Application::renderPatchWindow() {
  ImGui::Begin("Patch");
  ImGui::InputInt("universe", &m_patchUniverse);
  ImGui::InputInt("amount", &m_patchAmount);
  if (m_patchUniverse < 0)
    m_patchUniverse = 0;
  if (m_patchAmount < 0)
    m_patchAmount = 0;
  if (ImGui::Button("Patch"))
    patchFixtures(uint16_t(m_patchUniverse), uint16_t(m_patchAmount));
  ImGui::End();
}

void Application::renderFixtureListWindow() {
  ImGui::Begin("Fixtures");
  ImGui::Text("%zu fixtures patched", m_fixtures.size());
  ImGui::Separator();

  ImGui::TextDisabled("Click to select, Ctrl+Click to multi-select");
  ImGui::Separator();

  for (size_t i = 0; i < m_fixtures.size(); ++i) {
    FixtureCube &fc = m_fixtures[i];
    const uint16_t fid = fc.fids().front();
    std::shared_ptr<LightEngine::Fixtures::Fixture> fx = m_engine.getFixture(fid);

    ImGui::PushID(int(i));

    const char *name = fx ? fx->Name().c_str() : "?";
    char label[128];
    std::snprintf(label, sizeof(label), "FID %u  -  %s", unsigned(fid), name);

    // Selectable row drives selection; Ctrl adds/toggles, plain click replaces.
    if (ImGui::Selectable(label, fc.cube().selected(),
                          ImGuiSelectableFlags_SpanAllColumns)) {
      bool ctrl = ImGui::GetIO().KeyCtrl;
      if (!ctrl)
        for (auto &other : m_fixtures)
          other.cube().setSelected(false);
      fc.cube().setSelected(ctrl ? !fc.cube().selected() : true);
    }

    if (ImGui::TreeNode("details")) {
      if (fx) {
        // DMX address is 1-based; `start` is the 0-based offset in the universe.
        ImGui::Text("Universe : %u", unsigned(fx->Universe()));
        ImGui::Text("Address  : %u", unsigned(fx->start + 1));
        ImGui::Text("Footprint: %u channels", unsigned(fx->Footprint()));
        ImGui::Text("Color cells: %zu", fx->CellCount());

        ImGui::Text("Channels:");
        ImGui::Indent();
        uint32_t ch = 1;
        for (const auto &p : fx->Parameters()) {
          ImGui::BulletText("Ch %u: %s", unsigned(fx->start + ch),
                            attributeName(p.Attribute()));
          ++ch;
        }
        ImGui::Unindent();
      } else {
        ImGui::TextDisabled("(fixture data unavailable)");
      }

      ImGui::ColorEdit3("cube color", &fc.cube().color().x,
                        ImGuiColorEditFlags_NoInputs);
      ImGui::TreePop();
    }

    ImGui::PopID();
  }
  ImGui::End();
}

void Application::handleSelection() {
  ImGuiIO &io = ImGui::GetIO();
  // Ignore drags that start on / interact with an ImGui window.
  if (io.WantCaptureMouse && !m_dragging)
    return;

  const glm::vec2 mouse(io.MousePos.x, io.MousePos.y);

  if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
    m_dragging = true;
    m_dragStart = mouse;
  }

  if (!m_dragging)
    return;

  // Screen-space rectangle (in ImGui/window pixels).
  glm::vec2 lo = glm::min(m_dragStart, mouse);
  glm::vec2 hi = glm::max(m_dragStart, mouse);

  // Draw the marquee.
  ImGui::GetForegroundDrawList()->AddRectFilled(
      ImVec2(lo.x, lo.y), ImVec2(hi.x, hi.y), IM_COL32(255, 205, 25, 40));
  ImGui::GetForegroundDrawList()->AddRect(ImVec2(lo.x, lo.y), ImVec2(hi.x, hi.y),
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
  }
}

void Application::update(float dt) {
  if (m_spinning)
    m_angle += dt * 45.0f;
  if (m_angle >= 360.0f)
    m_angle -= 360.0f;

  // Spin each cube in place; its row position was set in setupFixtures().
  for (auto &fc : m_fixtures)
    fc.cube().setRotation(glm::vec3(0.0f, m_angle, 0.0f));
}

void Application::render() {
  glfwGetFramebufferSize(m_window.window, &m_fbWidth, &m_fbHeight);
  glViewport(0, 0, m_fbWidth, m_fbHeight);
  glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  float dist = m_rowWidth * 0.6f + 5.0f;
  m_view = glm::lookAt(glm::vec3(0.0f, 2.0f, dist), glm::vec3(0.0f),
                       glm::vec3(0.0f, 1.0f, 0.0f));
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
