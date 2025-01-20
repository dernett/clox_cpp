#ifndef clox_vm_h
#define clox_vm_h

#include <cstdint>

#include "chunk.hpp"
#include "compiler.hpp"
#include "value.hpp"

namespace clox {
enum InterpretResult : uint8_t {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR
};

class VM {
  Chunk chunk;
  size_t ip = 0;
  std::vector<Value> stack;

public:
  InterpretResult interpret(const char *source) {
    Chunk compiledChunk;
    Compiler compiler(source, compiledChunk);

    if (!compiler.compile()) {
      return INTERPRET_COMPILE_ERROR;
    }

    chunk = std::move(compiledChunk);
    ip = 0;

    return run();
  }

  void push(Value value) { stack.push_back(value); }

  [[nodiscard]] Value peek(size_t distance) const {
    assert(distance < stack.size());
    return stack[stack.size() - 1 - distance];
  }

  Value pop() {
    assert(!stack.empty());
    Value value = stack.back();
    stack.pop_back();
    return value;
  }

  template <class ValueType, class BinaryOp>
  InterpretResult binaryOp(ValueType valueType, BinaryOp op) {
    if (!peek(0).isNumber() || !peek(1).isNumber()) {
      runtimeError("Operands must be numbers.");
      return INTERPRET_RUNTIME_ERROR;
    }
    double b = pop().asNumber();
    double a = pop().asNumber();
    push(valueType(op(a, b)));
    return INTERPRET_OK;
  }

  InterpretResult run();

private:
  uint8_t readByte() { return chunk.getCode(ip++); }

  Value readConstant() { return chunk.getConstant(readByte()); }

  template <typename... Args>
  void runtimeError(std::format_string<Args...> fmt, Args &&...args) {
    std::println(std::cerr, fmt, std::forward(args)...);

    int line = chunk.getLine(ip - 1);
    std::println(std::cerr, "[line {}] in script", line);
    stack.clear();
  }
};

}; // namespace clox

#endif
