#include "vm.hpp"
#include "common.hpp"

namespace clox {
InterpretResult VM::run() {
  for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
    std::print("          ");
    for (Value value : stack) {
      std::print("[ {} ]", value);
    }
    std::println();
    chunk.disassembleInstruction(ip);
#endif
    uint8_t instruction{};
    switch (instruction = readByte()) {
    case OP_CONSTANT:
      push(readConstant());
      break;
    case OP_ADD:
      binaryOp(std::plus());
      break;
    case OP_SUBTRACT:
      binaryOp(std::minus());
      break;
    case OP_MULTIPLY:
      binaryOp(std::multiplies());
      break;
    case OP_DIVIDE:
      binaryOp(std::divides());
      break;
    case OP_NEGATE:
      push(Value::Number(-pop().asNumber()));
      break;
    case OP_RETURN:
      std::println("{}", pop());
      return INTERPRET_OK;
    }
  }
}
} // namespace clox
