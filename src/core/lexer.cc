/*---------------------------------------------------------------------------*
 *
 * Copyright 2020 Evan Cox
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *---------------------------------------------------------------------------*
 *
 * core/lexer.cc:
 *   Implements the `lexer` helper class which tokenizes text for the parser
 *
 *---------------------------------------------------------------------------*/

#include "cascade/core/lexer.hh"
#include "cascade/errors/error.hh"
#include "cascade/util/keywords.hh"
#include <cassert>
#include <optional>
#include <type_traits>
#include <unordered_map>

using namespace cascade;
using namespace core;
using ec = errors::error_code;

namespace fs = std::filesystem;

// symbols that are only ever single char
static std::unordered_map<std::string_view, token::kind> single_char_symbols{
    {"[", token::kind::symbol_openbracket},
    {"]", token::kind::symbol_closebracket},
    {"@", token::kind::symbol_at},
    {".", token::kind::symbol_dot},
    {"{", token::kind::symbol_openbrace},
    {"}", token::kind::symbol_closebrace},
    {"(", token::kind::symbol_openparen},
    {")", token::kind::symbol_closeparen},
    {";", token::kind::symbol_semicolon},
    {",", token::kind::symbol_comma},
};

// symbols that either are 2 characters, or start with the same character as a 2 char one
static std::unordered_map<std::string_view, token::kind> one_or_two_char_symbols{
    {"=", token::kind::symbol_equal},
    {":", token::kind::symbol_colon},
    {"::", token::kind::symbol_coloncolon},
    {"*", token::kind::symbol_star},
    {"&", token::kind::symbol_pound},
    {"|", token::kind::symbol_pipe},
    {"^", token::kind::symbol_caret},
    {"+", token::kind::symbol_plus},
    {"-", token::kind::symbol_hyphen},
    {"/", token::kind::symbol_forwardslash},
    {"%", token::kind::symbol_percent},
    {"<", token::kind::symbol_lt},
    {"<=", token::kind::symbol_leq},
    {">", token::kind::symbol_gt},
    {">=", token::kind::symbol_geq},
    {">>", token::kind::symbol_gtgt},
    {"<<", token::kind::symbol_ltlt},
    {"==", token::kind::symbol_equalequal},
    {"!=", token::kind::symbol_bangequal},
    {">>=", token::kind::symbol_gtgtequal},
    {"<<=", token::kind::symbol_ltltequal},
    {"&=", token::kind::symbol_poundequal},
    {"|=", token::kind::symbol_pipeequal},
    {"^=", token::kind::symbol_caretequal},
    {"%=", token::kind::symbol_percentequal},
    {"/=", token::kind::symbol_forwardslashequal},
    {"*=", token::kind::symbol_starequal},
    {"-=", token::kind::symbol_hyphenequal},
    {"+=", token::kind::symbol_plusequal},
};

bool token::is_literal() const {
  return type() == kind::literal_bool || type() == kind::literal_char
         || type() == kind::literal_string || type() == kind::literal_number;
}

bool token::is_symbol() const {
  // sorry about this.
  // if it makes you feel any better, I didn't type this all out by hand.
  return type() == kind::symbol_equal || type() == kind::symbol_colon || type() == kind::symbol_star
         || type() == kind::symbol_pound || type() == kind::symbol_openbracket
         || type() == kind::symbol_closebracket || type() == kind::symbol_at
         || type() == kind::symbol_dot || type() == kind::symbol_openbrace
         || type() == kind::symbol_closebrace || type() == kind::symbol_semicolon
         || type() == kind::symbol_pipe || type() == kind::symbol_caret
         || type() == kind::symbol_plus || type() == kind::symbol_hyphen
         || type() == kind::symbol_forwardslash || type() == kind::symbol_percent
         || type() == kind::symbol_lt || type() == kind::symbol_leq || type() == kind::symbol_gt
         || type() == kind::symbol_geq || type() == kind::symbol_gtgt || type() == kind::symbol_ltlt
         || type() == kind::symbol_equalequal || type() == kind::symbol_bangequal
         || type() == kind::symbol_gtgtequal || type() == kind::symbol_ltltequal
         || type() == kind::symbol_poundequal || type() == kind::symbol_pipeequal
         || type() == kind::symbol_caretequal || type() == kind::symbol_percentequal
         || type() == kind::symbol_forwardslashequal || type() == kind::symbol_starequal
         || type() == kind::symbol_hyphenequal || type() == kind::symbol_plusequal;
}

