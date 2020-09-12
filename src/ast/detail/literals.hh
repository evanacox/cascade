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
 * ast/detail/literals.hh:
 *   Outlines the literal AST nodes
 *
 *---------------------------------------------------------------------------*/

#ifndef CASCADE_AST_DETAIL_LITERALS_HH
#define CASCADE_AST_DETAIL_LITERALS_HH

#include "ast/detail/nodes.hh"
#include "core/lexer.hh"

namespace cascade::ast {
  /** @brief Tags a literal's type, for use when getting the value */
  enum class literal_type { lit_char, lit_string, lit_int, lit_bool, lit_float };

  /** @brief Represents an arbritrary (tagged) literal */
  class literal : public expression {
  public:
    explicit literal(kind k, core::source_info info) : expression(k, std::move(info)) {}

    // prevent name hiding
    using expression::is;
    using expression::is_not;
    using expression::is_one_of;

    /** @brief Returns if the literal is of @p type */
    [[nodiscard]] virtual bool is(literal_type type) const = 0;

    /** @brief Returns if the literal is not of @p type */
    [[nodiscard]] bool is_not(literal_type type) const { return !is(type); }

    /** @brief Returns if the literal is of @p first or @p rest */
    template <class... Rest>[[nodiscard]] bool is_one_of(literal_type first, Rest... rest) const {
      if (is(first)) {
        return true;
      }

      if constexpr (sizeof...(rest) > 0) {
        return is_one_of(std::forward<literal_type>(rest...));
      }

      return false;
    }
  };

  class char_literal : public literal, public visitable<char_literal> {
    char m_value;

  public:
    /**
     * @brief Creates a new char literal
     * @param info The source info
     * @param c The character to load
     */
    explicit char_literal(core::source_info info, char c)
        : literal(kind::literal_char, std::move(info))
        , m_value(c) {}

    [[nodiscard]] virtual bool is(literal_type type) const {
      return type == literal_type::lit_char;
    }

    [[nodiscard]] char value() const { return m_value; }
  };

  class string_literal : public literal, public visitable<string_literal> {
    std::string m_value;

  public:
    /**
     * @brief Creates a new string literal
     * @param info The source info
     * @param str Pointer to the literal string in the source
     */
    explicit string_literal(core::source_info info, std::string_view str)
        : literal(kind::literal_string, std::move(info))
        , m_value(std::string{str}) {}

    [[nodiscard]] virtual bool is(literal_type type) const {
      return type == literal_type::lit_string;
    }

    [[nodiscard]] std::string_view value() const { return m_value; }
  };

  class int_literal : public literal, public visitable<int_literal> {
    int m_value;

  public:
    /**
     * @brief Creates a new int literal
     * @param info The source info
     * @param n The value of the literal
     */
    explicit int_literal(core::source_info info, int n)
        : literal(kind::literal_number, std::move(info))
        , m_value(n) {}

    [[nodiscard]] virtual bool is(literal_type type) const { return type == literal_type::lit_int; }

    [[nodiscard]] int value() const { return m_value; }
  };

  class float_literal : public literal, public visitable<float_literal> {
    float m_value;

  public:
    /**
     * @brief Creates a new float literal
     * @param info The source info
     * @param n The value of the literal
     */
    explicit float_literal(core::source_info info, float n)
        : literal(kind::literal_float, std::move(info))
        , m_value(n) {}

    [[nodiscard]] virtual bool is(literal_type type) const {
      return type == literal_type::lit_float;
    }

    [[nodiscard]] float value() const { return m_value; }
  };

  class bool_literal : public literal, public visitable<bool_literal> {
    bool m_value;

  public:
    /**
     * @brief Creates a new bool literal
     * @param info The source info
     * @param b The value of the literal
     */
    explicit bool_literal(core::source_info info, bool b)
        : literal(kind::literal_bool, std::move(info))
        , m_value(b) {}

    [[nodiscard]] virtual bool is(literal_type type) const {
      return type == literal_type::lit_bool;
    }

    [[nodiscard]] bool value() const { return m_value; }
  };
} // namespace cascade::ast

#endif
