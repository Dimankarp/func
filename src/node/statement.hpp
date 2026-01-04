#pragma once
#include "location.hh"
#include "node/ast.hpp"
#include "node/expression.hpp"
#include "type/type.hpp"
#include <string>
#include <vector>
namespace func {

class block_statement : public ast_node_impl<block_statement> {
private:
  std::vector<unique_ptr<ast_node>> statements;

public:
  using ast_node_impl::ast_node_impl;
  void add_statement(unique_ptr<ast_node> s) {
    statements.push_back(std::move(s));
  }
  const std::vector<unique_ptr<ast_node>> &get_statements() const {
    return statements;
  }
};

class assign_statement : public ast_node_impl<assign_statement> {
  using Base = ast_node_impl<assign_statement>;

private:
  std::string identifier;
  unique_ptr<ast_node> exp;
  unique_ptr<type> type_obj;

public:
  assign_statement(unique_ptr<func::type> type, std::string &id,
                   yy::location loc)
      : identifier(id), Base(loc), type_obj{std::move(type)} {}

  assign_statement(unique_ptr<func::type> type, std::string &id,
                   unique_ptr<ast_node> exp, yy::location loc)
      : identifier(id), exp(std::move(exp)), Base(loc),
        type_obj{std::move(type)} {}

  assign_statement(std::string &id, unique_ptr<ast_node> exp, yy::location loc)
      : identifier(id), exp(std::move(exp)), Base(loc) {}

  const std::string &get_identifier() const { return identifier; }
  const unique_ptr<ast_node> &get_exp() const { return exp; }
  const unique_ptr<type> &get_type() const { return type_obj; }
};

class if_statement : public ast_node_impl<if_statement> {
  using Base = ast_node_impl<if_statement>;

private:
  unique_ptr<ast_node> condition;
  unique_ptr<block_statement> then_block;
  unique_ptr<block_statement> else_block;

public:
  if_statement(unique_ptr<ast_node> cond, unique_ptr<block_statement> then,
               yy::location loc)
      : condition(std::move(cond)), then_block(std::move(then)), Base(loc) {}

  void add_else(unique_ptr<block_statement> else_block) {
    this->else_block = std::move(else_block);
  }

  const unique_ptr<ast_node> &get_condition() const { return condition; }

  const unique_ptr<block_statement> &get_then_block() const {
    return then_block;
  }
  const unique_ptr<block_statement> &get_else_block() const {
    return else_block;
  }
};

class while_statement : public ast_node_impl<while_statement> {
  using Base = ast_node_impl<while_statement>;

private:
  unique_ptr<ast_node> condition;
  unique_ptr<block_statement> block;

public:
  while_statement(unique_ptr<ast_node> cond, unique_ptr<block_statement> block,
                  yy::location loc)
      : condition(std::move(cond)), block(std::move(block)), Base(loc) {}
  const unique_ptr<ast_node> &get_condition() const { return condition; }
  const unique_ptr<block_statement> &get_block() const { return block; }
};

class return_statement : public ast_node_impl<return_statement> {
  using Base = ast_node_impl<return_statement>;

private:
  unique_ptr<ast_node> exp;

public:
  return_statement(unique_ptr<ast_node> exp, yy::location loc)
      : exp{std::move(exp)}, Base{loc} {}
  const unique_ptr<ast_node> &get_exp() const { return exp; }
};

class subscript_assign_statement
    : public ast_node_impl<subscript_assign_statement> {
  using Base = ast_node_impl<subscript_assign_statement>;

private:
  unique_ptr<ast_node> pointer;
  unique_ptr<ast_node> index;
  unique_ptr<ast_node> exp;

public:
  subscript_assign_statement(unique_ptr<ast_node> pointer,
                             unique_ptr<ast_node> index,
                             unique_ptr<ast_node> exp, yy::location loc)
      : pointer{std::move(pointer)}, index{std::move(index)},
        exp{std::move(exp)}, Base{loc} {};
  const unique_ptr<ast_node> &get_pointer() const { return pointer; }
  const unique_ptr<ast_node> &get_index() const { return index; }
  const unique_ptr<ast_node> &get_exp() const { return exp; }
};

} // namespace func