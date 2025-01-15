#include <print>

#include "compiler.hpp"
#include "scanner.hpp"

namespace clox {
void compile(const char *source) {
  Scanner scanner(source);
  int line = -1;
  for (;;) {
    Token token = scanner.scanToken();
    if (token.line != line) {
      std::print("{:4} ", token.line);
      line = token.line;
    } else {
      std::print("   | ");
    }
    std::println("{:2} '{}'", static_cast<int>(token.type), token.str);

    if (token.type == TOKEN_EOF)
      break;
  }
}
}; // namespace clox
