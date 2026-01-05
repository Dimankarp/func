#pragma once

#include "driver.hpp"
#include "function/function.hpp"
#include "node/expression.hpp"
#include "node/program.hpp"
#include "node/statement.hpp"
namespace cmplr {

class expression_visitor {
public:
  virtual void visit_binop(const binop_expression &) = 0;
  virtual void visit_unarop(const unarop_expression &) = 0;
  virtual void visit_literal(const literal_expression &) = 0;
  virtual void visit_identifier(const identifier_expression &) = 0;
  virtual void visit_function_call(const function_call &) = 0;
  virtual void visit_subscript(const subscript_expression &) = 0;
  virtual ~expression_visitor() = default;
};

class statement_visitor {
public:
  virtual void visit_program(const program &) = 0;
  virtual void visit_block(const block_statement &) = 0;
  virtual void visit_return(const return_statement &) = 0;
  virtual void visit_assign(const assign_statement &) = 0;
  virtual void visit_if(const if_statement &) = 0;
  virtual void visit_while(const while_statement &) = 0;
  virtual void visit_function(const function &) = 0;
  virtual void visit_function_call(const function_call &) = 0;
  virtual void visit_subscript_assign(const subscript_assign_statement &) = 0;
  virtual ~statement_visitor() = default;
};

} // namespace cmplr