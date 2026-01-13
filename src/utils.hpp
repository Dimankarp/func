#pragma once

#include <cstdlib>
#include <utility>

namespace func {

template <typename... Args>
[[noreturn]] void panic(int exit_code, const char* format, Args&&... args) {
    printf(format, std::forward<Args>(args)...);
    exit(exit_code);
}
} // namespace func
