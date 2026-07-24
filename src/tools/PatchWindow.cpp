#include "Application.h"

#include <imgui.h>

// Popup opened from the toolbar's Patch button. Holds the patch options at the
// top and, below, a live view of every patched universe.
void Application::renderPatchPopup() {
  // The toolbar sets m_openPatchPopup; actually open the popup here so the
  // OpenPopup/BeginPopup calls share the same ID scope.
  if (m_openPatchPopup) {
    ImGui::OpenPopup("Patch");
    m_openPatchPopup = false;
  }

  // Center the popup on first appearance and give it a sensible default size.
  ImGui::SetNextWindowSize(ImVec2(560, 480), ImGuiCond_FirstUseEver);
  ImVec2 center = ImGui::GetMainViewport()->GetCenter();
  ImGui::SetNextWindowPos(center, ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));

  if (!ImGui::BeginPopupModal("Patch", nullptr, ImGuiWindowFlags_NoSavedSettings))
    return;

  // --- Patch options ---
  ImGui::TextUnformatted("Patch options");
  ImGui::InputInt("universe", &m_patchUniverse);
  ImGui::InputInt("amount", &m_patchAmount);
  if (m_patchUniverse < 0)
    m_patchUniverse = 0;
  if (m_patchAmount < 0)
    m_patchAmount = 0;
  ImGui::Checkbox("add to group", &m_patchAsGroup);
  if (ImGui::Button("Patch"))
    patchFixtures(uint16_t(m_patchUniverse), uint16_t(m_patchAmount),
                  m_patchAsGroup);
  ImGui::SameLine();
  if (ImGui::Button("Close"))
    ImGui::CloseCurrentPopup();

  ImGui::Separator();

  // --- Universe view (multiple universes) ---
  ImGui::TextUnformatted("Universes");
  ImGui::BeginChild("universeView", ImVec2(0, 0), ImGuiChildFlags_Borders);
  renderUniverseView();
  ImGui::EndChild();

  ImGui::EndPopup();
}
