#ifndef clox_vm_h
#define clox_vm_h

#include <cstdint>

namespace clox {
enum InterpretResult : uint8_t {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR
};

InterpretResult interpret(const char *source);
}; // namespace clox

#endif
