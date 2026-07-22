#include "Application.h"
#include "UiHelpers.h"

void Application::renderColorPresetWindow() {
  int selCount = int(m_engine.programmer().selection().size());
  presetPoolUI(
      "Color Presets", selCount, m_engine.stored().colorPresets(),
      [&] {
        m_engine.storeColorPreset(m_engine.stored().colorPresets().nextFree());
      },
      [&](uint32_t num) {
        m_engine.recallColorPreset(num);
        m_engine.update();
      });
}

void Application::renderDimmerPresetWindow() {
  int selCount = int(m_engine.programmer().selection().size());
  presetPoolUI(
      "Dimmer Presets", selCount, m_engine.stored().dimmerPresets(),
      [&] {
        m_engine.storeDimmerPreset(
            m_engine.stored().dimmerPresets().nextFree());
      },
      [&](uint32_t num) {
        m_engine.recallDimmerPreset(num);
        m_engine.update();
      });
}
