#include "Application.h"

#include <imgui.h>

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
