#include "visitor/operation.hpp"
#include "node/expression.hpp"
#include <iostream>
#include <variant>

namespace intrp {

lit_val expr_add(const lit_val &a, const lit_val &b) {
  if (std::holds_alternative<std::string>(a)) {
    const auto &astr = std::get<std::string>(a);
    const std::string &bstr = std::holds_alternative<std::string>(b)
                                  ? std::get<std::string>(b)
                                  : std::to_string(std::get<int>(b));

    return lit_val{astr + bstr};
  }
  const int aval = std::get<int>(a);
  const int bval = expect<int>(b);
  return aval + bval;
}

lit_val expr_sub(const lit_val &a, const lit_val &b) {
  return expect<int>(a) - expect<int>(b);
}
lit_val expr_mul(const lit_val &a, const lit_val &b) {
  return expect<int>(a) * expect<int>(b);
}
lit_val expr_div(const lit_val &a, const lit_val &b) {
  return static_cast<int>(expect<int>(a) / expect<int>(b));
}
lit_val expr_mod(const lit_val &a, const lit_val &b) {
  return expect<int>(a) % expect<int>(b);
}
lit_val expr_less(const lit_val &a, const lit_val &b) {
  return expect<int>(a) < expect<int>(b);
}
lit_val expr_grtr(const lit_val &a, const lit_val &b) {
  return expect<int>(a) > expect<int>(b);
}
lit_val expr_leq(const lit_val &a, const lit_val &b) {
  return expect<int>(a) <= expect<int>(b);
}
lit_val expr_greq(const lit_val &a, const lit_val &b) {
  return expect<int>(a) >= expect<int>(b);
}
lit_val expr_eq(const lit_val &a, const lit_val &b) {
  return expect<int>(a) == expect<int>(b);
}
lit_val expr_neq(const lit_val &a, const lit_val &b) {
  return expect<int>(a) != expect<int>(b);
}

lit_val expr_negate(const lit_val &a) { return -expect<int>(a); }

bool lit_valo_bool(const lit_val &a) {
  if (std::holds_alternative<std::string>(a)) {
    return !std::get<std::string>(a).empty();
  }
  return std::get<int>(a) != 0;
}

std::ostream &operator<<(std::ostream &output, const lit_val &expr) {
  if (std::holds_alternative<std::string>(expr)) {
    output << (std::get<std::string>(expr));
  } else {
    output << (std::get<int>(expr));
  }
  return output;
}

} // namespace intrp