#ifndef clox_value_h
#define clox_value_h

#include <cstdint>
#include <print>

namespace clox {
class Value {
public:
  enum ValueType : uint8_t { VAL_NUMBER };

  explicit Value(ValueType type) : type(type) {}

  virtual ~Value() = default;

  [[nodiscard]] ValueType getType() const { return type; }

  virtual void print() = 0;

private:
  ValueType type;
};

class Number : public Value {
  double number;

public:
  explicit Number(double number) : Value(VAL_NUMBER), number(number) {}

  [[nodiscard]] double getValue() const { return number; }

  void print() override { std::print("{}", number); }
};

}; // namespace clox
#endif
