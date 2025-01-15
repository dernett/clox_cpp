#include <filesystem>
#include <fstream>
#include <iostream>
#include <print>
#include <sstream>
#include <string>

#include "vm.hpp"

namespace fs = std::filesystem;

static void repl() {
  std::string line;
  for (;;) {
    std::print("> ");

    if (!std::getline(std::cin, line)) {
      std::println();
      break;
    }

    clox::interpret(line.c_str());
  }
}

static std::string readFile(const fs::path &path) {
  std::ifstream file(path, std::ios::in | std::ios::binary);
  if (!file.is_open()) {
    std::println(std::cerr, "Could not open file \"{}\".", path.string());
    std::exit(74);
  }

  std::ostringstream buffer;
  buffer << file.rdbuf();
  if (!file) {
    std::println(std::cerr, "Could not read file \"{}\".", path.string());
    std::exit(74);
  }

  return buffer.str();
}

static void runFile(const fs::path &path) {
  std::string source = readFile(path);
  clox::InterpretResult result = clox::interpret(source.c_str());

  if (result == clox::INTERPRET_COMPILE_ERROR)
    std::exit(65);
  if (result == clox::INTERPRET_RUNTIME_ERROR)
    std::exit(70);
}

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