/** @brief Implementation of the internal `impl` type */
class lexer::impl {
  /** @brief string_view to the source code */
  std::string_view m_source;

  /** @brief The current path being lexed */
  fs::path m_path;

  /** @brief The current position in the source */
  std::size_t m_pos = 0;

  /** @brief The current line */
  std::size_t m_line = 1;

  /** @brief The current column */
  std::size_t m_col = 1;

  /** @brief Used whenever a multi-char token is being consumed */
  std::size_t m_starting_pos = 0;

  /** @brief Used whenever a multi-char token is being consumed */
  std::size_t m_starting_line = 1;

  /** @brief Used whenever a multi-char token is being consumed */
  std::size_t m_starting_col = 1;

  /** @brief Iterator to m_source */
  std::string_view::const_iterator m_it;

  /** @brief The function to call with registered errors */
  lexer::register_fn m_register;

  /** @brief Updates the m_starting_* fields with the current lexer state */
  void update_starting();

  /**
   * @brief Creates a new token based on the lexer's current state
   * @param kind The kind of token to create
   * @param raw The raw token
   * @return A new token
   */
  [[nodiscard]] token create_token(token::kind kind, std::string_view raw) const;

  /**
   * @brief Creates and registers an error
   * @param code The error code
   * @param tok The error-causing token
   * @param note A helpful "note" message
   */
  void create_error(ec code, token tok, std::string note = "");

  /**
   * @brief Returns the next character, or EOF
   * @return The next char
   */
  [[nodiscard]] char peek() const;

  /**
   * @brief Returns the current character, AKA the one about to be consumed
   * @return The current character
   */
  [[nodiscard]] char current() const;

  /**
   * @brief Returns if the lexer is at the end of the characters
   * @return Whether lexer is at the end
   */
  [[nodiscard]] bool is_at_end() const;

  /**
   * @brief Consumes a digit literal and returns a token for it
   * @return The token for the digits
   */
  [[nodiscard]] std::optional<token> consume_digits();

  /**
   * @brief Consumes a keyword or an identifier
   * @return A token for the id/keyword
   */
  [[nodiscard]] std::optional<token> consume_identifier();

  /**
   * @brief Consumes a string literal
   * @return The string literal as a token
   */
  template <char C = '"'>[[nodiscard]] std::optional<token> consume_stringlike() {
    static_assert(C == '"' || C == '\'', "C must be character or string delimiter!");

    consume(); // consume first C

    while (!is_at_end() && current() != C) {
      if (current() == '\\' && peek() == C) {
        consume(2);

        continue;
      }

      consume();
    }

    if (is_at_end()) {
      auto kind = token::kind::literal_string;
      auto code = ec::unterminated_str;

      if constexpr (C == '\'') {
        code = ec::unterminated_char;
        kind = token::kind::literal_char;
      }

      create_error(
          code, create_token(kind, m_source.substr(m_starting_pos, m_pos - m_starting_pos)));

      return std::nullopt;
    }

    consume(); // consume ending C

    if constexpr (C == '"') {
      return create_token(
          token::kind::literal_string, m_source.substr(m_starting_pos, m_pos - m_starting_pos));
    } else {
      return create_token(
          token::kind::literal_char, m_source.substr(m_starting_pos, m_pos - m_starting_pos));
    }
  }

  /**
   * @brief Consumes a character from the string
   * @param n The number of characters to consume
   * @return The character consumed
   */
  char consume(int n = 1);

public:
  impl(std::string_view src, fs::path path, register_fn func)
      : m_source(src), m_path(path), m_it(m_source.begin()), m_register(func) {}

  std::vector<token> lex();
};

lexer::lexer(std::string_view source, fs::path path, register_fn register_func)
    : m_impl(std::make_unique<lexer::impl>(source, path, register_func)) {}

lexer::~lexer() = default;

std::vector<token> lexer::lex() {
  // at the moment it just needs to call the impl parse()
  return m_impl->lex();
}

char lexer::impl::peek() const { return m_it == m_source.end() ? EOF : m_it[1]; }

char lexer::impl::current() const {
  // no checks needed, if it's past the end it needs to crash at this point
  return *m_it;
}

bool lexer::impl::is_at_end() const {
  // current() won't return EOF, EOF isn't actually in the source string
  // peek() returns it if the current() is past the end
  return peek() == EOF;
}

