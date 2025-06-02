#pragma once

#include "driver.hpp"
#include "function/function.hpp"
#include "node/expression.hpp"
#include "node/program.hpp"
#include "node/statement.hpp"
namespace intrp {

class expression_visitor {
public:
  virtual void visit_binop(const binop_expression &) = 0;
  virtual void visit_unarop(const unarop_expression &) = 0;
  virtual void visit_literal(const literal_expression &) = 0;
  virtual void visit_identifier(const identifier_expression &) = 0;
  virtual void visit_function_call(const function_call &) = 0;
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
  virtual ~statement_visitor() = default;
};

// class expression_executor : public expression_visitor {
// private:
//   std::shared_ptr<var_table> table;
//   std::optional<lit_val> result;

// public:
//   expression_executor(std::shared_ptr<var_table> table);

//   /* Set as optional to validate that expression visit
//   set it. Kind of analogous to assert, but doesn't require one.
//   Just use .value() */

//   std::optional<lit_val> get_result() const;
//   void visit_binop(const binop_expression &) override;
//   void visit_unarop(const unarop_expression &) override;
//   void visit_literal(const literal_expression &) override;
//   void visit_identifier(const identifier_expression &) override;
// };

} // namespace intrp