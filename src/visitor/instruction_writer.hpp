#pragma once

#include "exception.hpp"
#include "location.hh"
#include "type/type.hpp"
#include "visitor/register_allocator.hpp"
#include "visitor/visitor.hpp"
#include "printer.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <list>
#include <memory>
#include <ostream>
#include <string>

namespace func::instr {

const uint8_t SP = 31;
const uint8_t BP = 30;
const uint8_t RR = 29; // Return register

class instruction_writer {
  uint16_t next_addr = 0;
  func::stream_proxy &out;

public:
  instruction_writer(func::stream_proxy &out) : out{out} {}
  uint16_t get_next_addr() const { return next_addr; }

  void lui(uint8_t d, int32_t imm) {
    out << "lui " << reg(d) << ", " << imm << "\n";
    next_addr++;
  }

  void addi(uint8_t d, uint8_t s, int32_t imm) {
    out << "addi " << reg(d) << ", " << reg(s) << ", " << imm << "\n";
    next_addr++;
  }

  void li(uint8_t d, int32_t imm) {
    out << "li " << reg(d) << ", " << imm << "\n";
    next_addr += imm >= (1 << 20) ? 2 : 1;
  }

  void li_label(uint8_t d, std::string label) {
    out << "li " << reg(d) << ", " << label << "\n";
    next_addr += 2;
  }

  void xori(uint8_t d, uint8_t s, int32_t imm) {
    out << "xori " << reg(d) << ", " << reg(s) << ", " << imm << "\n";
    next_addr++;
  }

  void add(uint8_t d, uint8_t s1, uint8_t s2) {
    out << "add " << reg(d) << ", " << reg(s1) << ", " << reg(s2) << "\n";
    next_addr++;
  }

  void sub(uint8_t d, uint8_t s1, uint8_t s2) {
    out << "sub " << reg(d) << ", " << reg(s1) << ", " << reg(s2) << "\n";
    next_addr++;
  }

  void xor_op(uint8_t d, uint8_t s1, uint8_t s2) {
    out << "xor " << reg(d) << ", " << reg(s1) << ", " << reg(s2) << "\n";
    next_addr++;
  }

  void srl(uint8_t d, uint8_t s1, uint8_t s2) {
    out << "srl " << reg(d) << ", " << reg(s1) << ", " << reg(s2) << "\n";
    next_addr++;
  }

  void sra(uint8_t d, uint8_t s1, uint8_t s2) {
    out << "sra " << reg(d) << ", " << reg(s1) << ", " << reg(s2) << "\n";
    next_addr++;
  }

  void or_op(uint8_t d, uint8_t s1, uint8_t s2) {
    out << "or " << reg(d) << ", " << reg(s1) << ", " << reg(s2) << "\n";
    next_addr++;
  }

  void and_op(uint8_t d, uint8_t s1, uint8_t s2) {
    out << "and " << reg(d) << ", " << reg(s1) << ", " << reg(s2) << "\n";
    next_addr++;
  }

  void mul(uint8_t d, uint8_t s1, uint8_t s2) {
    out << "mul " << reg(d) << ", " << reg(s1) << ", " << reg(s2) << "\n";
    next_addr++;
  }

  void div(uint8_t d, uint8_t s1, uint8_t s2) {
    out << "div " << reg(d) << ", " << reg(s1) << ", " << reg(s2) << "\n";
    next_addr++;
  }

  void rem(uint8_t d, uint8_t s1, uint8_t s2) {
    out << "rem " << reg(d) << ", " << reg(s1) << ", " << reg(s2) << "\n";
    next_addr++;
  }

  void sll(uint8_t d, uint8_t s1, uint8_t s2) {
    out << "sll " << reg(d) << ", " << reg(s1) << ", " << reg(s2) << "\n";
    next_addr++;
  }
  void slt(uint8_t d, uint8_t s1, uint8_t s2) {
    out << "slt " << reg(d) << ", " << reg(s1) << ", " << reg(s2) << "\n";
    next_addr++;
  }

  void seq(uint8_t d, uint8_t s1, uint8_t s2) {
    out << "seq " << reg(d) << ", " << reg(s1) << ", " << reg(s2) << "\n";
    next_addr++;
  }

  void sne(uint8_t d, uint8_t s1, uint8_t s2) {
    out << "sne " << reg(d) << ", " << reg(s1) << ", " << reg(s2) << "\n";
    next_addr++;
  }

  void sge(uint8_t d, uint8_t s1, uint8_t s2) {
    out << "sge " << reg(d) << ", " << reg(s1) << ", " << reg(s2) << "\n";
    next_addr++;
  }

  // Memory Access
  void lw(uint8_t d, uint8_t s, int32_t imm) {
    out << "lw " << reg(d) << ", " << reg(s) << ", " << imm << "\n";
    next_addr++;
  }

