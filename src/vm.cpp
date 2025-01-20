#include "vm.hpp"
#include "common.hpp"

#include <functional>

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
    InterpretResult flag = INTERPRET_OK;
    uint8_t instruction{};
    switch (instruction = readByte()) {
    case OP_CONSTANT:
      push(readConstant());
      break;
    case OP_NIL:
      push(Value::Nil());
      break;
    case OP_TRUE:
      push(Value::Bool(true));
      break;
    case OP_FALSE:
      push(Value::Bool(false));
      break;
    case OP_ADD:
      flag = binaryOp(Value::Number, std::plus());
      break;
    case OP_SUBTRACT:
      flag = binaryOp(Value::Number, std::minus());
      break;
    case OP_MULTIPLY:
      flag = binaryOp(Value::Number, std::multiplies());
      break;
    case OP_DIVIDE:
      flag = binaryOp(Value::Number, std::divides());
      break;
    case OP_NEGATE:
      if (!peek(0).isNumber()) {
        runtimeError("Operand must be a number.");
        return INTERPRET_RUNTIME_ERROR;
      }
      push(Value::Number(-pop().asNumber()));
      break;
    case OP_NOT:
      push(Value::Bool(pop().isFalsey()));
      break;
    case OP_RETURN:
      std::println("{}", pop());
      return INTERPRET_OK;
    }
    if (flag != INTERPRET_OK)
      return flag;
  }
}
} // namespace clox
