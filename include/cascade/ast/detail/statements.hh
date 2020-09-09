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
 * ast/detail/statements.hh:
 *   Outlines the statement AST nodes
 *
 *---------------------------------------------------------------------------*/

#ifndef CASCADE_AST_DETAIL_STATEMENTS_HH
#define CASCADE_AST_DETAIL_STATEMENTS_HH

#include "cascade/ast/detail/nodes.hh"
#include "cascade/core/lexer.hh"

namespace cascade::ast {
  /** @brief Simply an expression in place of a statement */
  class expression_statement : public statement {
    std::unique_ptr<expression> m_expr;

  public:
    explicit expression_statement(core::source_info info, std::unique_ptr<expression> expr)
        : statement(kind::statement_expression, std::move(info)), m_expr(std::move(expr)) {}

    [[nodiscard]] expression &expr() const { return *m_expr; }

    virtual void accept(ast_visitor &visitor) final { visitor.visit(*this); }
  };

  class let : public statement {
    std::unique_ptr<expression> m_initializer;
    std::unique_ptr<type_base> m_type;
    std::string m_name;

  public:
    explicit let(core::source_info info, std::unique_ptr<expression> init,
        std::unique_ptr<type_base> type, std::string_view name)
        : statement(kind::statement_let, std::move(info)),
          m_initializer(std::move(init)),
          m_type(std::move(type)),
          m_name(std::string{name}) {}

    [[nodiscard]] expression &initializer() const { return *m_initializer; }

    [[nodiscard]] type_base &type() const { return *m_type; }

    [[nodiscard]] std::string_view name() const { return m_name; }

    virtual void accept(ast_visitor &visitor) final { visitor.visit(*this); }
  };

  class mut : public statement {
    std::unique_ptr<expression> m_initializer;
    std::unique_ptr<type_base> m_type;
    std::string m_name;

  public:
    explicit mut(core::source_info info, std::unique_ptr<expression> init,
        std::unique_ptr<type_base> type, std::string_view name)
        : statement(kind::statement_mut, std::move(info)),
          m_initializer(std::move(init)),
          m_type(std::move(type)),
          m_name(std::string{name}) {}

    [[nodiscard]] expression &initializer() const { return *m_initializer; }

    [[nodiscard]] type_base &type() const { return *m_type; }

    [[nodiscard]] std::string_view name() const { return m_name; }

    virtual void accept(ast_visitor &visitor) final { visitor.visit(*this); }
  };

  class ret : public statement {
    std::optional<std::unique_ptr<expression>> m_return_value;

  public:
    explicit ret(core::source_info info, std::optional<std::unique_ptr<expression>> ret_val)
        : statement(kind::statement_ret, std::move(info)), m_return_value(std::move(ret_val)) {}

    [[nodiscard]] std::optional<std::reference_wrapper<expression>> return_value() const {
      if (m_return_value) {
        return *m_return_value.value();
      }

      return std::nullopt;
    }

    virtual void accept(ast_visitor &visitor) final { return visitor.visit(*this); }
  };

  class loop : public statement {
    std::optional<std::unique_ptr<expression>> m_condition;
    std::unique_ptr<expression> m_body;

  public:
    explicit loop(core::source_info info, std::optional<std::unique_ptr<expression>> condition,
        std::unique_ptr<expression> body)
        : statement(kind::statement_ret, std::move(info)),
          m_condition(std::move(condition)),
          m_body(std::move(body)) {}

    [[nodiscard]] std::optional<std::reference_wrapper<expression>> condition() const {
      if (m_condition) {
        return *m_condition.value();
      }

      return std::nullopt;
    }

    [[nodiscard]] expression &body() const { return *m_body; }

    virtual void accept(ast_visitor &visitor) final { return visitor.visit(*this); }
  };
} // namespace cascade::ast

#endif