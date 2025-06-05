#include "visitor/code_visitor.hpp"
#include "exception.hpp"
#include "location.hh"
#include "node/expression.hpp"
#include "node/statement.hpp"
#include "type/type.hpp"
#include "visitor/instruction_writer.hpp"
#include "visitor/operation.hpp"
#include "visitor/register_allocator.hpp"
#include "visitor/visitor.hpp"
#include <cstdint>
#include <list>
#include <memory>

namespace intrp {

code_visitor::code_visitor(std::ostream &out) : out{out}, writer{out} {}

void code_visitor::declare_print_func() {
  writer.label("PRINT");
  auto r = alloc.alloc("Get PRINT arg from stack");
  writer.get_arg(r, 1);
  writer.ewrite(r);
  alloc.dealloc(r);
  writer.ret(alloc);
}

void code_visitor::visit_program(const program &progr) {
  out << "#Enter program\n";
  writer.label("_START");
  writer.li(instr::SP, (1 << 16));
  writer.li(instr::BP, (1 << 16));
  auto r = alloc.alloc("Store addres for main");
  writer.li_label(r, "main");
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
  out << "#Done program\n";
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

  this->stack_height = f.get_params().size();

  f.get_block()->accept(*this);
  out << "#Done function " << f.get_identifier() << "\n";
}

void code_visitor::visit_block(const block_statement &b) {
  out << "#Enter block " << "\n";
  table.start_block();

  for (const auto &st : b.get_statements()) {
    st->accept(*this);
  }
  table.end_block();
  out << "#Done block " << "\n";
}
namespace {
std::vector<uint8_t> push_regs_before_call(instr::instruction_writer &w,
                                           reg_allocator &alloc) {
  auto regs = alloc.get_allocated_regs();
  for (auto r : regs) {
    w.push(r);
  }
  return regs;
}

void pop_regs_after_call(instr::instruction_writer &w,
                         std::vector<uint8_t> &regs) {
  auto iter = regs.rbegin();
  while (iter != regs.rend()) {
    w.pop(*iter);
    iter++;
  }
}

void load_variable(uint16_t d, sym_info& sym){
  switch (sym.access_type) {
    case sym_info::STACK:
      writer.get_arg(d, sym.offset);
      break;
    case sym_info::ABS:
      writer.li(d, sym.offset);
      break;
  }
}

void store_variable(uint16_t s, sym_info& sym){
  switch (sym.access_type) {
    case sym_info::STACK:
      writer.put_arg(s, sym.offset);
      break;
    case sym_info::ABS:
      auto r = alloc.alloc("Address of variable to store");
      writer.li(r, sym.offset);
      writer.sw(r, 0, s);
      alloc.dealloc(r);
      break;
  }
}

} // namespace
void code_visitor::visit_function_call(const function_call &fc) {
  out << "#Enter function call" << "\n";
  out << "#Pushing regs" << "\n";
  auto regs = push_regs_before_call(writer, alloc);

  fc.get_func()->accept(*this);
  expr_result func = std::move(this->result);

  auto r = alloc.alloc("Get function result from RR");

  // TODO: Typecheck

  std::vector<uint8_t> arg_regs{};
  for (const auto &e : fc.get_arg_list()) {
    e->accept(*this);
    arg_regs.push_back(this->result.reg_num);
  }
  writer.call_start(alloc, arg_regs.size());
  for (auto &arg : arg_regs) {
    writer.push(arg);
    alloc.dealloc(arg);
  }
  writer.call_end(func.reg_num);
  alloc.dealloc(func.reg_num);

  this->result.type_obj =
      std::move((dynamic_cast<function_type *>(func.type_obj.get()))
                    ->get_signature()
                    .back()
                    ->clone());
  writer.mov(r, instr::RR);
  this->result.reg_num = r;

  out << "#Recovering regs" << "\n";
  pop_regs_after_call(writer, regs);
  out << "#Done function call" << "\n";
};

void code_visitor::visit_identifier(const identifier_expression &id) {
  out << "#Enter identifier " << id.get_identificator() << "\n";
  auto sym = table.find(id.get_identificator());
  auto r = alloc.alloc("Identifier return register");
  load_variable(r, sym);
  this->result.reg_num = r;
  this->result.type_obj = std::move(sym.type_obj->clone());

  out << "#Done identifier " << id.get_identificator() << "\n";
};

void code_visitor::visit_literal(const literal_expression &lit) {
  out << "#Enter literal " << "\n";
  intrp::lit_val val = lit.get_val();
  auto r = alloc.alloc("Literal return register");
  if (auto *v = std::get_if<int>(&val)) {
    writer.li(r, *v);
    result.type_obj = std::make_unique<int_type>();
  } else if (auto *v = std::get_if<bool>(&val)) {
    writer.li(r, *v ? 1 : 0);
    result.type_obj = std::make_unique<bool_type>();
  } else if (auto *v = std::get_if<std::string>(&val)) {
    // TODO:
  }

  result.reg_num = r;
  out << "#Done literal " << "\n";
};

void code_visitor::visit_return(const return_statement &ret) {
  if (ret.get_exp() != nullptr) {
    ret.get_exp()->accept(*this);
    writer.mov(instr::RR, result.reg_num);
    alloc.dealloc(result.reg_num);
  }
  writer.ret(alloc);
};

void code_visitor::visit_binop(const binop_expression &bop) {
  bop.get_left()->accept(*this);
  expr_result left = std::move(this->result);
  bop.get_right()->accept(*this);
  expr_result right = std::move(this->result);
  try {
    switch (bop.get_op()) {
    case binop::ADD:
      this->result = expr_add(writer, alloc, left, right);
      break;
    case binop::SUB:
      this->result = expr_sub(writer, alloc, left, right);
      break;
    case binop::MUL:
      this->result = expr_mul(writer, alloc, left, right);
      break;
    case binop::DIV:
      this->result = expr_div(writer, alloc, left, right);
      break;
    case binop::MOD:
      this->result = expr_rem(writer, alloc, left, right);
      break;
    case binop::LESS:
      this->result = expr_less(writer, alloc, left, right);
      break;
    case binop::GRTR:
      this->result = expr_grtr(writer, alloc, left, right);
      break;
    case binop::EQ:
      this->result = expr_eq(writer, alloc, left, right);
      break;
    case binop::NEQ:
      this->result = expr_neq(writer, alloc, left, right);
      break;
    case binop::OR:
      this->result = expr_or(writer, alloc, left, right);
      break;
    case binop::AND:
      this->result = expr_add(writer, alloc, left, right);
      break;
    }
  } catch (unexpected_type_exception &e) {
    e.loc = bop.get_loc();
    throw e;
  }
  alloc.dealloc(left.reg_num);
  alloc.dealloc(right.reg_num);
};
void code_visitor::visit_unarop(const unarop_expression &) {};

void code_visitor::visit_assign(const assign_statement & stm) {
  struct intrp::sym_info& sym;

  if (stm.get_type() != nullptr){
    // push on stack
    writer.push(0);
    stack_height++;

    // add to sym_table
    sym =
      sym_info{stm.get_identifier(),
               stm.get_type()->clone(),
               sym_info::STACK, stack_height};
    table.add(std::move(sym));
  } else{
    sym = table.find(id.get_identificator());
  }

  // count expression and save result
  if (stm.get_exp() != nullptr ){
    stm.get_exp()->accept(*this);

    // TODO: Typecheck

    store_variable(result.reg_num, sym);
    alloc.dealloc(result.reg_num);
  }
};

void code_visitor::visit_if(const if_statement &) {};
void code_visitor::visit_while(const while_statement &) {};

} // namespace intrp
