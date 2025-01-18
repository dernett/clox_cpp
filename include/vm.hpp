#ifndef clox_vm_h
#define clox_vm_h

#include <cstdint>

#include "chunk.hpp"
#include "compiler.hpp"

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

  Value pop() {
    assert(!stack.empty());
    Value value = stack.back();
    stack.pop_back();
    return value;
  }

  template <class BinaryOp>
  void binaryOp(BinaryOp op) {
    double b = pop().asNumber();
    double a = pop().asNumber();
    push(Value::Number(op(a, b)));
  }

  InterpretResult run();

private:
  uint8_t readByte() { return chunk.getCode(ip++); }

  Value readConstant() { return chunk.getConstant(readByte()); }
};

}; // namespace clox

#endif
