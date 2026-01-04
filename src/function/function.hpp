#pragma once

#include "location.hh"
#include "node/ast.hpp"
#include "node/expression.hpp"
#include "node/statement.hpp"
#include <string>

namespace intrp {

class function_call : public ast_node_impl<function_call> {
  using Base = ast_node_impl<function_call>;
  std::unique_ptr<ast_node> func;
  std::vector<unique_ptr<ast_node>> arg_list;

public:
  function_call(std::unique_ptr<ast_node> func,
                std::vector<unique_ptr<ast_node>> &&arg_list, yy::location loc)
      : func{std::move(func)}, arg_list{std::move(arg_list)}, Base{loc} {}

  const std::unique_ptr<ast_node> &get_func() const { return func; };
  const std::vector<unique_ptr<ast_node>> &get_arg_list() const {
    return arg_list;
  };
};

class parameter {
  unique_ptr<type> type_obj;
  std::string identifier;

public:
  parameter() = default;
  parameter(unique_ptr<type> type_obj, std::string &identifier)
      : type_obj{std::move(type_obj)}, identifier{identifier} {}
  const unique_ptr<type> &get_type() const { return type_obj; }
  const std::string &get_identifier() const { return identifier; }
};

class function : public ast_node_impl<function> {
  using Base = ast_node_impl<function>;
  unique_ptr<type> type_obj;
  std::string identifier;
  std::vector<parameter> param_list;
  unique_ptr<block_statement> block;

public:
  function(unique_ptr<type> type_obj, std::string &identifier,
           std::vector<parameter> &&param_list,
           unique_ptr<block_statement> block, yy::location loc)
      : type_obj{std::move(type_obj)}, identifier{identifier},
        param_list{std::move(param_list)}, block{std::move(block)}, Base{loc} {}
  const unique_ptr<type> &get_type() const { return type_obj; }
  const std::string &get_identifier() const { return identifier; }
  const std::vector<parameter> &get_params() const { return param_list; }
  const unique_ptr<block_statement> &get_block() const { return block; }
};
} // namespace intrp