token lexer::impl::create_token(token::kind kind, std::string_view raw) const {
  return token(m_starting_pos, m_starting_line, m_starting_col, kind, raw, m_path);
}

void lexer::impl::create_error(ec code, token tok, std::string note) {
  m_register(errors::error::from(code, tok, note));
}

void lexer::impl::update_starting() {
  m_starting_pos = m_pos;
  m_starting_line = m_line;
  m_starting_col = m_col;
}

char lexer::impl::consume(int n) {
  for (auto i = 0; i < n; ++i) {
    switch (current()) {
      case '\n':
        ++m_line;
        m_col = 1;
        break;
      default:
        ++m_col;
        break;
    }

    ++m_pos;
    ++m_it;
  }

  return *m_it;
}

std::optional<token> lexer::impl::consume_digits() {
  while (!is_at_end() && (std::isdigit(current()) || current() == '.')) {
    consume();
  }

  if (std::isalpha(current())) {
    auto result = consume_identifier();

    create_error(ec::unexpected_tok, result.value(), "Did you leave out a space?");

    return std::nullopt;
  }

  return create_token(
      token::kind::literal_number, m_source.substr(m_starting_pos, m_pos - m_starting_pos));
}

std::optional<token> lexer::impl::consume_identifier() {
  while (!is_at_end() && (std::isalpha(current()) || std::isdigit(current()) || current() == '_')) {
    consume();
  }

  auto full = m_source.substr(m_starting_pos, m_pos - m_starting_pos);

  return create_token(
      util::is_kind(full) ? util::kind_from_string(full) : token::kind::identifier, full);
}

std::vector<token> lexer::impl::lex() {
  std::vector<token> tokens;

  while (!is_at_end()) {
    // chew through any whitespace
    if (std::isspace(current())) {
      do {
        consume();
      } while (std::isspace(current()));

      // it may or may not be at the end of the file, so the loop needs to restart to check
      continue;
    }

    // everything past the spaces needs to get the m_starting_* fields updated
    update_starting();

    // handle line comments
    if (current() == '-' && peek() == '-') {
      consume(2);

      while (!is_at_end() && current() != '\n') {
        consume();
      }

      if (is_at_end()) {
        continue;
      }

      consume();
    }

    else if (current() == '-' && peek() == '*') {
      consume(2);

      while (!is_at_end() && !(current() == '*' && peek() == '-')) {
        consume();
      }

      if (is_at_end()) {
        create_error(ec::unterminated_block_comment,
            create_token(token::kind::error, m_source.substr(m_starting_pos, 2)),
            "did you leave out '*-' to end the comment?");
      } else {
        consume(2);
      }
    }

    // handle digit literals
    else if (std::isdigit(current())) {
      if (auto result = consume_digits(); result) {
        tokens.emplace_back(std::move(result.value()));
      }
    }

    // handle keywords / identifiers
    else if (std::isalpha(current()) || current() == '_') {
      tokens.emplace_back(consume_identifier().value());
    }

    else if (auto single_char_search = single_char_symbols.find(std::string{current()});
             single_char_search != single_char_symbols.end()) {
      tokens.emplace_back(create_token(single_char_search->second, m_source.substr(m_pos, 1)));
      consume();
    }

    // could be a symbol with 2 characters
    else if (auto twoc_search_1 = one_or_two_char_symbols.find(std::string{current(), peek()});
             twoc_search_1 != one_or_two_char_symbols.end()) {
      tokens.emplace_back(create_token(twoc_search_1->second, m_source.substr(m_pos, 2)));
      consume(2);
    }

    // could be a 1 char symbol that starts with the same symbol as a 2 char
    else if (auto twoc_search_2 = one_or_two_char_symbols.find(std::string{current()});
             twoc_search_2 != one_or_two_char_symbols.end()) {
      tokens.emplace_back(create_token(twoc_search_2->second, m_source.substr(m_pos, 1)));
      consume();
    }

    // handle string literals
    else if (current() == '"') {
      if (auto result = consume_stringlike<'"'>(); result) {
        tokens.emplace_back(std::move(result.value()));
      }
    }

    // handle char literals
    else if (current() == '\'') {
      if (auto result = consume_stringlike<'\''>(); result) {
        tokens.emplace_back(std::move(result.value()));
      }
    }

    else {
      create_error(ec::unknown_char, create_token(token::kind::unknown, m_source.substr(m_pos, 1)));
      consume();
    }
  }

  return tokens;
}
