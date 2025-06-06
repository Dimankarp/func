#pragma once

#include "location.hh"
#include "type/type.hpp"
namespace intrp {

    void expect_types(const type& expected, const type& checked, yy::location loc);

}