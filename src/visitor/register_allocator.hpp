#pragma once
#include "exception.hpp"
#include <array>
#include <cstdint>
namespace intrp {

class reg_allocator {

  /*
  x31 - stack head points to curr value
  x30 - stack bottom
  x29 - return register
  ...
  x0 - always 0
  */
  const static uint8_t GENERAL_USE_REGISTER_NUM = 32 - 3;

  std::array<bool, GENERAL_USE_REGISTER_NUM> regs;

public:
  uint8_t alloc() {
    for (int i = 1; i < regs.size(); i++) {
      if (!regs[i]) {
        regs[i] = true;
        return i;
      }
    }
    throw not_enough_registers_exceptions{};
  }

  void dealloc(uint8_t reg) { regs[reg] = false; }
};

} // namespace intrp