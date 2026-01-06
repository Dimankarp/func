#include "visitor/print_visitor.hpp"
#include "function/function.hpp"
#include "node/expression.hpp"
#include "node/statement.hpp"

#include <iostream>

namespace func {

namespace {
template <class T> std::ostream &operator<<(print_visitor &v, T &t) {
  v.get_out() << v.tabs() << t;
  return v.get_out();
}

std::unordered_map<func::types, std::string> type_name = {
    {func::types::INT, "int"},
    {func::types::STRING, "string"},
    {func::types::BOOL, "bool"},
    {func::types::VOID, "void"},
    {func::types::FUNCTION, "..func.."}};

std::unordered_map<func::binop, std::string> binop_name = {
    {func::binop::ADD, "+"},  {func::binop::SUB, "-"},
    {func::binop::MUL, "*"},  {func::binop::DIV, "/"},
    {func::binop::MOD, "%"},  {func::binop::LESS, "<"},
    {func::binop::GRTR, ">"}, {func::binop::EQ, "=="},
    {func::binop::NEQ, "!="}, {func::binop::OR, "||"},
    {func::binop::AND, "&&"}};

std::unordered_map<func::unarop, std::string> unarop_name = {
    {func::unarop::MINUS, "-"}, {func::unarop::NOT, "!"}};
} // namespace

void print_visitor::print_type(func::type &type) {
  if (type.get_type() != func::types::FUNCTION)
    out << type_name[type.get_type()];
  else {
    function_type &func = dynamic_cast<function_type &>(type);
    out << "<func>";
  }
}

void print_visitor::visit_program(const program &progr) {
  *this << "program:" << "\n";
  offset++;
  for (auto &func : progr.get_funcs()) {
    func->accept(*this);
  }
  offset--;
}

void print_visitor::visit_function(const function &func) {

  *this << type_name[func.get_type()->get_type()] << " "
        << func.get_identifier() << " ";

  out << "\n";
  offset++;
  for (auto &param : func.get_params()) {
    *this << type_name[param.get_type()->get_type()] << " "
          << param.get_identifier() << "\n";
  }

  func.get_block()->accept(*this);
  offset--;
}

void print_visitor::visit_block(const block_statement &block) {
  *this << "{" << "\n";
  offset++;
  for (auto &statement : block.get_statements()) {
    statement->accept(*this);
  }
  offset--;
  *this << "}" << "\n";
}

void print_visitor::visit_assign(const assign_statement &statem) {
  if (statem.get_type() != nullptr)
    *this << type_name[statem.get_type()->get_type()] << " ";
  else
    *this << ""; // to keep offset

  out << statem.get_identifier() << " =" << "\n";

  offset++;
  if (statem.get_exp() != nullptr) {
    statem.get_exp()->accept(*this);
  }
  offset--;
};

void print_visitor::visit_return(const return_statement &ret) {
  *this << "return" << "\n";
  offset++;
  if (ret.get_exp() != nullptr)
    ret.get_exp()->accept(*this);
  offset--;
};

void print_visitor::visit_if(const if_statement &statem) {
  *this << "if" << "\n";
  offset++;
  statem.get_condition()->accept(*this);
  offset--;

  statem.get_then_block()->accept(*this);

  if (statem.get_else_block() != nullptr) {
    *this << "else" << "\n";
    statem.get_else_block()->accept(*this);
  }
};

void print_visitor::visit_while(const while_statement &statem) {
  *this << "while" << "\n";
  offset++;
  statem.get_condition()->accept(*this);
  offset--;

  statem.get_block()->accept(*this);
};

void print_visitor::visit_function_call(const function_call &func) {
  func.get_func()->accept(*this);

  offset++;
  for (auto &arg : func.get_arg_list()) {
    arg->accept(*this);
  }
  offset--;

  if (func.get_arg_list().size() == 0)
    *this << "\n";
};

void print_visitor::visit_binop(const binop_expression &op) {
  *this << binop_name[op.get_op()] << "\n";
  offset++;
  op.get_left()->accept(*this);
  op.get_right()->accept(*this);
  offset--;
};

void print_visitor::visit_unarop(const unarop_expression &op) {
  *this << unarop_name[op.get_op()] << "\n";
  offset++;
  op.get_exp()->accept(*this);
  offset--;
};

void print_visitor::visit_literal(const literal_expression &lit) {
  func::lit_val val = lit.get_val();
  if (auto *v = std::get_if<int>(&val)) {
    *this << *v;
  } else if (auto *v = std::get_if<bool>(&val)) {
    *this << std::boolalpha << *v;
  } else if (auto *v = std::get_if<std::string>(&val)) {
    *this << *v;
  } else {
    *this << "UNKNOWN LITYERAL";
  }
  out << "\n";
};

void print_visitor::visit_identifier(const identifier_expression &id) {
  *this << id.get_identificator() << "\n";
};

void print_visitor::visit_subscript(const subscript_expression &sub) {
  *this << ".[..]" << "\n";
  offset++;
  sub.get_pointer()->accept(*this);
  sub.get_index()->accept(*this);
  offset--;
};

void print_visitor::visit_subscript_assign(
    const subscript_assign_statement &sub) {
  *this << ".[..]" << "\n";
  offset+=2;
  sub.get_pointer()->accept(*this);
  sub.get_index()->accept(*this);
  offset--;
  *this << "=" << "\n";
  offset++;
  sub.get_exp()->accept(*this);
  offset-=2;
};
} // namespace func
