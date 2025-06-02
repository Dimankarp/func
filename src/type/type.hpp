#pragma once

#include <memory>
#include <variant>
#include <vector>
namespace intrp {

enum class types : char { INT, STRING, BOOL, VOID, FUNCTION };

class type {
public:
  virtual types get_type() = 0;
  virtual ~type() = default;
};

class int_type : public type {
public:
  types get_type() override;
};

class string_type : public type {
public:
  types get_type() override;
};

class bool_type : public type {
public:
  types get_type() override;
};

class void_type : public type {
public:
  types get_type() override;
};

class function_type : public type {
  std::vector<std::unique_ptr<type>> signature;

public:
  function_type(std::vector<std::unique_ptr<type>> &&sign);
  types get_type() override;
};

} // namespace intrp
