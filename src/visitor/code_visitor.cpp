#include "visitor/code_visitor.hpp"
#include "exception.hpp"
#include "location.hh"
#include "node/expression.hpp"
#include "node/statement.hpp"
#include "type/type.hpp"
#include "visitor/instruction_writer.hpp"
#include "visitor/operation.hpp"
#include "visitor/register_allocator.hpp"
#include <cstdint>
#include <memory>

namespace intrp {

code_visitor::code_visitor(std::ostream &out) : out{out}, writer{out} {}

void code_visitor::declare_write_func() {
  auto func_addr = writer.get_next_addr();

  writer.write_func(alloc);

  std::vector<unique_ptr<intrp::type>> write_sign;
  write_sign.push_back(std::make_unique<int_type>());
  write_sign.push_back(std::make_unique<void_type>());
  auto info = sym_info{
      "write", std::make_unique<intrp::function_type>(std::move(write_sign)),
      sym_info::ABS, func_addr};
  table.add(std::move(info));
}

void code_visitor::declare_read_func() {
  auto func_addr = writer.get_next_addr();

  writer.read_func(alloc);

  std::vector<unique_ptr<intrp::type>> read_sign;
  read_sign.push_back(std::make_unique<void_type>());
  read_sign.push_back(std::make_unique<int_type>());
  auto info = sym_info{
      "read", std::make_unique<intrp::function_type>(std::move(read_sign)),
      sym_info::ABS, func_addr};
  table.add(std::move(info));
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

  declare_write_func();
  declare_read_func();

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

  // Staring block for func param
  table.start_block();

  for (int i = 0; i < f.get_params().size(); i++) {
    auto &param = f.get_params()[i];

    // Registering parameters
    table.add(sym_info{param.get_identifier(), param.get_type()->clone(),
                       sym_info::STACK, static_cast<uint16_t>(i + 1)});
  }

  writer.label(f.get_identifier()); // TODO: MANGLE
  this->stack_height = f.get_params().size();
  f.get_block()->accept(*this);

  // Ending block for func param
  table.end_block();
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

void load_variable(instr::instruction_writer &writer, uint8_t d,
                   sym_info &sym) {
  switch (sym.access_type) {
  case sym_info::STACK:
    writer.get_arg(d, sym.offset);
    break;
  case sym_info::ABS:
    writer.li(d, sym.offset);
    break;
  }
}

void store_variable(instr::instruction_writer &writer, reg_allocator &alloc,
                    uint8_t s, const sym_info &sym) {
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

  fc.get_func()->accept(*this);
  expr_result func = std::move(this->result);

  // TODO: Typecheck

  std::vector<uint8_t> arg_regs{};
  for (const auto &e : fc.get_arg_list()) {
    e->accept(*this);
    arg_regs.push_back(this->result.reg_num);
  }

  out << "#Pushing regs" << "\n";
  auto regs = push_regs_before_call(writer, alloc);
  stack_height += regs.size();

  writer.call_start(alloc, arg_regs.size());
  for (auto &arg : arg_regs) {
    writer.push(arg);
    alloc.dealloc(arg);
  }
  writer.call_end(func.reg_num);
  alloc.dealloc(func.reg_num);

  out << "#Recovering regs" << "\n";
  pop_regs_after_call(writer, regs);
  stack_height -= regs.size();

  this->result.type_obj =
      std::move((dynamic_cast<function_type *>(func.type_obj.get()))
                    ->get_signature()
                    .back()
                    ->clone());

  auto r = alloc.alloc("Get function result from RR");
  writer.mov(r, instr::RR);
  this->result.reg_num = r;

  out << "#Done function call" << "\n";
};

void code_visitor::visit_identifier(const identifier_expression &id) {
  out << "#Enter identifier " << id.get_identificator() << "\n";
  auto sym = table.find(id.get_identificator());
  auto r = alloc.alloc("Identifier return register");
  load_variable(writer, r, sym);
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
    writer.push_str(alloc, *v);
    this->stack_height += (v->length()) + 1;
    writer.mov(r, instr::SP);
    result.type_obj = std::make_unique<string_type>();
  }

  result.reg_num = r;
  out << "#Done literal " << "\n";
};

void code_visitor::visit_return(const return_statement &ret) {
  out << "#Enter return" << "\n";
  if (ret.get_exp() != nullptr) {
    ret.get_exp()->accept(*this);
    writer.mov(instr::RR, result.reg_num);
    alloc.dealloc(result.reg_num);
  }
  writer.ret(alloc);
  out << "#Done return" << "\n";
};

void code_visitor::visit_binop(const binop_expression &bop) {
  out << "#Enter binop" << "\n";
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
      this->result = expr_and(writer, alloc, left, right);
      break;
    }
  } catch (unexpected_type_exception &e) {
    e.loc = bop.get_loc();
    throw e;
  }
  alloc.dealloc(left.reg_num);
  alloc.dealloc(right.reg_num);
  out << "#Done binop" << "\n";
};

