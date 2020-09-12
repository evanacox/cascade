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
 * ast/detail/expressions.hh:
 *   Outlines the expression AST nodes
 *
 *---------------------------------------------------------------------------*/

#ifndef CASCADE_AST_DETAIL_EXPRESSIONS_HH
#define CASCADE_AST_DETAIL_EXPRESSIONS_HH

#include "ast/detail/nodes.hh"
#include "ast/detail/types.hh"
#include "core/lexer.hh"

namespace cascade::ast {
  class identifier : public expression, public visitable<identifier> {
    std::string m_name;

  public:
    explicit identifier(core::source_info info, std::string_view name)
        : expression(kind::identifier, std::move(info))
        , m_name(std::string{name}) {}

    [[nodiscard]] std::string_view name() const { return m_name; }
  };

  class call : public expression, public visitable<call> {
    std::unique_ptr<expression> m_callee;
    std::vector<std::unique_ptr<expression>> m_args;

  public:
    explicit call(core::source_info info,
        std::unique_ptr<expression> callee,
        std::vector<std::unique_ptr<expression>> args)
        : expression(kind::expression_call, std::move(info))
        , m_callee(std::move(callee))
        , m_args(std::move(args)) {}

    [[nodiscard]] expression &callee() const { return *m_callee; }

    [[nodiscard]] const std::vector<std::unique_ptr<expression>> &args() const { return m_args; }
  };

  class binary : public expression, public visitable<binary> {
    core::token::kind m_op;
    std::unique_ptr<expression> m_lhs;
    std::unique_ptr<expression> m_rhs;

  public:
    explicit binary(core::source_info info,
        core::token::kind op,
        std::unique_ptr<expression> lhs,
        std::unique_ptr<expression> rhs)
        : expression(kind::expression_binary, std::move(info))
        , m_op(op)
        , m_lhs(std::move(lhs))
        , m_rhs(std::move(rhs)) {}

    [[nodiscard]] core::token::kind op() const { return m_op; }

    [[nodiscard]] expression &lhs() const { return *m_lhs; }

    [[nodiscard]] expression &rhs() const { return *m_rhs; }
  };

  class unary : public expression, public visitable<unary> {
    core::token::kind m_op;
    std::unique_ptr<expression> m_rhs;

  public:
    explicit unary(core::source_info info, core::token::kind op, std::unique_ptr<expression> rhs)
        : expression(kind::expression_unary, std::move(info))
        , m_op(op)
        , m_rhs(std::move(rhs)) {}

    [[nodiscard]] core::token::kind op() const { return m_op; }

    [[nodiscard]] expression &rhs() const { return *m_rhs; }
  };

  class field_access : public expression, public visitable<field_access> {
    std::unique_ptr<expression> m_accessed;
    std::string m_field;

  public:
    explicit field_access(core::source_info info,
        std::unique_ptr<expression> accessed,
        std::string field)
        : expression(kind::expression_field_access, std::move(info))
        , m_accessed(std::move(accessed))
        , m_field(std::move(field)) {}

    [[nodiscard]] expression &accessed() const { return *m_accessed; }

    [[nodiscard]] std::string_view field_name() const { return m_field; }
  };

  class index : public expression, public visitable<index> {
    std::unique_ptr<expression> m_array;
    std::unique_ptr<expression> m_index;

  public:
    explicit index(core::source_info info,
        std::unique_ptr<expression> array,
        std::unique_ptr<expression> idx)
        : expression(kind::expression_index, std::move(info))
        , m_array(std::move(array))
        , m_index(std::move(idx)) {}

    [[nodiscard]] expression &array() const { return *m_array; }

    [[nodiscard]] expression &idx() const { return *m_index; }
  };

  class if_else : public expression, public visitable<if_else> {
    std::unique_ptr<expression> m_condition;
    std::unique_ptr<expression> m_true;
    std::optional<std::unique_ptr<expression>> m_false;

  public:
    explicit if_else(core::source_info info,
        std::unique_ptr<expression> cond,
        std::unique_ptr<expression> true_clause,
        std::optional<std::unique_ptr<expression>> else_clause = std::nullopt)
        : expression(kind::expression_if_else, std::move(info))
        , m_condition(std::move(cond))
        , m_true(std::move(true_clause))
        , m_false(std::move(else_clause)) {}

    [[nodiscard]] expression &condition() const { return *m_condition; }

    [[nodiscard]] expression &true_clause() const { return *m_true; }

    [[nodiscard]] std::optional<std::reference_wrapper<expression>> else_clause() const {
      if (m_false) {
        return *m_false.value();
      }

      return std::nullopt;
    }
  };

  class block : public expression, public visitable<block> {
    std::vector<std::unique_ptr<statement>> m_statements;

    std::unique_ptr<type_base> m_return_type;

  public:
    explicit block(core::source_info info,
        std::vector<std::unique_ptr<statement>> stmts,
        std::unique_ptr<type_base> type)
        : expression(kind::expression_block, std::move(info))
        , m_statements(std::move(stmts))
        , m_return_type(std::move(type)) {}

    [[nodiscard]] const std::vector<std::unique_ptr<statement>> &statements() const {
      return m_statements;
    }

    [[nodiscard]] type_base &type() const { return *m_return_type; }
  };

  class struct_init : public expression, public visitable<struct_init> {
  public:
    struct pair {
      std::string field_name;
      std::unique_ptr<expression> value;
    };

  private:
    std::string m_struct_name;
    std::vector<pair> m_init;

  public:
    explicit struct_init(core::source_info info, std::string name, std::vector<pair> inits)
        : expression(kind::expression_struct, std::move(info))
        , m_struct_name(std::move(name))
        , m_init(std::move(inits)) {}

    [[nodiscard]] const std::vector<pair> &pairs() const { return m_init; }

    [[nodiscard]] std::string_view name() const { return m_struct_name; }
  };
} // namespace cascade::ast

#endif
