#pragma once

#include "driver.hpp"
#include "node/expression.hpp"
#include "node/statement.hpp"
#include <optional>
namespace intrp {

class expression_visitor {
public:
  virtual void visit_binop(const binop_expression &) = 0;
  virtual void visit_unarop(const unarop_expression &) = 0;
  virtual void visit_literal(const literal_expression &) = 0;
  virtual void visit_identifier(const identifier_expression &) = 0;
  virtual ~expression_visitor() = default;
};

class statement_visitor {
public:
  virtual void visit_block(const block_statement &) = 0;
  virtual void visit_print(const print_statement &) = 0;
  virtual void visit_assign(const assign_statement &) = 0;
  virtual void visit_if(const if_statement &) = 0;
  virtual void visit_while(const while_statement &) = 0;
  virtual ~statement_visitor() = default;
};

class statement_executor : public statement_visitor {
private:
  std::shared_ptr<var_table> table;

public:
  statement_executor(std::shared_ptr<var_table> table);
  void visit_block(const block_statement &) override;
  void visit_print(const print_statement &) override;
  void visit_assign(const assign_statement &) override;
  void visit_if(const if_statement &) override;
  void visit_while(const while_statement &) override;
};

class expression_executor : public expression_visitor {
private:
  std::shared_ptr<var_table> table;
  std::optional<expr_t> result;

public:
  expression_executor(std::shared_ptr<var_table> table);

  /* Set as optional to validate that expression visit
  set it. Kind of analogous to assert, but doesn't require one.
  Just use .value() */

  std::optional<expr_t> get_result() const;
  void visit_binop(const binop_expression &) override;
  void visit_unarop(const unarop_expression &) override;
  void visit_literal(const literal_expression &) override;
  void visit_identifier(const identifier_expression &) override;
};

} // namespace intrp