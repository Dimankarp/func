#pragma once

#include "location.hh"
#include "node/expression.hpp"
#include "node/statement.hpp"
#include <string>

namespace func {

class function_call : public expression, public statement {
  std::unique_ptr<func::expression> func;
  std::vector<unique_ptr<func::expression>> arg_list;

public:
  function_call(std::unique_ptr<func::expression> func,
                std::vector<unique_ptr<func::expression>> &&arg_list,
                yy::location loc);
  void accept(statement_visitor &visitor) override;
  void accept(expression_visitor &visitor) override;
  const std::unique_ptr<func::expression> &get_func() const { return func; };
  const std::vector<unique_ptr<func::expression>> &get_arg_list() const {
    return arg_list;
  };
};

class parameter {
  unique_ptr<type> type_obj;
  std::string identifier;

public:
  parameter() = default;
  parameter(unique_ptr<type> type_obj, std::string &identifier);
  const unique_ptr<type> &get_type() const { return type_obj; }
  const std::string &get_identifier() const { return identifier; }
};

class function : public statement {
  unique_ptr<type> type_obj;
  std::string identifier;
  std::vector<parameter> param_list;
  unique_ptr<block_statement> block;

public:
  function(unique_ptr<type> type_obj, std::string &identifier,
           std::vector<parameter> &&param_list,
           unique_ptr<block_statement> block, yy::location loc);
  void accept(statement_visitor &visitor) override;
  const unique_ptr<type> &get_type() const { return type_obj; }
  const std::string &get_identifier() const { return identifier; }
  const std::vector<parameter> &get_params() const { return param_list; }
  const unique_ptr<block_statement> &get_block() const { return block; }
};
} // namespace func
