#include "function/function.hpp"
#include "location.hh"
#include "node/statement.hpp"
#include "visitor/visitor.hpp"

namespace intrp {

function_call::function_call(
    std::string &id, std::vector<unique_ptr<intrp::expression>> &&arg_list,
    yy::location loc)
    : identifier{id}, arg_list{std::move(arg_list)}, statement{loc},
      expression{loc} {}
void function_call::accept(statement_visitor &visitor) {
  visitor.visit_function_call(*this);
}

void function_call::accept(expression_visitor &visitor) {
  visitor.visit_function_call(*this);
}

parameter::parameter(unique_ptr<type> type_obj, std::string &identifier)
    : type_obj{std::move(type_obj)}, identifier{identifier} {};

function::function(unique_ptr<type> type_obj, std::string &identifier,
                   std::vector<parameter> &&param_list,
                   unique_ptr<block_statement> block, yy::location loc)
    : type_obj{std::move(type_obj)}, identifier{identifier},
      param_list{std::move(param_list)}, block{std::move(block)},
      statement{loc} {}
void function::accept(statement_visitor &visitor) { visitor.visit_function(*this); };
} // namespace intrp
