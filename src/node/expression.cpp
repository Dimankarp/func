#include "node/expression.hpp"

#include "visitor/visitor.hpp"
#include <utility>
namespace intrp {
using std::unique_ptr;

yy::location expression::get_loc() const { return loc; }
expression::expression(yy::location loc) : loc(loc) {};

// Binop
binop_expression::binop_expression(binop op, unique_ptr<expression> left,
                                   unique_ptr<expression> right,
                                   yy::location loc)
    : op(op), left(std::move(left)), right(std::move(right)), expression(loc) {}

void binop_expression::accept(expression_visitor &visitor) {
  visitor.visit_binop(*this);
}
binop binop_expression::get_op() const { return op; }
const unique_ptr<expression> &binop_expression::get_left() const {
  return left;
}
const unique_ptr<expression> &binop_expression::get_right() const {
  return right;
}
// Unarop
unarop_expression::unarop_expression(unarop op, unique_ptr<expression> exp,
                                     yy::location loc)
    : op(op), exp(std::move(exp)), expression(loc) {};
void unarop_expression::accept(expression_visitor &visitor) {
  visitor.visit_unarop(*this);
};
unarop unarop_expression::get_op() const { return op; }
const unique_ptr<expression> &unarop_expression::get_exp() const { return exp; }

// Literal
literal_expression::literal_expression(lit_val val, yy::location loc)
    : val(std::move(val)), expression(loc) {};
void literal_expression::accept(expression_visitor &visitor) {
  visitor.visit_literal(*this);
};
lit_val literal_expression::get_val() const { return val; }

// Identifier
identifier_expression::identifier_expression(std::string identificator,
                                             yy::location loc)
    : identificator(std::move(identificator)), expression(loc) {}
void identifier_expression::accept(expression_visitor &visitor) {
  visitor.visit_identifier(*this);
};

const std::string &identifier_expression::get_identificator() const {
  return identificator;
}

} // namespace intrp