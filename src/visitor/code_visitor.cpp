#include "visitor/code_visitor.hpp"
#include "exception.hpp"
#include "location.hh"
#include "node/statement.hpp"
#include "type/type.hpp"
#include "visitor/instruction_writer.hpp"
#include "visitor/visitor.hpp"
#include <cstdint>
#include <list>
#include <memory>

namespace intrp {

code_visitor::code_visitor(std::ostream &out) : out{out}, writer{out} {}

void code_visitor::declare_print_func() {
  writer.label("PRINT");
  auto r = alloc.alloc();
  writer.get_arg(r, 1);
  writer.ewrite(r);
  alloc.dealloc(r);
  writer.ret(alloc);
}

void code_visitor::visit_program(const program &progr) {
  out << "#Enter program\n";
  writer.label("_START");
  auto r = alloc.alloc();
  writer.li_label(r, "MAIN");
  writer.call_start(alloc, 0);
  writer.call_end(r);
  alloc.dealloc(r);
  writer.ebreak();

  auto print_addr = writer.get_next_addr();

  declare_print_func();
  std::vector<unique_ptr<intrp::type>> print_sign;
  print_sign.push_back(std::make_unique<int_type>());
  print_sign.push_back(std::make_unique<void_type>());
  auto info = sym_info{
      "print", std::make_unique<intrp::function_type>(std::move(print_sign)),
      sym_info::ABS, print_addr};
  table.add(std::move(info));
  out << "#Iterating through functions\n";
  for (const auto &func : progr.get_funcs()) {
    func->accept(*this);
  }
}

void code_visitor::visit_function(const function &f) {
  out << "#Enter function " << f.get_identifier() << "\n";
  auto signature = std::vector<unique_ptr<type>>();
  for (const auto &p : f.get_params()) {
    signature.push_back(p.get_type()->clone());
  }
  signature.push_back(f.get_type()->clone());
  auto info =
      sym_info{f.get_identifier(),
               std::make_unique<intrp::function_type>(std::move(signature)),
               sym_info::ABS, writer.get_next_addr()};

  table.add(std::move(info));
  writer.label(f.get_identifier()); // TODO: MANGLE
  f.get_block()->accept(*this);
}

void code_visitor::visit_block(const block_statement &b) {
  table.start_block();

  for (const auto &st : b.get_statements()) {
    st->accept(*this);
  }
  table.end_block();
}

void code_visitor::visit_binop(const binop_expression &) {};
void code_visitor::visit_unarop(const unarop_expression &) {};
void code_visitor::visit_literal(const literal_expression &) {};
void code_visitor::visit_identifier(const identifier_expression &) {};
void code_visitor::visit_function_call(const function_call &) {};

void code_visitor::visit_return(const return_statement &) {};
void code_visitor::visit_assign(const assign_statement &) {};
void code_visitor::visit_if(const if_statement &) {};
void code_visitor::visit_while(const while_statement &) {};


} // namespace intrp
