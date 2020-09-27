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
 * ast/detail/types.hh:
 *   Outlines the declaration AST nodes
 *
 *---------------------------------------------------------------------------*/

#ifndef CASCADE_AST_DETAIL_TYPES_HH
#define CASCADE_AST_DETAIL_TYPES_HH

#include "ast/detail/nodes.hh"
#include "core/lexer.hh"
#include <cassert>
#include <deque>
#include <memory>

namespace cascade::ast {
  /**
   * @brief Internal structure that encodes only the type, used in the typechecker
   * Makes it easy to modify type attributes (e.g remove/add pointer, reference, promotions, casts,
   * etc)
   */
  class type_data {
  public:
    /** @brief Any modifiers on a base type */
    enum class type_modifiers : char { ref, mut_ref, ptr, mut_ptr, array };

    /** @brief */
    enum class type_base {
      boolean,
      integer,
      unsigned_integer,
      floating_point,
      user_defined,
      implied,
      void_type,
      error_type,
    };

  private:
    /** @brief The actual stream of modifiers on the type */
    std::deque<type_modifiers> m_modifiers;

    /** @brief The actual "type" of a thing, e.g "bool" in "*mut bool" */
    type_base m_base;

    /**
     * @brief Represents either the precision of the builtin (if m_type ends as a builtin)
     * or the name of a userdef
     */
    std::variant<std::size_t, std::string> m_data;

  public:
    /**
     * @brief Creates a builtin type of @p precision
     * @param modifs The modifiers for the type
     * @param base The builtin type
     * @param precision The precision of the builtin
     */
    type_data(std::deque<type_modifiers> modifs, type_base base, std::size_t precision)
        : m_modifiers(std::move(modifs))
        , m_base(base)
        , m_data() {
      m_data.emplace<std::size_t>(precision);
    }

    type_data(std::deque<type_modifiers> modifs, type_base base, std::string name)
        : m_modifiers(std::move(modifs))
        , m_base(base)
        , m_data() {
      m_data.emplace<std::string>(std::move(name));
    }

    /** @brief Returns a mutable reference to the type modifiers */
    [[nodiscard]] std::deque<type_modifiers> &modifiers() { return m_modifiers; }

    /** @brief Returns a mutable reference to the base type */
    [[nodiscard]] type_base &base() { return m_base; }

    /** @brief Returns a mutable reference to the raw data variant */
    [[nodiscard]] std::variant<std::size_t, std::string> &data() { return m_data; }

    /** @brief Returns a const reference to the type modifiers */
    [[nodiscard]] const std::deque<type_modifiers> &modifiers() const { return m_modifiers; }

    /** @brief Returns a const reference to the base type */
    [[nodiscard]] const type_base &base() const { return m_base; }

    /** @brief Returns a const reference to the raw data variant */
    [[nodiscard]] const std::variant<std::size_t, std::string> &data() const { return m_data; }

    /** @brief Gets the data as a size_t */
    [[nodiscard]] std::size_t precision() const { return std::get<std::size_t>(m_data); }

    /** @brief Gets the data as an std::string */
    [[nodiscard]] std::string name() const { return std::get<std::string>(m_data); }

    /**
     * @brief Performs memberwise equality on two type_data objects
     * @param other The other type_data to compare against
     * @return True if they're equal
     */
    bool operator==(const type_data &other) const {
      // <error-type> is only given when an error is reported,
      // it's used to prevent cascading errors if an expression with
      // an error type is used inside a larger thing
      if (is(type_base::error_type)) {
        return true;
      }

      if (other.is(type_base::error_type)) {
        return true;
      }

      return modifiers() == other.modifiers() && data() == other.data() && base() == other.base();
    }

    /**
     * @brief Performs memberwise equality on two type_data objects
     * @param other The other type_data to compare against
     * @return True if they're not equal
     */
    bool operator!=(const type_data &other) const { return !(*this == other); }

    /**
     * @brief Checks if a type is of @p type
     * @param type The type to check against
     * @return If the type is of @p type for a base
     */
    [[nodiscard]] bool is(type_base type) const { return m_base == type; }

    /**
     * @brief Checks if a type is not of @p type
     * @param type The type to check against
     * @return If the base type is not @p type
     */
    [[nodiscard]] bool is_not(type_base type) const { return !is(type); }

    /**
     * @brief Checks if a type is of @p type or one of @p rest is the base type
     * @param type The type to check against
     * @param rest The rest of the types to check against
     * @return If the base type is @p type or is included in @p rest
     */
    template <class... Rest>[[nodiscard]] bool is_one_of(type_base first, Rest... rest) const {
      if (is(first)) {
        return true;
      }

      if constexpr (sizeof...(rest) > 0) {
        return is_one_of(std::forward<type_base>(rest...));
      }

      return false;
    }

    /**
     * @brief Returns if the type is one of the builtin types
     * @return True if the type is one of the builtin types
     */
    [[nodiscard]] bool is_builtin() const {
      return is_not(type_base::implied) && is_not(type_base::void_type)
             && is_not(type_base::user_defined);
    }

    /**
     * @brief Returns if the type is an error_type
     * @return True if the type is an error_type
     */
    [[nodiscard]] bool is_error() const { return is(type_base::error_type); }
  };

  /** @brief A simple class holding a type */
  class type : public node, public visitable<type> {
  public:
    using type_modifiers = type_data::type_modifiers;
    using type_base = type_data::type_base;

  private:
    type_data m_type;

  protected:
    // ctor that the implied/void types can use
    explicit type(kind type, core::source_info info, type_base base)
        : node(type, std::move(info))
        , m_type({}, base, 0) {}

  public:
    /**
     * @brief Creates a new **builtin** type
     * @param info The source info with the location of the type
     * @param mods Any modifiers on the type (e.g &mut, *, [])
     * @param base The "base" type (e.g bool, integer, float)
     * @param precision The precision of the builtin type
     */
    explicit type(core::source_info info,
        std::deque<type_modifiers> mods,
        type_base base,
        std::size_t precision)
        : node(kind::type, std::move(info))
        , m_type(std::move(mods), base, std::move(precision)) {}

    /**
     * @brief Creates a new **user defined** type
     * @param info The source info with the location of the type
     * @param mods Any modifiers on the type (e.g &mut, *, [])
     * @param name The name of the userdefined type
     */
    explicit type(core::source_info info, std::deque<type_modifiers> mods, std::string name)
        : node(kind::type, std::move(info))
        , m_type(std::move(mods), type_base::user_defined, std::move(name)) {}

    [[nodiscard]] virtual bool is_expression() const final { return false; }

    [[nodiscard]] virtual bool is_declaration() const final { return false; }

    [[nodiscard]] virtual bool is_statement() const final { return false; }

    /** @brief Returns the type's information */
    [[nodiscard]] type_data &data() { return m_type; }

    [[nodiscard]] const type_data &data() const { return m_type; }
  };

  /** @brief Serves as a marker for a type that the user left implied */
  class implied : public type {
  public:
    /**
     * @brief Creates an implied instance
     * @param info The source info for the location where the type **would be**.
     */
    explicit implied(core::source_info info)
        : type(kind::type_implied, std::move(info), type_base::implied) {}
  };

  /** @brief Serves as a marker for something that doesn't really *have* a type */
  class void_type : public type {
  public:
    /**
     * @brief Creates a void_type instance
     * @param info The source info for the location where the type **would be**.
     */
    explicit void_type(core::source_info info)
        : type(kind::type_void, std::move(info), type_base::void_type) {}
  };
} // namespace cascade::ast

#endif
