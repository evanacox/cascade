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

#include "ast/visitor.hh"
#include "core/lexer.hh"

namespace cascade::ast {
  /** @brief A type of node */
  enum class kind {
    literal_char,
    literal_string,
    literal_number,
    literal_bool,
    literal_float,
    identifier,
    type_ptr,
    type_ref,
    type_array,
    type_builtin,
    type_userdef,
    type_implied,
    type_void,
    declaration_const,
    declaration_static,
    declaration_fn,
    declaration_struct,
    declaration_module,
    declaration_import,
    declaration_export,
    declaration_argument,
    declaration_type,
    expression_call,
    expression_binary,
    expression_unary,
    expression_field_access,
    expression_index,
    expression_if_else,
    expression_block,
    expression_array,
    expression_struct,
    statement_expression,
    statement_let,
    statement_mut,
    statement_ret,
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

    template <class T> T accept(visitor<T> &visitor);

    /** @brief Returns the node's type */
    [[nodiscard]] kind raw_kind() const { return m_type; }

    /** @brief Returns a reference to the node's source mapping, for debug
     * purposes */
    [[nodiscard]] const core::source_info &info() const { return m_info; }

    /**
     * @brief Returns if the node is of @p type
     * @param type The type to check for
     */
    [[nodiscard]] bool is(kind type) const { return this->m_type == type; }

    /**
     * @brief Returns if the token is not of @p type, it's effectively
     * an !token.is(type)
     * @param type The type to check against
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

    /** @brief Returns if the node is an expression node */
    [[nodiscard]] virtual bool is_expression() const = 0;

    /** @brief Returns if the node is a top-level declaration */
    [[nodiscard]] virtual bool is_declaration() const = 0;

    /** @brief Returns if the node is a statement */
    [[nodiscard]] virtual bool is_statement() const = 0;

    virtual ~node(){};
  };

  /** @brief "visitable" mixin for the AST types */
  template <class T> class visitable {
  public:
    /**
     * @brief Standard visitor method for void
     * @param visitor The visitor to accept
     */
    void visit_accept(visitor<void> &visitor) {
      // each type inheriting from this becomes visitable
      return visitor.visit(static_cast<T &>(*this));
    }

    /**
     * @brief Templated accept method for work with visitors with return values
     * @param visitor The visitor to accept
     * @return The result of the visit method
     */
    template <class R> R visit_accept(visitor<R> &visitor) {
      return visitor.visit(static_cast<T &>(*this));
    }
  };

  /** @brief Tag type for top-level declarations, also defines the is_*()
   * methods */
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
  class type_base : public node, public visitable<type_base> {
  public:
    explicit type_base(kind t, core::source_info info) : node(std::move(t), std::move(info)) {}

    [[nodiscard]] virtual bool is_expression() const final { return false; }

    [[nodiscard]] virtual bool is_declaration() const final { return false; }

    [[nodiscard]] virtual bool is_statement() const final { return false; }

    // the objects representing types are recursive, a visitor would need to
    // figure out the type anyway.
  };
} // namespace cascade::ast

#endif
