#include "type.hpp"
#include <memory>
#include <string>
#include <unordered_map>
namespace intrp {

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
    type &func_cur_type = *func.signature[i];
    if (!signature[i]->equals(func_cur_type))
      return false;
  }
  return true;
}
namespace {

std::unordered_map<intrp::types, std::string> type_name_for_expect = {
    {intrp::types::INT, "int"},
    {intrp::types::STRING, "string"},
    {intrp::types::BOOL, "bool"},
    {intrp::types::VOID, "void"},
    {intrp::types::FUNCTION, "..func.."}};
};

std::string types_to_string(const intrp::types t) {
  return intrp::type_name_for_expect[t];
}

// NOLINTNEXTLINE(misc-no-recursion)
std::string type_to_string(const intrp::type &t) {
  if (t.get_type() != types::FUNCTION)
    return intrp::types_to_string(t.get_type());
  const auto &fun = dynamic_cast<const function_type &>(t);
  std::string res = "(" + intrp::type_to_string(*fun.get_signature().front());
  auto iter = fun.get_signature().begin();
  iter++;
  while (iter != fun.get_signature().end()) {
    res.append("-" + intrp::type_to_string(**iter));
  }
  res.append(")");
  return res;
}

} // namespace intrp
