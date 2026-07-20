#pragma once

#include "Cube.h"

#include <cstdint>
#include <vector>

// Couples a Cube with the fixture id(s) it represents and the universe it
// lives on.
class FixtureCube {
public:
  FixtureCube(std::vector<uint16_t> fids, uint16_t universe)
      : m_fids(std::move(fids)), m_universe(universe) {}

  Cube &cube() { return m_cube; }
  const Cube &cube() const { return m_cube; }

  const std::vector<uint16_t> &fids() const { return m_fids; }
  uint16_t universe() const { return m_universe; }

private:
  Cube m_cube;
  std::vector<uint16_t> m_fids;
  uint16_t m_universe = 0;
};
