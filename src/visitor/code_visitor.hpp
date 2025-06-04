#pragma once

#include "exception.hpp"
#include "location.hh"
#include "type/type.hpp"
#include "visitor/instruction_writer.hpp"
#include "visitor/visitor.hpp"
#include <array>
#include <cstdint>
#include <list>
#include <memory>

namespace intrp {

struct expr_result {
  std::unique_ptr<intrp::type> type_obj;
  uint8_t reg_num;
};

struct sym_info {
  std::string name;
  std::unique_ptr<intrp::type> type_obj;
  enum : char { STACK, ABS } access_type;
  uint16_t offset;
  yy::location declare_loc = yy::location{};
  bool is_delimeter = false;
};

class sym_table {
  std::list<sym_info> table;

public:
  void start_block() {
    sym_info delim{};
    delim.is_delimeter = true;
    table.push_back(delim);
  }

  void end_block() {
    while (!table.back().is_delimeter) {
      table.pop_back();
    }
    table.pop_back();
  }

  void add(sym_info &&sym) {

    auto iter = table.rbegin();
    while (!iter->is_delimeter) {
      if (iter->name == sym.name)
        throw symbol_redeclaratione_exception{sym.name, iter->declare_loc,
                                              sym.declare_loc};
      iter++;
    }
    table.push_back(std::move(sym));
  }
};

class reg_allocator {

  /*
  x31 - stack head points to curr value
  x30 - stack bottom
  x29 - return register
  ...
  x0 - always 0
  */
  const static uint8_t GENERAL_USE_REGISTER_NUM = 32 - 3;

  std::array<bool, GENERAL_USE_REGISTER_NUM> regs;

public:
  uint8_t alloc() {
    for (int i = 1; i < regs.size(); i++) {
      if (!regs[i]) {
        regs[i] = true;
        return i;
      }
    }
    throw not_enough_registers_exceptions{};
  }

  void dealloc(uint8_t reg) { regs[reg] = false; }
};

class code_visitor : public expression_visitor, public statement_visitor {
private:
  std::reference_wrapper<std::ostream> out;
  expr_result result;
  sym_table table;
  reg_allocator alloc;
  intrp::instr::instruction_writer writer;

public:
  code_visitor(std::ostream &out);

  void visit_binop(const binop_expression &) override;
  void visit_unarop(const unarop_expression &) override;
  void visit_literal(const literal_expression &) override;
  void visit_identifier(const identifier_expression &) override;
  void visit_function_call(const function_call &) override;

  void visit_program(const program &) override;
  void visit_block(const block_statement &) override;
  void visit_return(const return_statement &) override;
  void visit_assign(const assign_statement &) override;
  void visit_if(const if_statement &) override;
  void visit_while(const while_statement &) override;
  void visit_function(const function &) override;

private:
  void declare_print_func();
};

} // namespace intrp