void code_visitor::visit_assign(const assign_statement &stm) {
  out << "#Enter assing" << "\n";
  /*
  3 Variants:
  int a;
  int a = 42;
  a = 42;
  */

  if (stm.get_type() != nullptr) {
    // push on stack
    writer.push(0);
    stack_height++;

    // add to sym_table
    sym_info sym = sym_info{stm.get_identifier(), stm.get_type()->clone(),
                            sym_info::STACK, stack_height};
    table.add(std::move(sym));
    out << "#Done assign" << "\n";
  }

  const sym_info &sym = table.find(stm.get_identifier());

  // count expression and save result
  if (stm.get_exp() != nullptr) {
    stm.get_exp()->accept(*this);

    // TODO: Typecheck

    store_variable(writer, alloc, result.reg_num, sym);
    alloc.dealloc(result.reg_num);
  }
};

void code_visitor::visit_unarop(const unarop_expression &unop) {
  unop.get_exp()->accept(*this);
  expr_result res = std::move(this->result);
  try {
    switch (unop.get_op()) {

    case unarop::MINUS:
      this->result = expr_minus(writer, alloc, res);
      break;
    case unarop::NOT:
      this->result = expr_not(writer, alloc, res);
      break;
    }
  } catch (unexpected_type_exception &e) {
    e.loc = unop.get_loc();
    throw e;
  }
  alloc.dealloc(res.reg_num);
};

void code_visitor::visit_if(const if_statement &stm) {
  out << "#Enter if" << "\n";
  std::string then_end_label = "THEN_END_" + std::to_string(label_ind++);
  std::string else_end_label = "ELSE_END_" + std::to_string(label_ind++);

  stm.get_condition()->accept(*this);
  // TODO: Typecheck result is bool
  writer.beq(result.reg_num, 0, then_end_label);
  alloc.dealloc(result.reg_num);

  stm.get_then_block()->accept(*this);

  if (stm.get_else_block() != nullptr) {
    writer.jal_label(0, else_end_label);
  }

  writer.label(then_end_label);

  if (stm.get_else_block() != nullptr) {
    stm.get_else_block()->accept(*this);
    writer.label(else_end_label);
  }
  out << "#Done if" << "\n";
};

void code_visitor::visit_while(const while_statement &stm) {
  out << "#Enter while" << "\n";
  std::string while_start_label = "WHILE_START_" + std::to_string(label_ind++);
  std::string while_end_label = "WHILE_END_" + std::to_string(label_ind++);

  writer.label(while_start_label);

  stm.get_condition()->accept(*this);
  // TODO: Typecheck result is bool
  writer.beq(result.reg_num, 0, while_end_label);
  alloc.dealloc(result.reg_num);

  stm.get_block()->accept(*this);
  writer.jal_label(0, while_start_label);

  writer.label(while_end_label);
  out << "#Done while" << "\n";
};

void code_visitor::visit_subscript(const subscript_expression &sub) {
  out << "#Enter subscript" << "\n";
  sub.get_pointer()->accept(*this);
  expr_result ptr = std::move(this->result);

  if (ptr.type_obj->get_type() != types::STRING)
    throw unexpected_type_exception{
        {"expected string as subscript target", sub.get_pointer()->get_loc()}};

  sub.get_index()->accept(*this);
  expr_result idx = std::move(this->result);

  if (idx.type_obj->get_type() != types::INT)
    throw unexpected_type_exception{
        {"expected int as subscript index", sub.get_index()->get_loc()}};

  auto r = alloc.alloc();

  writer.add(ptr.reg_num, ptr.reg_num, idx.reg_num);
  writer.lw(r, ptr.reg_num, 0);

  alloc.dealloc(ptr.reg_num);
  alloc.dealloc(idx.reg_num);
  result.reg_num = r;
  result.type_obj = std::make_unique<int_type>();
  out << "#Done subscript" << "\n";
};

void code_visitor::visit_subscript_assign(
    const subscript_assign_statement &sub) {
  out << "#Enter subscript assign" << "\n";
  sub.get_pointer()->accept(*this);
  expr_result ptr = std::move(this->result);

  if (ptr.type_obj->get_type() != types::STRING)
    throw unexpected_type_exception{
        {"expected string as subscript target", sub.get_pointer()->get_loc()}};

  sub.get_index()->accept(*this);
  expr_result idx = std::move(this->result);

  if (idx.type_obj->get_type() != types::INT)
    throw unexpected_type_exception{
        {"expected int as subscript index", sub.get_index()->get_loc()}};

  writer.add(ptr.reg_num, ptr.reg_num, idx.reg_num);
  alloc.dealloc(idx.reg_num);

  sub.get_exp()->accept(*this);
  expr_result exp = std::move(this->result);

  if (exp.type_obj->get_type() != types::INT)
    throw unexpected_type_exception{
        {"expected int as assignee to string subscript",
         sub.get_exp()->get_loc()}};

  writer.sw(ptr.reg_num, 0, exp.reg_num);

  alloc.dealloc(ptr.reg_num);
  alloc.dealloc(exp.reg_num);
  out << "#Done subscript assign" << "\n";
};

} // namespace intrp