  void sw(uint8_t s1, int32_t imm, uint8_t s2) {
    out << "sw " << reg(s1) << ", " << imm << ", " << reg(s2) << "\n";
    next_addr++;
  }

  // Jump and Link
  void jalr(uint8_t d, uint8_t s, int32_t imm) {
    out << "jalr " << reg(d) << ", " << reg(s) << ", " << imm << "\n";
    next_addr++;
  }

  void jal(uint8_t d, int32_t imm) {
    out << "jal " << reg(d) << ", " << imm << "\n";
    next_addr++;
  }

  void jal_label(uint8_t d, std::string label) {
    out << "jal " << reg(d) << ", " << label << "\n";
    next_addr++;
  }

  // Branches
  void beq(uint8_t s1, uint8_t s2, int32_t imm) {
    out << "beq " << reg(s1) << ", " << reg(s2) << ", " << imm << "\n";
    next_addr++;
  }

  void bne(uint8_t s1, uint8_t s2, int32_t imm) {
    out << "bne " << reg(s1) << ", " << reg(s2) << ", " << imm << "\n";
    next_addr++;
  }

  void blt(uint8_t s1, uint8_t s2, int32_t imm) {
    out << "blt " << reg(s1) << ", " << reg(s2) << ", " << imm << "\n";
    next_addr++;
  }

  void bge(uint8_t s1, uint8_t s2, int32_t imm) {
    out << "bge " << reg(s1) << ", " << reg(s2) << ", " << imm << "\n";
    next_addr++;
  }

  // System Instructions
  void ebreak() {
    out << "ebreak\n";
    next_addr++;
  }

  void eread(uint8_t d) {
    out << "eread " << reg(d) << "\n";
    next_addr++;
  }

  void ewrite(uint8_t s) {
    out << "ewrite " << reg(s) << "\n";
    next_addr++;
  }

  // Stack pseudo-instructions

  void push(uint8_t src) {
    addi(SP, SP, -1);
    sw(SP, 0, src);
  }

  void push_str(reg_allocator &alloc, std::string &str) {
    auto r = alloc.alloc("For pushing str on stack");
    addi(r, 0, '\0');
    sw(SP, -1, r);

    for (int i = 0; i < str.length(); i++) {
      auto offset = -i - 2;
      auto sym = *(str.rbegin() + i);
      addi(r, 0, sym);
      sw(SP, offset, r);
    }
    addi(SP, SP, -(str.length() + 1));
    alloc.dealloc(r);
  }

  void pop(uint8_t src) {
    lw(src, SP, 0);
    addi(SP, SP, 1);
  }

  // Arg num starts from 1
  void get_arg(uint8_t dest, uint8_t offset) { lw(dest, BP, -offset); }

  void put_arg(uint8_t source, uint8_t offset) { sw(BP, -offset, source); }

  void label(std::string label) { out << label << ": " << "\n"; }

  void mov(uint8_t dest, uint8_t src) { addi(dest, src, 0); }

  void call_start(reg_allocator &alloc, uint8_t args_count) {
    auto r = alloc.alloc("Call_Start push pc & puch bp");
    // Push pc
    jal(r, 0);
    addi(r, r, 7 + (args_count * 2)); // push of arg is 2 instr
    push(r);

    // Push bp (x30)
    push(BP);

    // BP <- SP
    mov(BP, SP);
    alloc.dealloc(r);
  }

  void call_end(uint8_t addr_reg) {
    // Jump
    jalr(0, addr_reg, 0);
  }

  void ret(reg_allocator &alloc) {
    // SP <- BP
    addi(SP, BP, 0);
    // Pop BP
    pop(BP);

    // Pop PC
    auto r = alloc.alloc("Pop PC");
    pop(r);
    jalr(0, r, 0);
    alloc.dealloc(r);
  }

  // Conditional branches to LABEL
  void beq(uint8_t s1, uint8_t s2, const std::string &label) {
    bne(s1, s2, 1);
    jal_label(0, label);
  }

  void bne(uint8_t s1, uint8_t s2, const std::string &label) {
    beq(s1, s2, 1);
    jal_label(0, label);
  }

  void blt(uint8_t s1, uint8_t s2, const std::string &label) {
    bge(s1, s2, 1);
    jal_label(0, label);
  }

  void bge(uint8_t s1, uint8_t s2, const std::string &label) {
    blt(s1, s2, 1);
    jal_label(0, label);
  }

  // Built in funcs

  void write_func(reg_allocator &alloc) {
    label("WRITE");
    auto r = alloc.alloc("Get WRITE arg from stack");
    get_arg(r, 1);
    ewrite(r);
    alloc.dealloc(r);
    ret(alloc);
  }

  void read_func(reg_allocator &alloc) {
    label("READ");
    eread(RR);
    ret(alloc);
  }

private:
  std::string reg(uint8_t n) { return "x" + std::to_string(n); }
};

} // namespace func::instr
