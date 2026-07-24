#include "Application.h"
#include "UiHelpers.h"

#include <imgui.h>

#include "LightEngine/Fixture/Fixture.h"

#include <array>
#include <cstdio>
#include <string>

// Universe viewer: a 512-channel grid per universe. Channels owned by a patched
// fixture are highlighted (and hoverable for details); unpatched channels are
// dimmed. Values read live from each universe's DMX buffer.
void Application::renderUniverseView() {
  const auto &universes = m_engine.patcher().universes();
  if (universes.empty())
    ImGui::TextDisabled("No universes patched.");

  for (const auto &[uid, uni] : universes) {
    char header[64];
    std::snprintf(header, sizeof(header), "Universe %u  (%zu fixtures)###uni%u",
                  unsigned(uid), uni.fixtureCount(), unsigned(uid));

    if (!ImGui::CollapsingHeader(header, ImGuiTreeNodeFlags_DefaultOpen))
      continue;

    // Map every channel to its owning fixture (nullptr = unpatched).
    std::array<const LightEngine::Fixtures::Fixture *, 512> owner{};
    for (const auto &fx : uni.fixtures()) {
      const uint32_t last = fx->start + fx->Footprint();
      for (uint32_t c = fx->start; c < last && c < 512; ++c)
        owner[c] = fx.get();
    }

    const std::array<uint8_t, 512> &buf = uni.buffer();

    const ImGuiTableFlags flags =
        ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit;

    ImGui::PushID(int(uid));
    if (ImGui::BeginTable("channels", 16, flags)) {
      for (int ch = 0; ch < 512; ++ch) {
        ImGui::TableNextColumn();
        const LightEngine::Fixtures::Fixture *fx = owner[ch];
        const ImVec4 col = fx ? ImVec4(0.35f, 0.80f, 0.45f, 1.0f)  // patched
                              : ImVec4(0.35f, 0.35f, 0.35f, 1.0f); // free
        ImGui::TextColored(col, "%3u", unsigned(buf[ch]));

        if (fx && ImGui::IsItemHovered()) {
          // Channel offset within this fixture -> attribute name, if any.
          const char *attr = "-";
          const uint32_t offset = uint32_t(ch) - fx->start;
          uint32_t idx = 0;
          for (const auto &p : fx->Parameters()) {
            if (idx == offset) {
              attr = attributeName(p.Attribute());
              break;
            }
            ++idx;
          }
          ImGui::SetTooltip("Ch %d (1-based)\n%s\nFID %u  %s\nValue %u", ch + 1,
                            attr, unsigned(fx->Fid()), fx->Name().c_str(),
                            unsigned(buf[ch]));
        }
      }
      ImGui::EndTable();
    }
    ImGui::PopID();
    ImGui::Spacing();
  }
}
