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
 * core/lexer.hh:
 *   Defines the `lexer` helper class which tokenizes text for the parser
 *
 *---------------------------------------------------------------------------*/

#ifndef CASCADE_CORE_LEXER_HH
#define CASCADE_CORE_LEXER_HH

#include <cstddef>
#include <filesystem>
#include <functional>
#include <limits>
#include <memory>
#include <string_view>
#include <vector>

namespace cascade::errors {
  class error;
}

namespace cascade::core {
  /** @brief Information that links a token/node to the original source */
  struct source_info {
    /** @brief The offset the object begins at in the source code */
    std::size_t position;

    /** @brief The line the object is on */
    std::size_t line;

    /** @brief The column the object is on */
    std::size_t col;

    /** @brief The length of the object */
    std::size_t length;

    /** @brief The path of the file */
    std::filesystem::path path;
  };

  /** @brief Represents a single lexical token */
  class token {
  public:
    /** @brief Represents the "what kind of" token it is */
    enum class kind : unsigned {
      /** @brief An "unknown" token, almost always an error */
      unknown = std::numeric_limits<std::underlying_type_t<kind>>::min(),
      /** @brief A detected error */
      error,
      /** @brief 'c' */
      literal_char,
      /** @brief "string" */
      literal_string,
      /** @brief 48230 */
      literal_number,
      /** @brief `true` | `false` */
      literal_bool,
      /** @brief An identifier */
      identifier,
      /** @brief `const` */
      keyword_const,
      /** @brief `static` */
      keyword_static,
      /** @brief `fn` */
      keyword_fn,
      /** @brief `struct` */
      keyword_struct,
      /** @brief `pub` */
      keyword_pub,
      /** @brief `let` */
      keyword_let,
      /** @brief `mut` */
      keyword_mut,
      /** @brief `loop` */
      keyword_loop,
      /** @brief `while` */
      keyword_while,
      /** @brief `for` */
      keyword_for,
      /** @brief `in` */
      keyword_in,
      /** @brief `break` */
      keyword_break,
      /** @brief `continue` */
      keyword_continue,
      /** @brief `ret` */
      keyword_ret,
      /** @brief `assert` */
      keyword_assert,
      /** @brief `module` */
      keyword_module,
      /** @brief `import` */
      keyword_import,
      /** @brief `as` */
      keyword_as,
      /** @brief `from` */
      keyword_from,
      /** @brief `export` */
      keyword_export,
      /** @brief `if` */
      keyword_if,
      /** @brief `then` */
      keyword_then,
      /** @brief `else` */
      keyword_else,
      /** @brief `and` */
      keyword_and,
      /** @brief `or` */
      keyword_or,
      /** @brief `xor` */
      keyword_xor,
      /** @brief `not` */
      keyword_not,
      /** @brief `=` */
      symbol_equal,
      /** @brief `:` */
      symbol_colon,
      /** @brief `::` */
      symbol_coloncolon,
      /** @brief `*` */
      symbol_star,
      /** @brief `&` */
      symbol_pound,
      /** @brief `[` */
      symbol_openbracket,
      /** @brief `]` */
      symbol_closebracket,
      /** @brief `@` */
      symbol_at,
      /** @brief `.` */
      symbol_dot,
      /** @brief `{` */
      symbol_openbrace,
      /** @brief `}` */
      symbol_closebrace,
      /** @brief `(` */
      symbol_openparen,
      /** @brief `)` */
      symbol_closeparen,
      /** @brief `;` */
      symbol_semicolon,
      /** @brief `|` */
      symbol_pipe,
      /** @brief `^` */
      symbol_caret,
      /** @brief `+` */
      symbol_plus,
      /** @brief `-` */
      symbol_hyphen,
      /** @brief `/` */
      symbol_forwardslash,
      /** @brief `%` */
      symbol_percent,
      /** @brief `<` */
      symbol_lt,
      /** @brief `<=` */
      symbol_leq,
      /** @brief `>` */
      symbol_gt,
      /** @brief `>=` */
      symbol_geq,
      /** @brief `>>` */
      symbol_gtgt,
      /** @brief `<<` */
      symbol_ltlt,
      /** @brief `==` */
      symbol_equalequal,
      /** @brief `!=` */
      symbol_bangequal,
      /** @brief `>>=` */
      symbol_gtgtequal,
      /** @brief `<<=` */
      symbol_ltltequal,
      /** @brief `&=` */
      symbol_poundequal,
      /** @brief `|=` */
      symbol_pipeequal,
      /** @brief `^=` */
      symbol_caretequal,
      /** @brief `%=` */
      symbol_percentequal,
      /** @brief `/=` */
      symbol_forwardslashequal,
      /** @brief `*=` */
      symbol_starequal,
      /** @brief `-=` */
      symbol_hyphenequal,
      /** @brief `+=` */
      symbol_plusequal,
      /** @brief ',' */
      symbol_comma,
    };

