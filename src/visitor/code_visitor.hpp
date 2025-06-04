#pragma once

#include "exception.hpp"
#include "location.hh"
#include "type/type.hpp"
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
  enum : char { STACK, LABEL } access_type;
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

public:
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
};

namespace instr {

const uint8_t SP = 31;
const uint8_t BP = 30;
const uint8_t RR = 29; // Return register

inline std::string reg(uint8_t n) { return "x" + std::to_string(n); }

inline void push(std::ostream &s, uint8_t src) {
  s << "addi x31, x31, -1\n";
  s << "sw x31, 0, " << reg(src) << "\n";
}

inline void pop(std::ostream &s, uint8_t src) {
  s << "lw " << reg(src) << ", " << reg(SP) << ", 0\n";
  s << "addi x31, x31, 1\n";
}

inline void call(std::ostream &s, reg_allocator &alloc, std::string &label) {
  auto r = alloc.alloc();
  // Push pc
  s << "jal " << reg(r) << ", 0" << "\n";
  s << "addi " << reg(r) << ", " << reg(r) << ", 7\n"; // Potential BUG!!!!
  instr::push(s, r);

  // Push bp (x30)
  instr::push(s, BP);

  // BP <- SP
  s << "addi " << reg(BP) << ", " << reg(SP) << ", 0\n";

  // Jump
  s << "jal  " << "x0, " << label << "\n";

  alloc.dealloc(r);
}

inline void ret(std::ostream &s, reg_allocator &alloc) {
  // SP <- BP
  s << "addi " << reg(SP) << ", " << reg(BP) << ", 0\n";

  // Pop BP
  pop(s, BP);

  // Pop PC
  auto r = alloc.alloc();
  pop(s, r);
  s << "jalr " << "x0, " << reg(r) << ", " << "0\n";

  alloc.dealloc(r);
}

} // namespace instr

} // namespace intrp
