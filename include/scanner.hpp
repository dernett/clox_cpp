#ifndef clox_scanner_hpp
#define clox_scanner_hpp

#include <cstdint>
#include <string_view>

namespace clox {

enum TokenType : uint8_t {
  // Single-character tokens.
  TOKEN_LEFT_PAREN,
  TOKEN_RIGHT_PAREN,
  TOKEN_LEFT_BRACE,
  TOKEN_RIGHT_BRACE,
  TOKEN_COMMA,
  TOKEN_DOT,
  TOKEN_MINUS,
  TOKEN_PLUS,
  TOKEN_SEMICOLON,
  TOKEN_SLASH,
  TOKEN_STAR,
  // One or two character tokens.
  TOKEN_BANG,
  TOKEN_BANG_EQUAL,
  TOKEN_EQUAL,
  TOKEN_EQUAL_EQUAL,
  TOKEN_GREATER,
  TOKEN_GREATER_EQUAL,
  TOKEN_LESS,
  TOKEN_LESS_EQUAL,
  // Literals.
  TOKEN_IDENTIFIER,
  TOKEN_STRING,
  TOKEN_NUMBER,
  // Keywords.
  TOKEN_AND,
  TOKEN_CLASS,
  TOKEN_ELSE,
  TOKEN_FALSE,
  TOKEN_FOR,
  TOKEN_FUN,
  TOKEN_IF,
  TOKEN_NIL,
  TOKEN_OR,
  TOKEN_PRINT,
  TOKEN_RETURN,
  TOKEN_SUPER,
  TOKEN_THIS,
  TOKEN_TRUE,
  TOKEN_VAR,
  TOKEN_WHILE,

  TOKEN_ERROR,
  TOKEN_EOF
};

// TODO: Reorder to minimize padding?
struct Token {
  TokenType type;
  std::string_view str;
  int line;
};

class Scanner {
  const char *start;
  const char *current;
  int line = 1;

  [[nodiscard]] static bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
  }

  [[nodiscard]] static bool isDigit(char c) { return c >= '0' && c <= '9'; }

public:
  explicit Scanner(const char *source) : start(source), current(source) {}

  Token scanToken() {
    skipWhitespace();
    start = current;

    if (isAtEnd())
      return makeToken(TOKEN_EOF);

    char c = advance();

    if (isAlpha(c))
      return identifier();

    if (isDigit(c))
      return number();

    switch (c) {
    case '(':
      return makeToken(TOKEN_LEFT_PAREN);
    case ')':
      return makeToken(TOKEN_RIGHT_PAREN);
    case '{':
      return makeToken(TOKEN_LEFT_BRACE);
    case '}':
      return makeToken(TOKEN_RIGHT_BRACE);
    case ';':
      return makeToken(TOKEN_SEMICOLON);
    case ',':
      return makeToken(TOKEN_COMMA);
    case '.':
      return makeToken(TOKEN_DOT);
    case '-':
      return makeToken(TOKEN_MINUS);
    case '+':
      return makeToken(TOKEN_PLUS);
    case '/':
      return makeToken(TOKEN_SLASH);
    case '*':
      return makeToken(TOKEN_STAR);
    case '!':
      return makeToken(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
    case '=':
      return makeToken(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
    case '<':
      return makeToken(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
    case '>':
      return makeToken(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
    case '"':
      return string();
    default:
      break;
    }

    return errorToken("Unexpected character.");
  }

private:
  [[nodiscard]] bool isAtEnd() const { return *current == '\0'; }

  char advance() {
    current++;
    return current[-1];
  }

  [[nodiscard]] char peek() const { return *current; }

  [[nodiscard]] char peekNext() const {
    if (isAtEnd())
      return '\0';
    return current[1];
  }

  bool match(char expected) {
    if (isAtEnd())
      return false;
    if (*current != expected)
      return false;
    current++;
    return true;
  }

  [[nodiscard]] Token makeToken(TokenType type) const {
    return {.type = type,
            .str = {start, static_cast<size_t>(current - start)},
            .line = line};
  }

  [[nodiscard]] Token errorToken(std::string_view message) const {
    return {.type = TOKEN_ERROR, .str = message, .line = line};
  }

  void skipWhitespace() {
    for (;;) {
      char c = peek();
      switch (c) {
      case ' ':
      case '\r':
      case '\t':
        advance();
        break;
      case '\n':
        line++;
        advance();
        break;
      case '/':
        if (peekNext() == '/') {
          // A comment goes until the end of the line.
          while (peek() != '\n' && !isAtEnd())
            advance();
        } else {
          return;
        }
        break;
      default:
        return;
      }
    }
  }

  [[nodiscard]] TokenType checkKeyword(int offset, int length,
                                       std::string_view rest,
                                       TokenType type) const {
    if (current - start == offset + length &&
        std::string_view(start + offset, length) == rest)
      return type;
    return TOKEN_IDENTIFIER;
  }

  [[nodiscard]] TokenType identifierType() const {
    switch (start[0]) {
    case 'a':
      return checkKeyword(1, 2, "nd", TOKEN_AND);
    case 'c':
      return checkKeyword(1, 4, "lass", TOKEN_CLASS);
    case 'e':
      return checkKeyword(1, 3, "lse", TOKEN_ELSE);
    case 'f':
      if (current - start > 1) {
        switch (start[1]) {
        case 'a':
          return checkKeyword(2, 3, "lse", TOKEN_FALSE);
        case 'o':
          return checkKeyword(2, 1, "r", TOKEN_FOR);
        case 'u':
          return checkKeyword(2, 1, "n", TOKEN_FUN);
        default:
          break;
        }
      }
      break;
    case 'i':
      return checkKeyword(1, 1, "f", TOKEN_IF);
    case 'n':
      return checkKeyword(1, 2, "il", TOKEN_NIL);
    case 'o':
      return checkKeyword(1, 1, "r", TOKEN_OR);
    case 'p':
      return checkKeyword(1, 4, "rint", TOKEN_PRINT);
    case 'r':
      return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
    case 's':
      return checkKeyword(1, 4, "uper", TOKEN_SUPER);
    case 't':
      if (current - start > 1) {
        switch (start[1]) {
        case 'h':
          return checkKeyword(2, 2, "is", TOKEN_THIS);
        case 'r':
          return checkKeyword(2, 2, "ue", TOKEN_TRUE);
        default:
          break;
        }
      }
      break;
    case 'v':
      return checkKeyword(1, 2, "ar", TOKEN_VAR);
    case 'w':
      return checkKeyword(1, 4, "hile", TOKEN_WHILE);
    default:
      break;
    }

    return TOKEN_IDENTIFIER;
  }

  Token identifier() {
    while (isAlpha(peek()) || isDigit(peek()))
      advance();
    return makeToken(identifierType());
  }

  Token number() {
    while (isDigit(peek()))
      advance();

    // Look for a fractional part.
    if (peek() == '.' && isDigit(peekNext())) {
      // Consume the ".".
      advance();

      while (isDigit(peek()))
        advance();
    }

    return makeToken(TOKEN_NUMBER);
  }

  Token string() {
    while (peek() != '"' && !isAtEnd()) {
      if (peek() == '\n')
        line++;
      advance();
    }

    if (isAtEnd())
      return errorToken("Unterminated string.");

    // The closing quote.
    advance();
    return makeToken(TOKEN_STRING);
  }
};
}; // namespace clox

#endif