  private:
    /** @brief Source info for the token */
    source_info m_info;

    /** @brief The type of the token */
    kind m_type;

    /** @brief Pointer to the raw token */
    std::string_view m_raw;

  public:
    /**
     * @brief Creates a token
     * @param pos The position in the source string the token begins
     * @param line The line the token is on
     * @param col The column the token is on
     * @param type The type of token it is
     * @param raw The raw token
     * @param path The path of the file
     */
    token(std::size_t pos, std::size_t line, std::size_t col, kind type, std::string_view raw,
        std::filesystem::path path)
        : m_info({pos, line, col, raw.size(), path}), m_type(type), m_raw(std::move(raw)) {}

    /**
     * @brief Returns the token's offset in the source
     * @return An offset
     */
    [[nodiscard]] std::size_t position() const { return m_info.position; }

    /**
     * @brief Returns the line the token appears on
     * @return The line of the token
     */
    [[nodiscard]] std::size_t line() const { return m_info.line; }

    /**
     * @brief Returns the column of the token
     * @return The column number
     */
    [[nodiscard]] std::size_t column() const { return m_info.col; }

    /**
     * @brief Returns the number of characters in the token
     * @return The number of characters
     */
    [[nodiscard]] std::size_t length() const { return m_info.length; }

    /**
     * @brief Returns a string_view to the raw token,
     * or an error if token.type() == type::error
     * @return The raw token
     */
    [[nodiscard]] std::string_view raw() const { return m_raw; }

    /**
     * @brief Returns the token's type
     * @return The token's type
     */
    [[nodiscard]] kind type() const { return m_type; }

    /**
     * @brief Returns the path of the token
     * @return The token's path
     */
    [[nodiscard]] const std::filesystem::path &path() const { return m_info.path; }

    /**
     * @brief Returns if the token is of @p type
     * @param type The type to check for
     * @return If @p type is the token's type
     */
    [[nodiscard]] bool is(kind type) const { return this->type() == type; }

    /**
     * @brief Returns if the token is not of @p type, it's effectively
     * an !token.is(type)
     * @param type The type to check against
     * @return true if @p type doesn't match the token
     */
    [[nodiscard]] bool is_not(kind type) const { return !is(type); }

    /**
     * @brief Checks if the token is one of a list of type
     * @param first The first type to check against
     * @param rest The rest of the types to check against
     * @return If the token is one of the types provided
     */
    template <class... Rest>[[nodiscard]] bool is_one_of(kind first, Rest... rest) const {
      if (is(first)) {
        return true;
      }

      if constexpr (sizeof...(rest) > 0) {
        return is_one_of(std::forward<kind>(rest...));
      }

      return false;
    }

    /**
     * @brief Returns true if the token type is a literal type
     * @return If the token is a literal type
     */
    [[nodiscard]] bool is_literal() const;

    /**
     * @brief Returns true if the token type is a symbol type
     * @return If the token is a symbol type
     */
    [[nodiscard]] bool is_symbol() const;
  };

  /*
   ####################################################################
   *
   * DESIGN NOTE: This would have just been a freestanding function, but
   * making it a class to begin with made it much easier to potentially
   * change to a lazily-lexed implementation
   *
   ####################################################################
   */

  /** @brief The lexer class */
  class lexer {
    /** @brief Internal implementation of the lexer */
    class impl;

    /** @brief Pointer to the implementation */
    std::unique_ptr<impl> m_impl;

  public:
    using register_fn = std::function<void(std::unique_ptr<errors::error>)>;

    using return_type = std::vector<token>;

    /**
     * @brief Creates the lexer
     * @param source The source code
     * @param file_path The path of the file being lexed
     * @param register_error A function that's called any time an error is created
     * by the lexer. The error is passed into the function.
     */
    explicit lexer(
        std::string_view source, std::filesystem::path file_path, register_fn register_error);

    /**
     * @brief (eagerly) lexes the source string given
     * @return A list of tokens
     */
    return_type lex();

    /** @brief Implemented as default */
    ~lexer();
  };
} // namespace cascade::core

#endif