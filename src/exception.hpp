#pragma once

#include "codegen/location.hh"
#include <ostream>
#include <sstream>
#include <string>
namespace func {

enum error_codes{
    E_SYTNTAX = 1,
    E_SYMBOL = 2,
    E_TYPE = 3,
    E_PARAMS = 4,
    E_OTHER = 5,
};

struct syntax_exception {
    std::string reason;
    yy::location loc;
};

struct undeclared_variable_exception : public syntax_exception {};

struct unexpected_type_exception : public syntax_exception {};

struct symbol_redeclaration_exception : public syntax_exception {
    symbol_redeclaration_exception(const std::string& sym,
                                    yy::location origin_loc,
                                    yy::location current_loc) {
        this->loc = current_loc;
        std::stringstream ss{};
        ss << "Symbol redeclaration: " << sym << " originally declared at " << origin_loc;
        this->reason = ss.str();
    }
};

struct symbol_not_found_exception : public syntax_exception {};

struct not_enough_registers_exception {};

struct global_syntax_exception {
    std::string reason;
};

std::ostream& operator<<(std::ostream& outs, const syntax_exception& e);
std::ostream& operator<<(std::ostream& outs, const global_syntax_exception& e);
} // namespace func