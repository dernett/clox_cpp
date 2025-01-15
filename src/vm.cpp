#include "vm.hpp"
#include "compiler.hpp"

namespace clox {
InterpretResult interpret(const char *source) {
  compile(source);
  return INTERPRET_OK;
}
}; // namespace clox
