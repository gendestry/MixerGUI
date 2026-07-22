#include "Application.h"
#include "UiHelpers.h"

#include <imgui.h>

#include <glm/glm.hpp>

void Application::renderToolbar() {
  ImGui::Begin("Toolbar");

  // Tool mode (icon buttons).
  if (iconButton("##select", m_tool == Tool::Select, "Select (marquee)",
                 drawSelectIcon))
    m_tool = Tool::Select;
  ImGui::SameLine();
  if (iconButton("##move", m_tool == Tool::Move, "Move selected on ground",
                 drawMoveIcon))
    m_tool = Tool::Move;
  ImGui::SameLine();
  if (iconButton("##clear", false, "Clear programmer (C)", drawClearIcon))
    clearProgrammer();

  ImGui::TextDisabled(m_tool == Tool::Select
                          ? "Left-drag: marquee select"
                          : "Left-drag: move selected on ground");
  ImGui::Separator();

  // Count selection and compute its centroid.
  glm::vec3 center(0.0f);
  int n = 0;
  for (auto &fc : m_fixtures)
    if (fc.cube().selected()) {
      center += fc.cube().position();
      ++n;
    }
  ImGui::Text("%d selected", n);

  if (n > 0) {
    center /= float(n);
    // Editing the centroid moves the whole selection by the delta.
    glm::vec3 newCenter = center;
    if (ImGui::DragFloat3("position", &newCenter.x, 0.05f)) {
      glm::vec3 delta = newCenter - center;
      for (auto &fc : m_fixtures)
        if (fc.cube().selected())
          fc.cube().setPosition(fc.cube().position() + delta);
    }
    if (ImGui::Button("Drop to ground (y=0)")) {
      for (auto &fc : m_fixtures)
        if (fc.cube().selected()) {
          glm::vec3 p = fc.cube().position();
          p.y = 0.0f;
          fc.cube().setPosition(p);
        }
    }
  } else {
    ImGui::TextDisabled("(select fixtures to position them)");
  }

  ImGui::Separator();
  ImGui::Text("Camera");
  ImGui::DragFloat("zoom", &m_camDist, 0.2f, 1.0f, 200.0f);
  ImGui::DragFloat3("target", &m_camTarget.x, 0.05f);
  ImGui::TextDisabled("Scroll: zoom  -  Right-drag: pan");

  ImGui::End();
}
