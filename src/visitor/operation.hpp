#pragma once

#include "exception.hpp"
#include "node/expression.hpp"
#include <variant>
namespace intrp {

template <typename T> T expect(const expr_t &a) {
  if (!std::holds_alternative<T>(a))
    throw unexpected_type_exception();
  return std::get<T>(a);
}

expr_t expr_add(const expr_t &a, const expr_t &b);
expr_t expr_sub(const expr_t &a, const expr_t &b);
expr_t expr_mul(const expr_t &a, const expr_t &b);
expr_t expr_div(const expr_t &a, const expr_t &b);
expr_t expr_mod(const expr_t &a, const expr_t &b);
expr_t expr_less(const expr_t &a, const expr_t &b);
expr_t expr_grtr(const expr_t &a, const expr_t &b);
expr_t expr_leq(const expr_t &a, const expr_t &b);
expr_t expr_greq(const expr_t &a, const expr_t &b);
expr_t expr_eq(const expr_t &a, const expr_t &b);
expr_t expr_neq(const expr_t &a, const expr_t &b);
expr_t expr_negate(const expr_t &a);
std::ostream &operator<<(std::ostream &output, const expr_t &expr);
bool expr_to_bool(const expr_t &a);
} // namespace intrp