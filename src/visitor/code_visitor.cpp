#pragma once

#include "visitor/code_visitor.hpp"
#include "exception.hpp"
#include "location.hh"
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

  out << "_START:\n";
  instr::call(out, alloc, "MAIN");
  out << "ebreak\n";

  for (auto &func : progr.get_funcs()) {
    func->accept(*this);
  }
}

} // namespace intrp
