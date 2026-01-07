#pragma once
#include "exception.hpp"
#include "printer.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

namespace func {

class reg_allocator {

  /*
  x31 - stack head points to curr value
  x30 - stack bottom
  x29 - return register
  ...
  x0 - always 0
  */
  const static uint8_t GENERAL_USE_REGISTER_NUM = 32 - 3;
  std::array<bool, GENERAL_USE_REGISTER_NUM> regs{};

private:
  func::stream_proxy &alloc_out;

public:
  reg_allocator(func::stream_proxy &out) : alloc_out{out} {}

  uint8_t alloc() {
    for (int i = 1; i < regs.size(); i++) {
      if (!regs[i]) {
        alloc_out << "# ALLOC: " << std::to_string(i) << "\n";
        regs[i] = true;
        return i;
      }
    }
    throw not_enough_registers_exception{};
  }

  uint8_t alloc(const std::string &reason) {
    for (int i = 1; i < regs.size(); i++) {
      if (!regs[i]) {
        alloc_out << "# ALLOC: " << std::to_string(i) << " " << reason << "\n";
        regs[i] = true;
        return i;
      }
    }
    throw not_enough_registers_exception{};
  }

  std::vector<uint8_t> get_allocated_regs() {
    std::vector<uint8_t> allocated;
    for (int i = 1; i < regs.size(); i++) {
      if (regs[i])
        allocated.push_back(i);
    }
    return allocated;
  }

  void dealloc(uint8_t reg) {
    alloc_out << "# RELEASE: " << std::to_string(reg) << "\n";
    assert(regs[reg] == true);
    regs[reg] = false;
  }
};

} // namespace func