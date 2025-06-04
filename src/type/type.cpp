#include "type.hpp"
#include <algorithm>
#include <memory>

namespace intrp {
types int_type::get_type() { return types::INT; }
std::unique_ptr<type> int_type::clone() {
  return std::make_unique<int_type>();
};

types string_type::get_type() { return types::STRING; }
std::unique_ptr<type> string_type::clone() {
  return std::make_unique<string_type>();
};

types bool_type::get_type() { return types::BOOL; }
std::unique_ptr<type> bool_type::clone() {
  return std::make_unique<bool_type>();
};

types void_type::get_type() { return types::VOID; }
std::unique_ptr<type> void_type::clone() {
  return std::make_unique<void_type>();
};

types function_type::get_type() { return types::FUNCTION; }

function_type::function_type(std::vector<std::unique_ptr<type>> &&sign)
    : signature{std::move(sign)} {}

std::unique_ptr<type> function_type::clone() {
  std::vector<std::unique_ptr<type>> output;
  std::transform(this->signature.begin(), this->signature.end(),
                 std::back_inserter(output),
                 [](const std::unique_ptr<type> &a) { return a->clone(); });
  return std::make_unique<function_type>(std::move(output));
};

} // namespace intrp
