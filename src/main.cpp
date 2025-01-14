#include <filesystem>
#include <iostream>
#include <print>

namespace fs = std::filesystem;

namespace {
void repl() {}

void runFile(const fs::path &path) { (void)path; }
} // namespace

int main(int argc, char *argv[]) {
  if (argc == 1) {
    repl();
  } else if (argc == 2) {
    runFile(argv[1]);
  } else {
    std::println(std::cerr, "Usage: clox [path]");
    std::exit(64);
  }

  return 0;
}
