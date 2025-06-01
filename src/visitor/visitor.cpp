#include "visitor/visitor.hpp"
#include "node/expression.hpp"
#include "visitor/operation.hpp"
#include "node/statement.hpp"
#include <memory>
#include <utility>

namespace intrp {

statement_executor::statement_executor(std::shared_ptr<var_table> table)
    : table(std::move(table)) {};

void statement_executor::visit_assign(const assign_statement &s) {
  expression_executor exp_exec{table};
  s.get_exp()->accept(exp_exec);
  (*table)[s.get_identifier()] = exp_exec.get_result().value();
}

void statement_executor::visit_block(const block_statement &block) {
  for (const auto &s : block.get_statements()) {
    s->accept(*this);
  }
}

void statement_executor::visit_if(const if_statement &s) {
  expression_executor exp_exec{table};
  s.get_condition()->accept(exp_exec);
  if (expr_to_bool(exp_exec.get_result().value())) {
    s.get_then_block()->accept(*this);
  } else if (s.get_else_block().has_value()) {
    s.get_else_block().value()->accept(*this);
  }
}

void statement_executor::visit_print(const print_statement &s) {
  expression_executor exp_exec{table};
  s.get_exp()->accept(exp_exec);
  intrp::operator<<(std::cout, exp_exec.get_result().value());
}

void statement_executor::visit_while(const while_statement &s) {
  expression_executor exp_exec{table};
  while ((s.get_condition()->accept(exp_exec),
          expr_to_bool(exp_exec.get_result().value()))) {
    s.get_block()->accept(*this);
  }
}

expression_executor::expression_executor(std::shared_ptr<var_table> table)
    : table(std::move(table)) {};

std::optional<expr_t> expression_executor::get_result() const { return result; }

void expression_executor::visit_binop(const binop_expression &e) {
  e.get_left()->accept(*this);
  const expr_t a = result.value();
  e.get_right()->accept(*this);
  const expr_t b = result.value();
  try {
    switch (e.get_op()) {
    case binop::ADD:
      result = expr_add(a, b);
      break;
    case binop::SUB:
      result = expr_sub(a, b);
      break;
    case binop::MUL:
      result = expr_mul(a, b);
      break;
    case binop::DIV:
      result = expr_div(a, b);
      break;
    case binop::MOD:
      result = expr_mod(a, b);
      break;
    case binop::LESS:
      result = expr_less(a, b);
      break;
    case binop::GRTR:
      result = expr_grtr(a, b);
      break;
    case binop::LEQ:
      result = expr_leq(a, b);
      break;
    case binop::GREQ:
      result = expr_greq(a, b);
      break;
    case binop::EQ:
      result = expr_eq(a, b);
      break;
    case binop::NEQ:
      result = expr_neq(a, b);
      break;
    }
  } catch (unexpected_type_exception &excp) {
    excp.loc = e.get_loc();
    throw excp;
  }
}

void expression_executor::visit_unarop(const unarop_expression &e) {
  try {
    switch (e.get_op()) {
    case unarop::MINUS:
      e.get_exp()->accept(*this);
      result = expr_negate(result.value());
      break;
    }
  } catch (unexpected_type_exception &excp) {
    excp.loc = e.get_loc();
    throw excp;
  }
}
void expression_executor::visit_literal(const literal_expression &e) {
  result = e.get_val();
}
void expression_executor::visit_identifier(const identifier_expression &e) {
  auto var = (*table).find(e.get_identificator());
  if (var == (*table).end()) {
    auto loc = e.get_loc();
    throw undeclared_variable_exception{
        {"referencing undeclared variable " + e.get_identificator(),
         e.get_loc()}};
  }
  result = var->second;
};

} // namespace intrp