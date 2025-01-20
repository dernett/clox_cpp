#ifndef clox_value_h
#define clox_value_h

#include <cassert>
#include <print>

namespace clox {

enum ValueType : uint8_t {
  VAL_BOOL,
  VAL_NIL,
  VAL_NUMBER,
};

class Value {
  ValueType type;
  union {
    bool boolean;
    double number;
  };

public:
  [[nodiscard]] static Value Bool(bool value) {
    Value v{};
    v.type = VAL_BOOL;
    v.boolean = value;
    return v;
  }

  [[nodiscard]] static Value Nil() {
    Value v{};
    v.type = VAL_NIL;
    return v;
  }

  [[nodiscard]] static Value Number(double value) {
    Value v{};
    v.type = VAL_NUMBER;
    v.number = value;
    return v;
  }

  [[nodiscard]] bool isBool() const { return getType() == VAL_BOOL; }

  [[nodiscard]] bool isNil() const { return getType() == VAL_NIL; }

  [[nodiscard]] bool isNumber() const { return getType() == VAL_NUMBER; }

  [[nodiscard]] bool isFalsey() const {
    return isNil() || (isBool() && !asBool());
  }

  [[nodiscard]] bool asBool() const {
    assert(type == VAL_BOOL);
    return boolean;
  }

  [[nodiscard]] double asNumber() const {
    assert(type == VAL_NUMBER);
    return number;
  }

  [[nodiscard]] ValueType getType() const { return type; }

private:
  Value() = default;
};
}; // namespace clox

template <>
struct std::formatter<clox::Value> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(const clox::Value &value, std::format_context &ctx) const {
    switch (value.getType()) {
    case clox::VAL_BOOL:
      return std::format_to(ctx.out(), "{}", value.asBool());
    case clox::VAL_NIL:
      return std::format_to(ctx.out(), "nil");
    case clox::VAL_NUMBER:
      return std::format_to(ctx.out(), "{}", value.asNumber());
    }
  }
};

#endif
