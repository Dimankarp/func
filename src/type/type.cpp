#include "type.hpp"
#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>
namespace intrp {
types int_type::get_type() const { return types::INT; }
types int_type::static_get_type() { return types::INT; }
std::unique_ptr<type> int_type::clone() const {
  return std::make_unique<int_type>();
};

types string_type::get_type() const { return types::STRING; }
types string_type::static_get_type() { return types::STRING; }
std::unique_ptr<type> string_type::clone() const {
  return std::make_unique<string_type>();
};

types bool_type::get_type() const { return types::BOOL; }
types bool_type::static_get_type() { return types::BOOL; }
std::unique_ptr<type> bool_type::clone() const {
  return std::make_unique<bool_type>();
};

types void_type::get_type() const { return types::VOID; }
types void_type::static_get_type() { return types::VOID; }
std::unique_ptr<type> void_type::clone() const {
  return std::make_unique<void_type>();
};

types function_type::get_type() const { return types::FUNCTION; }
types function_type::static_get_type() { return types::FUNCTION; }
const std::vector<std::unique_ptr<type>> &function_type::get_signature() const {
  return signature;
}

function_type::function_type(std::vector<std::unique_ptr<type>> &&sign)
    : signature{std::move(sign)} {}

std::unique_ptr<type> function_type::clone() const {
  std::vector<std::unique_ptr<type>> output;
  std::transform(this->signature.begin(), this->signature.end(),
                 std::back_inserter(output),
                 [](const std::unique_ptr<type> &a) { return a->clone(); });
  return std::make_unique<function_type>(std::move(output));
};

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
