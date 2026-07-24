#pragma once

// Shared UI helpers used by the per-window render code in src/tools/.
// Header-only (inline / templates) so each translation unit can use them.

#include "LightEngine/GDTF/LogicalChannel.h"

#include <imgui.h>

#include <string>
#include <utility>

// --- Vector icons drawn onto a square toggle button (no icon font needed) ---

// A square toggle button that draws a custom icon; returns true when clicked.
// `draw(dl, center, r, col)` renders the icon centered in the button.
template <class DrawFn>
inline bool iconButton(const char *id, bool active, const char *tooltip,
                       DrawFn draw) {
  const float sz = ImGui::GetFrameHeight() * 1.4f;
  ImVec2 p = ImGui::GetCursorScreenPos();

  ImU32 bg =
      ImGui::GetColorU32(active ? ImGuiCol_ButtonActive : ImGuiCol_Button);
  bool clicked = ImGui::InvisibleButton(id, ImVec2(sz, sz));
  if (ImGui::IsItemHovered()) {
    bg = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
    if (tooltip)
      ImGui::SetTooltip("%s", tooltip);
  }

  ImDrawList *dl = ImGui::GetWindowDrawList();
  dl->AddRectFilled(p, ImVec2(p.x + sz, p.y + sz), bg, 4.0f);
  if (active)
    dl->AddRect(p, ImVec2(p.x + sz, p.y + sz),
                ImGui::GetColorU32(ImVec4(1.0f, 0.8f, 0.1f, 1.0f)), 4.0f, 0,
                2.0f);

  ImVec2 c(p.x + sz * 0.5f, p.y + sz * 0.5f);
  ImU32 col = ImGui::GetColorU32(ImGuiCol_Text);
  draw(dl, c, sz * 0.32f, col);
  return clicked;
}

// Mouse-pointer (select) icon.
inline void drawSelectIcon(ImDrawList *dl, ImVec2 c, float r, ImU32 col) {
  ImVec2 a(c.x - r * 0.7f, c.y - r);
  ImVec2 b(c.x - r * 0.7f, c.y + r * 0.9f);
  ImVec2 d(c.x - r * 0.1f, c.y + r * 0.25f);
  ImVec2 e(c.x + r * 0.9f, c.y + r * 0.1f);
  dl->AddTriangleFilled(a, b, d, col);
  dl->AddTriangleFilled(a, d, e, col);
}

// Four-way move (arrows) icon.
inline void drawMoveIcon(ImDrawList *dl, ImVec2 c, float r, ImU32 col) {
  dl->AddLine(ImVec2(c.x - r, c.y), ImVec2(c.x + r, c.y), col, 1.5f);
  dl->AddLine(ImVec2(c.x, c.y - r), ImVec2(c.x, c.y + r), col, 1.5f);
  const float h = r * 0.4f;
  // left / right / up / down arrowheads
  dl->AddTriangleFilled(ImVec2(c.x - r, c.y), ImVec2(c.x - r + h, c.y - h),
                        ImVec2(c.x - r + h, c.y + h), col);
  dl->AddTriangleFilled(ImVec2(c.x + r, c.y), ImVec2(c.x + r - h, c.y - h),
                        ImVec2(c.x + r - h, c.y + h), col);
  dl->AddTriangleFilled(ImVec2(c.x, c.y - r), ImVec2(c.x - h, c.y - r + h),
                        ImVec2(c.x + h, c.y - r + h), col);
  dl->AddTriangleFilled(ImVec2(c.x, c.y + r), ImVec2(c.x - h, c.y + r - h),
                        ImVec2(c.x + h, c.y + r - h), col);
}

// Clear (eraser / circle-slash) icon.
inline void drawClearIcon(ImDrawList *dl, ImVec2 c, float r, ImU32 col) {
  dl->AddCircle(c, r, col, 0, 1.8f);
  const float d = r * 0.707f;
  dl->AddLine(ImVec2(c.x - d, c.y - d), ImVec2(c.x + d, c.y + d), col, 1.8f);
}

