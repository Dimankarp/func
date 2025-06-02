#include "visitor/visitor.hpp"
#include "node/expression.hpp"
#include "node/statement.hpp"
#include "visitor/operation.hpp"
#include "visitor/print_visitor.hpp"
#include "function/function.hpp"

namespace intrp {

void print_visitor::visit_program(const program & progr){
  *this <<= "program:" << "\n";
  for (auto& func : progr.get_funcs()){
    func.get()->accept(*this);
  }
}

void print_visitor::visit_function(const function & func){

  *this <<= type_name[func.get_type()->get_type()] 
        << " "
        << func.get_identifier()
  ;`

  out << "( ";
  for (auto& param : func.get_params()){
    out << type_name[param.get_type()->get_type()] 
        << " "
        << param.get_identifier()
        << ","
    ;
  }
  out << ")";

  offset++;
  func.get_block()->accept(*this);
  offset--;
}

namespace{
  template<class T>
  std::ostream& operator<<= (print_visitor& v, T& t){
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

void print_visitor::visit_block(const block_statement & block){

}


void print_visitor::visit_return(const return_statement &) {};
void print_visitor::visit_assign(const assign_statement &) {};
void print_visitor::visit_if(const if_statement &) {};
void print_visitor::visit_while(const while_statement &) {};
void print_visitor::visit_function_call(const function_call &) {};

void print_visitor::visit_binop(const binop_expression &) {};
void print_visitor::visit_unarop(const unarop_expression &) {};
void print_visitor::visit_literal(const literal_expression &) {};
void print_visitor::visit_identifier(const identifier_expression &) {};
void print_visitor::visit_function_call(const function_call &) {};


} // namespace intrp