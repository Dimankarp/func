#include "visitor/operation.hpp"
#include "node/expression.hpp"
#include <iostream>
#include <variant>

namespace intrp {

expr_t expr_add(const expr_t &a, const expr_t &b) {
  if (std::holds_alternative<std::string>(a)) {
    const auto &astr = std::get<std::string>(a);
    const std::string &bstr = std::holds_alternative<std::string>(b)
                                  ? std::get<std::string>(b)
                                  : std::to_string(std::get<int>(b));

    return expr_t{astr + bstr};
  }
  const int aval = std::get<int>(a);
  const int bval = expect<int>(b);
  return aval + bval;
}

expr_t expr_sub(const expr_t &a, const expr_t &b) {
  return expect<int>(a) - expect<int>(b);
}
expr_t expr_mul(const expr_t &a, const expr_t &b) {
  return expect<int>(a) * expect<int>(b);
}
expr_t expr_div(const expr_t &a, const expr_t &b) {
  return static_cast<int>(expect<int>(a) / expect<int>(b));
}
expr_t expr_mod(const expr_t &a, const expr_t &b) {
  return expect<int>(a) % expect<int>(b);
}
expr_t expr_less(const expr_t &a, const expr_t &b) {
  return expect<int>(a) < expect<int>(b);
}
expr_t expr_grtr(const expr_t &a, const expr_t &b) {
  return expect<int>(a) > expect<int>(b);
}
expr_t expr_leq(const expr_t &a, const expr_t &b) {
  return expect<int>(a) <= expect<int>(b);
}
expr_t expr_greq(const expr_t &a, const expr_t &b) {
  return expect<int>(a) >= expect<int>(b);
}
expr_t expr_eq(const expr_t &a, const expr_t &b) {
  return expect<int>(a) == expect<int>(b);
}
expr_t expr_neq(const expr_t &a, const expr_t &b) {
  return expect<int>(a) != expect<int>(b);
}

expr_t expr_negate(const expr_t &a) { return -expect<int>(a); }

bool expr_to_bool(const expr_t &a) {
  if (std::holds_alternative<std::string>(a)) {
    return !std::get<std::string>(a).empty();
  }
  return std::get<int>(a) != 0;
}

std::ostream &operator<<(std::ostream &output, const expr_t &expr) {
  if (std::holds_alternative<std::string>(expr)) {
    output << (std::get<std::string>(expr));
  } else {
    output << (std::get<int>(expr));
  }
  return output;
}

} // namespace intrp