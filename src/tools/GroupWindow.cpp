#include "Application.h"

#include <imgui.h>

#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

void Application::renderGroupWindow() {
  ImGui::Begin("Groups");

  // Store the current selection as a new group (next free slot).
  int selCount = int(m_engine.programmer().selection().size());
  ImGui::BeginDisabled(selCount == 0);
  if (ImGui::Button("Store selection as group")) {
    m_engine.storeGroup();
  }
  ImGui::EndDisabled();
  ImGui::SameLine();
  ImGui::TextDisabled("(%d selected)", selCount);
  ImGui::Separator();

  const auto &groups = m_engine.stored().groups();
  if (groups.empty()) {
    ImGui::TextDisabled("No groups stored.");
  } else {
    for (const auto &[num, g] : groups) {
      ImGui::PushID(int(num));
      const std::string &name = g->name();
      std::vector<uint16_t> fids = g->fids();

      char label[160];
      std::snprintf(label, sizeof(label), "#%u %s  (%zu fixtures)",
                    unsigned(num), name.empty() ? "(unnamed)" : name.c_str(),
                    fids.size());

      if (ImGui::Button("Select")) {
        if (ImGui::GetIO().KeyShift) {
          // Shift-click: add this group's fixtures to the current selection.
          std::vector<uint16_t> sel = m_engine.programmer().selection().fids();
          for (uint16_t f : fids)
            if (std::find(sel.begin(), sel.end(), f) == sel.end())
              sel.push_back(f);
          m_engine.programmer().select(sel);
        } else {
          m_engine.selectGroup(num);
        }
        m_engine.update();
        syncCubesFromEngine();
      }
      ImGui::SameLine();
      if (ImGui::TreeNode(label)) {
        std::string ids;
        for (size_t i = 0; i < fids.size(); ++i) {
          if (i)
            ids += ", ";
          ids += std::to_string(fids[i]);
        }
        ImGui::TextWrapped("FIDs: %s", ids.c_str());
        ImGui::TreePop();
      }
      ImGui::PopID();
    }
  }

  ImGui::End();
}