// Patch (grid / plus) icon: a small cell grid with a plus, hinting at adding
// channels to a universe.
inline void drawPatchIcon(ImDrawList *dl, ImVec2 c, float r, ImU32 col) {
  dl->AddRect(ImVec2(c.x - r, c.y - r), ImVec2(c.x + r, c.y + r), col, 0.0f, 0,
              1.5f);
  dl->AddLine(ImVec2(c.x, c.y - r), ImVec2(c.x, c.y + r), col, 1.2f);
  dl->AddLine(ImVec2(c.x - r, c.y), ImVec2(c.x + r, c.y), col, 1.2f);
}

// A boxed, titled 3-axis attribute editor (Position, Rotation, ... — anything
// with x/y/z components). Each axis is edited as either a single value (all
// selected items share it) or a "from .. to" range that is spread linearly
// across the `count` items in order.
//
//   get(axis)         -> std::pair<float,float> {first item, last item} value
//   set(axis, i, val) -> write item i's value on that axis
//   rangeMode[3]        per-axis single/range toggle (caller-owned, persists)
//
// Returns true if any value changed this frame.
template <class GetFn, class SetFn>
inline bool axisSpreadEditor(const char *title, int count, bool rangeMode[3],
                             float speed, GetFn get, SetFn set) {
  bool changed = false;
  const char *axisName[3] = {"x", "y", "z"};
  const float fieldW = 70.0f;

  ImGui::PushID(title);
  ImGui::TextUnformatted(title);
  ImGui::BeginChild("box", ImVec2(0, 0),
                    ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);

  for (int a = 0; a < 3; ++a) {
    ImGui::PushID(a);
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(axisName[a]);
    ImGui::SameLine();

    std::pair<float, float> cur = get(a);
    if (!rangeMode[a]) {
      float v = cur.first;
      ImGui::SetNextItemWidth(fieldW);
      if (ImGui::DragFloat("##v", &v, speed)) {
        for (int i = 0; i < count; ++i)
          set(a, i, v);
        changed = true;
      }
    } else {
      float from = cur.first, to = cur.second;
      ImGui::SetNextItemWidth(fieldW);
      bool e = ImGui::DragFloat("##from", &from, speed);
      ImGui::SameLine();
      ImGui::TextUnformatted("..");
      ImGui::SameLine();
      ImGui::SetNextItemWidth(fieldW);
      e |= ImGui::DragFloat("##to", &to, speed);
      if (e) {
        for (int i = 0; i < count; ++i) {
          float t = count > 1 ? float(i) / float(count - 1) : 0.0f;
          set(a, i, from + (to - from) * t);
        }
        changed = true;
      }
    }
    ImGui::SameLine();
    // Toggle single <-> range spread for this axis.
    if (ImGui::SmallButton(rangeMode[a] ? "range" : "single"))
      rangeMode[a] = !rangeMode[a];
    ImGui::PopID();
  }

  ImGui::EndChild();
  ImGui::PopID();
  return changed;
}

// Shared UI for a numbered preset pool: a "Store" button (captures the current
// selection into the next free slot) and a list with per-preset "Recall".
template <class Pool, class StoreFn, class RecallFn>
inline void presetPoolUI(const char *title, int selCount, const Pool &pool,
                         StoreFn store, RecallFn recall) {
  ImGui::Begin(title);

  ImGui::BeginDisabled(selCount == 0);
  if (ImGui::Button("Store from selection"))
    store();
  ImGui::EndDisabled();
  ImGui::SameLine();
  ImGui::TextDisabled("(%d selected)", selCount);
  ImGui::Separator();

  if (pool.empty()) {
    ImGui::TextDisabled("No presets stored.");
  } else {
    for (const auto &[num, preset] : pool) {
      ImGui::PushID(int(num));
      if (ImGui::Button("Recall"))
        recall(num);
      ImGui::SameLine();
      const std::string &name = preset->name();
      ImGui::Text("#%u %s  (%zu fixtures)", unsigned(num),
                  name.empty() ? "(unnamed)" : name.c_str(), preset->size());
      ImGui::PopID();
    }
  }

  ImGui::End();
}

// Human-readable name for a GDTF attribute.
inline const char *attributeName(LightEngine::GDTF::Attribute a) {
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
