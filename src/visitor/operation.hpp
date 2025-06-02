#pragma once

#include "exception.hpp"
#include "node/expression.hpp"
#include <variant>
namespace intrp {

template <typename T> T expect(const lit_val &a) {
  if (!std::holds_alternative<T>(a))
    throw unexpected_type_exception();
  return std::get<T>(a);
}

lit_val expr_add(const lit_val &a, const lit_val &b);
lit_val expr_sub(const lit_val &a, const lit_val &b);
lit_val expr_mul(const lit_val &a, const lit_val &b);
lit_val expr_div(const lit_val &a, const lit_val &b);
lit_val expr_mod(const lit_val &a, const lit_val &b);
lit_val expr_less(const lit_val &a, const lit_val &b);
lit_val expr_grtr(const lit_val &a, const lit_val &b);
lit_val expr_leq(const lit_val &a, const lit_val &b);
lit_val expr_greq(const lit_val &a, const lit_val &b);
lit_val expr_eq(const lit_val &a, const lit_val &b);
lit_val expr_neq(const lit_val &a, const lit_val &b);
lit_val expr_negate(const lit_val &a);
std::ostream &operator<<(std::ostream &output, const lit_val &expr);
bool lit_valo_bool(const lit_val &a);
} // namespace intrp