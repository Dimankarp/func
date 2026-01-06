#pragma once

#include "driver.hpp"
#include "function/function.hpp"
#include "node/expression.hpp"
#include "node/program.hpp"
#include "node/statement.hpp"
#include "visitor/visitor.hpp"
namespace func {

class print_visitor : public expression_visitor, public statement_visitor {
private:
  std::ostream &out;
  int offset = 0;

  void print_type(func::type &);

public:
  print_visitor(std::ostream &ostream) : out{ostream} {}

  std::string tabs() { return std::string(offset * 3, ' '); };

  std::ostream &get_out() { return out; };

  void visit_binop(const binop_expression &) override;
  void visit_unarop(const unarop_expression &) override;
  void visit_literal(const literal_expression &) override;
  void visit_identifier(const identifier_expression &) override;
  void visit_function_call(const function_call &) override;
  void visit_subscript(const subscript_expression &) override;

  void visit_subscript_assign(const subscript_assign_statement &) override;
  void visit_program(const program &) override;
  void visit_block(const block_statement &) override;
  void visit_return(const return_statement &) override;
  void visit_assign(const assign_statement &) override;
  void visit_if(const if_statement &) override;
  void visit_while(const while_statement &) override;
  void visit_function(const function &) override;
};

} // namespace func
