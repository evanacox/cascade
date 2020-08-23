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
 * ast/detail/nodes.hh:
 *   Outlines the abstract AST base nodes
 *
 *---------------------------------------------------------------------------*/

#ifndef CASCADE_AST_DETAIL_NODES_HH
#define CASCADE_AST_DETAIL_NODES_HH

#include "cascade/core/lexer.hh"

namespace cascade::ast {
  /** @brief Abstract visitor type */
  class ast_visitor;

  /** @brief A type of node */
  enum class kind {
    literal_char,
    literal_string,
    literal_number,
    literal_bool,
    identifier,
    type_ptr,
    type_ref,
    type_array,
    type_builtin,
    type_userdef,
    declaration_const,
    declaration_static,
    declaration_fn,
    declaration_struct,
    declaration_module,
    declaration_import,
  };

  /** @brief Abstract base node type */
  class node {
  protected:
    /** @brief Source mapping for the node */
    core::source_info m_info;

    /** @brief The kind of node */
    kind m_type;

  public:
    /**
     * @brief Initializes the base node
     * @param type The type of node it is
     * @param info The source mappings
     */
    explicit node(kind type, core::source_info info) : m_info(info), m_type(type) {}

    /**
     * @brief Accepts a visitor to the node
     * @param visitor The visitor to accept
     */
    virtual void accept(ast_visitor &visitor) = 0;

    /**
     * @brief Returns if the node is of @p type
     * @param type The type to check for
     * @return If @p type is the token's type
     */
    [[nodiscard]] bool is(kind type) const { return this->m_type == type; }

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
     * @brief Returns if the node is an expression node
     * @return If the node is one of the expression types
     */
    [[nodiscard]] virtual bool is_expression() const = 0;

    /**
     * @brief Returns if the node is a top-level declaration
     * @return If the node is a top-level decl
     */
    [[nodiscard]] virtual bool is_declaration() const = 0;

    /**
     * @brief Returns if the node is a statement
     * @return If the node is a statement
     */
    [[nodiscard]] virtual bool is_statement() const = 0;

    /** @brief Virtual destructor */
    virtual ~node(){};
  };

  /** @brief Tag type for top-level declarations, also defines the is_*() methods */
  class declaration : public node {
  public:
    explicit declaration(kind type, core::source_info info)
        : node(std::move(type), std::move(info)) {}

    [[nodiscard]] virtual bool is_expression() const final { return false; }

    [[nodiscard]] virtual bool is_declaration() const final { return true; }

    [[nodiscard]] virtual bool is_statement() const final { return false; }
  };

  /** @brief Tag type for statements, defines the is_*() methods */
  class statement : public node {
  public:
    explicit statement(kind type, core::source_info info)
        : node(std::move(type), std::move(info)) {}

    [[nodiscard]] virtual bool is_expression() const final { return false; }

    [[nodiscard]] virtual bool is_declaration() const final { return false; }

    [[nodiscard]] virtual bool is_statement() const final { return true; }
  };

  /** @brief Tag type for expressions, defines the is_*() methods */
  class expression : public node {
  public:
    explicit expression(kind type, core::source_info info)
        : node(std::move(type), std::move(info)) {}

    [[nodiscard]] virtual bool is_expression() const final { return true; }

    [[nodiscard]] virtual bool is_declaration() const final { return false; }

    [[nodiscard]] virtual bool is_statement() const final { return false; }
  };

  /** @brief Tag type for types, defines the is_*() methods */
  class type : public node {
  public:
    explicit type(kind t, core::source_info info) : node(std::move(t), std::move(info)) {}

    [[nodiscard]] virtual bool is_expression() const final { return false; }

    [[nodiscard]] virtual bool is_declaration() const final { return false; }

    [[nodiscard]] virtual bool is_statement() const final { return false; }
  };
} // namespace cascade::ast

#endif