#include "Application.h"

#include <imgui.h>

#include <cstdio>
#include <string>

void Application::runCommand(const char *line) {
  if (!line || line[0] == '\0')
    return;

  m_cmdLog.push_back(std::string("> ") + line);
  if (!m_commandsLoaded) {
    m_cmdLog.emplace_back("  (commands not loaded)");
    return;
  }

  if (!m_engine.command(line)) {
    m_cmdLog.emplace_back("  parse error");
    return;
  }
  // Tick the engine so the command's edits compose and resolve immediately.
  m_engine.update();
  const auto &prog = m_engine.programmer();

  // Reflect the engine's programmer selection onto the cubes.
  syncCubesFromEngine();

  char msg[128];
  std::snprintf(msg, sizeof(msg), "  ok [selection=%zu edits=%zu]",
                prog.selection().size(), prog.edits().size());
  m_cmdLog.emplace_back(msg);
}

void Application::renderCommandWindow() {
  ImGui::SetNextWindowSize(ImVec2(420, 240), ImGuiCond_FirstUseEver);
  ImGui::Begin("Command");

  // Scrolling log.
  const float footer = ImGui::GetFrameHeightWithSpacing();
  ImGui::BeginChild("log", ImVec2(0, -footer), true,
                    ImGuiWindowFlags_HorizontalScrollbar);
  for (const auto &line : m_cmdLog)
    ImGui::TextUnformatted(line.c_str());
  if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
    ImGui::SetScrollHereY(1.0f);
  ImGui::EndChild();

  // Auto-focus: if the user starts typing anywhere and no other field is
  // active, redirect the keystrokes into the command line.
  if (ImGui::GetIO().InputQueueCharacters.Size > 0 && !ImGui::IsAnyItemActive())
    ImGui::SetKeyboardFocusHere();

  // Input line.
  ImGui::SetNextItemWidth(-1.0f);
  if (ImGui::InputText("##cmd", m_cmdInput, sizeof(m_cmdInput),
                       ImGuiInputTextFlags_EnterReturnsTrue)) {
    runCommand(m_cmdInput);
    m_cmdInput[0] = '\0';
    ImGui::SetKeyboardFocusHere(-1); // keep focus on the input
  }
  ImGui::End();
}
