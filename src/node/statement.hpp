#pragma once
#include "location.hh"
#include "node/expression.hpp"
#include "type/type.hpp"
#include <memory>
#include <optional>
#include <string>
#include <vector>
namespace func {
using std::unique_ptr;
class statement_visitor;

class statement {
private:
  yy::location loc;

public:
  virtual ~statement() = default;
  virtual void accept(statement_visitor &) = 0;
  statement(yy::location loc);
  yy::location get_loc() const;
};

class block_statement : public statement {
private:
  std::vector<unique_ptr<statement>> statements;

public:
  block_statement(yy::location loc);
  void add_statement(unique_ptr<statement> s);
  void accept(statement_visitor &visitor) override;
  const std::vector<unique_ptr<statement>> &get_statements() const;
};

class assign_statement : public statement {
private:
  std::string identifier;
  unique_ptr<expression> exp;
  unique_ptr<type> type_obj;

public:
  assign_statement(unique_ptr<type> type, std::string &id, yy::location loc);
  assign_statement(unique_ptr<func::type> type, std::string &id,
                   unique_ptr<expression> exp, yy::location loc);
  assign_statement(std::string &id, unique_ptr<expression> exp,
                   yy::location loc);

  void accept(statement_visitor &visitor) override;
  const std::string &get_identifier() const;
  const unique_ptr<expression> &get_exp() const;
  const unique_ptr<type> &get_type() const;
};

class if_statement : public statement {
private:
  unique_ptr<expression> condition;
  unique_ptr<block_statement> then_block;
  unique_ptr<block_statement> else_block;

public:
  if_statement(unique_ptr<expression> cond, unique_ptr<block_statement> then,
               yy::location loc);
  void accept(statement_visitor &visitor) override;
  void add_else(unique_ptr<block_statement> else_block);
  const unique_ptr<expression> &get_condition() const;
  const unique_ptr<block_statement> &get_then_block() const;
  const unique_ptr<block_statement> &get_else_block() const;
};

class while_statement : public statement {
private:
  unique_ptr<expression> condition;
  unique_ptr<block_statement> block;

public:
  while_statement(unique_ptr<expression> cond,
                  unique_ptr<block_statement> block, yy::location loc);
  void accept(statement_visitor &visitor) override;
  const unique_ptr<expression> &get_condition() const;
  const unique_ptr<block_statement> &get_block() const;
};

class return_statement : public statement {
private:
  unique_ptr<expression> exp;

public:
  return_statement(unique_ptr<expression> exp, yy::location loc);
  void accept(statement_visitor &visitor) override;
  const unique_ptr<expression> &get_exp() const;
};

class subscript_assign_statement : public statement {
private:
  unique_ptr<expression> pointer;
  unique_ptr<expression> index;
  unique_ptr<expression> exp;

public:
  subscript_assign_statement(unique_ptr<expression> pointer,
                             unique_ptr<expression> index,
                             unique_ptr<expression> exp, yy::location loc);
  void accept(statement_visitor &visitor) override;
  const unique_ptr<expression> &get_pointer() const;
  const unique_ptr<expression> &get_index() const;
  const unique_ptr<expression> &get_exp() const;
};

} // namespace func