#pragma once

#include <memory>
#include <variant>
#include <vector>
namespace intrp {

enum class types : char { INT, STRING, BOOL, VOID, FUNCTION };

class type {
public:
  virtual types get_type() = 0;
  virtual std::unique_ptr<type> clone() = 0;
  virtual ~type() = default;
};

class int_type : public type {
public:
  types get_type() override;
  std::unique_ptr<type> clone() override;
};

class string_type : public type {
public:
  types get_type() override;
  std::unique_ptr<type> clone() override;
};

class bool_type : public type {
public:
  types get_type() override;
  std::unique_ptr<type> clone() override;
};

class void_type : public type {
public:
  types get_type() override;
  std::unique_ptr<type> clone() override;
};

class function_type : public type {
  std::vector<std::unique_ptr<type>> signature;

public:
  function_type(std::vector<std::unique_ptr<type>> &&sign);
  const std::vector<std::unique_ptr<type>>& get_signature();
  types get_type() override;
  std::unique_ptr<type> clone() override;
};

} // namespace intrp
