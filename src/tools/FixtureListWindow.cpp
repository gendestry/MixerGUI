#include "Application.h"
#include "UiHelpers.h"

#include <imgui.h>

#include "LightEngine/Fixture/Fixture.h"

#include <cstdio>
#include <memory>
#include <string>

void Application::renderFixtureListWindow() {
  ImGui::Begin("Fixtures");
  ImGui::Text("%zu fixtures patched", m_fixtures.size());
  ImGui::TextDisabled("Click to select, Ctrl+Click to multi-select");
  ImGui::Separator();

  const ImGuiTableFlags flags =
      ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
      ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY |
      ImGuiTableFlags_SizingStretchProp;

  if (ImGui::BeginTable("fixtures", 6, flags)) {
    ImGui::TableSetupScrollFreeze(0, 1); // keep header visible
    ImGui::TableSetupColumn("FID", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Name");
    ImGui::TableSetupColumn("Universe", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Parameters");
    ImGui::TableSetupColumn("Out", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableHeadersRow();

    for (size_t i = 0; i < m_fixtures.size(); ++i) {
      FixtureCube &fc = m_fixtures[i];
      const uint16_t fid = fc.fids().front();
      std::shared_ptr<LightEngine::Fixtures::Fixture> fx =
          m_engine.getFixture(fid);

      ImGui::PushID(int(i));
      ImGui::TableNextRow();

      // Column 0: FID (also the selectable that drives row selection).
      ImGui::TableSetColumnIndex(0);
      char fidLabel[32];
      std::snprintf(fidLabel, sizeof(fidLabel), "%u", unsigned(fid));
      if (ImGui::Selectable(fidLabel, fc.cube().selected(),
                            ImGuiSelectableFlags_SpanAllColumns)) {
        bool ctrl = ImGui::GetIO().KeyCtrl;
        if (!ctrl)
          for (auto &other : m_fixtures)
            other.cube().setSelected(false);
        fc.cube().setSelected(ctrl ? !fc.cube().selected() : true);
        syncEngineFromCubes();
      }

      // Column 1: name.
      ImGui::TableSetColumnIndex(1);
      ImGui::TextUnformatted(fx ? fx->Name().c_str() : "?");

      // Column 2: universe.
      ImGui::TableSetColumnIndex(2);
      ImGui::Text("%u", unsigned(fc.universe()));

      // Column 3: DMX address (1-based) and footprint.
      ImGui::TableSetColumnIndex(3);
      if (fx)
        ImGui::Text("%u (%u ch)", unsigned(fx->start + 1),
                    unsigned(fx->Footprint()));
      else
        ImGui::TextDisabled("-");

      // Column 4: parameters (attribute list).
      ImGui::TableSetColumnIndex(4);
      if (fx) {
        std::string params;
        for (const auto &p : fx->Parameters()) {
          if (!params.empty())
            params += ", ";
          params += attributeName(p.Attribute());
        }
        ImGui::TextUnformatted(params.c_str());
      } else {
        ImGui::TextDisabled("-");
      }

      // Column 5: live output color swatch.
      ImGui::TableSetColumnIndex(5);
      const glm::vec3 &c = fc.cube().color();
      ImGui::ColorButton("##swatch", ImVec4(c.r, c.g, c.b, 1.0f),
                         ImGuiColorEditFlags_NoTooltip, ImVec2(18, 18));

      ImGui::PopID();
    }
    ImGui::EndTable();
  }
  ImGui::End();
}
