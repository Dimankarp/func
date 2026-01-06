#include "type.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
namespace func {

const std::vector<std::unique_ptr<type>> &function_type::get_signature() const {
  return signature;
}
function_type::function_type(std::vector<std::unique_ptr<type>> &&sign)
    : signature{std::move(sign)} {}

function_type::function_type(const function_type &t) {
  signature.reserve(t.signature.size());
  for (const auto &a : t.signature) {
    signature.push_back(a ? a->clone() : nullptr);
  }
}

bool function_type::equals(const type &f) const {
  if (f.get_type() != types::FUNCTION)
    return false;
  const auto &func = dynamic_cast<const function_type &>(f);
  if (signature.size() != func.signature.size())
    return false;
  for (int i = 0; i < signature.size(); i++) {
    type const &func_cur_type = *func.signature[i];
    if (!signature[i]->equals(func_cur_type))
      return false;
  }
  return true;
}
namespace {

std::unordered_map<func::types, std::string> type_name_for_expect = {
    {func::types::INT, "int"},
    {func::types::STRING, "string"},
    {func::types::BOOL, "bool"},
    {func::types::VOID, "void"},
    {func::types::FUNCTION, "..func.."}};
};

std::string types_to_string(const func::types t) {
  return func::type_name_for_expect[t];
}

// NOLINTNEXTLINE(misc-no-recursion)
std::string type_to_string(const func::type &t) {
  if (t.get_type() != types::FUNCTION)
    return func::types_to_string(t.get_type());
  const auto &fun = dynamic_cast<const function_type &>(t);
  std::string res = "(" + func::type_to_string(*fun.get_signature().front());
  auto iter = fun.get_signature().begin();
  iter++;
  while (iter != fun.get_signature().end()) {
    res.append("-" + func::type_to_string(**iter));
  }
  res.append(")");
  return res;
}

} // namespace func
