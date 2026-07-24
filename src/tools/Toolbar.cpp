#include "Application.h"
#include "UiHelpers.h"

#include <imgui.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <utility>
#include <vector>

// Vertical strip of icon-only tool buttons. Positioning / camera controls live
// in the separate Transform window (renderTransformWindow).
void Application::renderToolbar() {
  ImGui::Begin("Toolbar");

  if (iconButton("##select", m_tool == Tool::Select, "Select (marquee)",
                 drawSelectIcon))
    m_tool = Tool::Select;
  if (iconButton("##move", m_tool == Tool::Move, "Move selected on ground",
                 drawMoveIcon))
    m_tool = Tool::Move;
  if (iconButton("##clear", false, "Clear programmer (C)", drawClearIcon))
    clearProgrammer();
  if (iconButton("##patch", false, "Patch fixtures / universe view",
                 drawPatchIcon))
    m_openPatchPopup = true;

  ImGui::End();
}

// 3D manipulation: selection position and camera controls.
void Application::renderTransformWindow() {
  ImGui::Begin("Transform");

  ImGui::TextDisabled(m_tool == Tool::Select
                          ? "Left-drag: marquee select"
                          : "Left-drag: move selected on ground");
  ImGui::Separator();

  // Gather the selected fixtures (order = patch order); the spread editors
  // interpolate a range across this ordering.
  std::vector<FixtureCube *> sel;
  for (auto &fc : m_fixtures)
    if (fc.cube().selected())
      sel.push_back(&fc);
  const int n = int(sel.size());
  ImGui::Text("%d selected", n);

  if (n > 0) {
    // Position: single value (all equal) or from..to spread per axis.
    axisSpreadEditor(
        "Position", n, m_posRange, 0.05f,
        [&](int a) {
          return std::pair<float, float>(sel.front()->cube().position()[a],
                                         sel.back()->cube().position()[a]);
        },
        [&](int a, int i, float v) {
          glm::vec3 p = sel[i]->cube().position();
          p[a] = v;
          sel[i]->cube().setPosition(p);
        });

    // Rotation (degrees): applied to the whole group as a rigid body about its
    // centroid, not per-fixture. Edits are applied as the delta since last
    // frame so the selection orbits the centroid.
    glm::vec3 centroid(0.0f);
    for (auto *fc : sel)
      centroid += fc->cube().position();
    centroid /= float(n);

    ImGui::TextUnformatted("Rotation (group)");
    if (ImGui::DragFloat3("##grouprot", &m_groupRot.x, 0.5f)) {
      glm::vec3 d = m_groupRot - m_groupRotPrev;
      glm::mat4 R(1.0f);
      R = glm::rotate(R, glm::radians(d.x), glm::vec3(1.0f, 0.0f, 0.0f));
      R = glm::rotate(R, glm::radians(d.y), glm::vec3(0.0f, 1.0f, 0.0f));
      R = glm::rotate(R, glm::radians(d.z), glm::vec3(0.0f, 0.0f, 1.0f));
      for (auto *fc : sel) {
        glm::vec3 p = fc->cube().position();
        glm::vec3 rel = p - centroid;
        rel = glm::vec3(R * glm::vec4(rel, 1.0f));
        fc->cube().setPosition(centroid + rel);
        fc->cube().setRotation(fc->cube().rotation() + d);
      }
    }
    m_groupRotPrev = m_groupRot;

    if (ImGui::Button("Drop to ground (y=0)")) {
      for (auto *fc : sel) {
        glm::vec3 p = fc->cube().position();
        p.y = 0.0f;
        fc->cube().setPosition(p);
      }
    }
  } else {
    ImGui::TextDisabled("(select fixtures to position them)");
  }

  ImGui::Separator();
  ImGui::Text("Camera");
  ImGui::DragFloat("zoom", &m_camDist, 0.2f, 1.0f, 200.0f);
  ImGui::DragFloat3("target", &m_camTarget.x, 0.05f);

  // Orbit angles, edited in degrees; stored as radians.
  float yaw = glm::degrees(m_camYaw);
  float pitch = glm::degrees(m_camPitch);
  if (ImGui::DragFloat("yaw", &yaw, 0.5f))
    m_camYaw = glm::radians(yaw);
  if (ImGui::DragFloat("pitch", &pitch, 0.5f, -89.0f, 89.0f))
    m_camPitch = glm::radians(pitch);

  ImGui::TextDisabled("Scroll: zoom  -  Right-drag: pan  -  Middle: orbit");

  ImGui::End();
}
