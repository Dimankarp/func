#include "visitor/visitor.hpp"
#include "node/expression.hpp"
#include "node/statement.hpp"
#include "visitor/operation.hpp"
#include "visitor/print_visitor.hpp"
#include "function/function.hpp"

namespace intrp {


namespace{
  template<class T>
  std::ostream& operator<< (print_visitor& v, T& t){
    v.get_out() << v.tabs() << t;
    return v.get_out();
  } 

  std::unordered_map<intrp::types, std::string> type_name =
    {
        {intrp::types::INT, "int"},
        {intrp::types::STRING, "string"},
        {intrp::types::BOOL, "bool"},
        {intrp::types::VOID, "void"},
        {intrp::types::FUNCTION, "..func.."}
    };
}

void print_visitor::visit_program(const program & progr){
  *this << "program:" << "\n";
  offset++;
  for (auto& func : progr.get_funcs()){
    func->accept(*this);
  }
  offset--;
}

void print_visitor::visit_function(const function & func){

  *this << type_name[func.get_type()->get_type()] 
        << " "
        << func.get_identifier()
        << " "
  ;

  out << "(\n";
  offset++;
  for (auto& param : func.get_params()){
    *this << type_name[param.get_type()->get_type()] 
          << " "
          << param.get_identifier()
          << ","
    ;
  }
  out << "\n" << ")" << "\n"
  ;

  func.get_block()->accept(*this);
  offset--;
}

void print_visitor::visit_block(const block_statement & block){
  *this << "{" << "\n";
  offset++;
  for (auto& statement : block.get_statements()){
    statement->accept(*this);
  }
  offset--;
  *this << "}" << "\n";
}

void print_visitor::visit_assign(const assign_statement & statem) {
  if (statem.get_type() != null ){
    *this << type_name[statem.get_type()->get_type()] << " ";
  }

  *this << statem.get_identifier() << "=" << "\n";

  offset++;
  if (statem.get_exp() != null ){
    statem.get_exp()->accept(*this);
  }
  offset--;
};

void print_visitor::visit_return(const return_statement &) {};
void print_visitor::visit_if(const if_statement &) {};
void print_visitor::visit_while(const while_statement &) {};
void print_visitor::visit_function_call(const function_call &) {};

void print_visitor::visit_binop(const binop_expression &) {};
void print_visitor::visit_unarop(const unarop_expression &) {};

void print_visitor::visit_literal(const literal_expression & lit) {
  if (auto* v = std::get_if<int>(lit.get_val())) {
    *this << v;
  } else if auto* v = std::get_if<bool>(lit.get_val())) {
    *this << v;
  } else if (auto* v = std::get_if<std::string>(lit.get_val())) {
    *this << v;
  } else {
    *this << "UNKNOWN LITYERAL";
  }
};

void print_visitor::visit_identifier(const identifier_expression &) {};


} // namespace intrp
