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

#include "cascade/ast/detail/nodes.hh"
#include "cascade/core/lexer.hh"

namespace cascade::ast {
  class identifier : public expression {
    std::string m_name;

  public:
    explicit identifier(core::source_info info, std::string_view name)
        : expression(kind::identifier, std::move(info)), m_name(std::string{name}) {}

    virtual void accept(ast_visitor &visitor) final { visitor.visit(*this); }

    [[nodiscard]] std::string_view name() const { return m_name; }
  };

  class call : public expression {
    std::shared_ptr<expression> m_callee;
    std::vector<std::shared_ptr<expression>> m_args;

  public:
    explicit call(core::source_info info, std::shared_ptr<expression> callee,
        std::vector<std::shared_ptr<expression>> args)
        : expression(kind::expression_call, std::move(info)),
          m_callee(std::move(callee)),
          m_args(std::move(args)) {}

    virtual void accept(ast_visitor &visitor) final { return visitor.visit(*this); }

    [[nodiscard]] std::shared_ptr<expression> callee() const { return m_callee; }

    [[nodiscard]] const std::vector<std::shared_ptr<expression>> &args() const { return m_args; }
  };

  class binary : public expression {
    core::token::kind m_op;
    std::shared_ptr<expression> m_lhs;
    std::shared_ptr<expression> m_rhs;

  public:
    explicit binary(core::source_info info, core::token::kind op, std::shared_ptr<expression> lhs,
        std::shared_ptr<expression> rhs)
        : expression(kind::expression_binary, std::move(info)),
          m_op(op),
          m_lhs(std::move(lhs)),
          m_rhs(std::move(rhs)) {}

    virtual void accept(ast_visitor &visitor) final { return visitor.visit(*this); }

    [[nodiscard]] core::token::kind op() const { return m_op; }

    [[nodiscard]] std::shared_ptr<expression> lhs() const { return m_lhs; }

    [[nodiscard]] std::shared_ptr<expression> rhs() const { return m_rhs; }
  };

  class unary : public expression {
    core::token::kind m_op;
    std::shared_ptr<expression> m_rhs;

  public:
    explicit unary(core::source_info info, core::token::kind op, std::shared_ptr<expression> rhs)
        : expression(kind::expression_unary, std::move(info)), m_op(op), m_rhs(std::move(rhs)) {}

    virtual void accept(ast_visitor &visitor) final { return visitor.visit(*this); }

    [[nodiscard]] core::token::kind op() const { return m_op; }

    [[nodiscard]] std::shared_ptr<expression> rhs() const { return m_rhs; }
  };

  class field_access : public expression {
    std::shared_ptr<expression> m_accessed;
    std::string m_field;

  public:
    explicit field_access(
        core::source_info info, std::shared_ptr<expression> accessed, std::string field)
        : expression(kind::expression_field_access, std::move(info)),
          m_accessed(std::move(accessed)),
          m_field(std::move(field)) {}

    [[nodiscard]] std::shared_ptr<expression> accessed() const { return m_accessed; }

    [[nodiscard]] std::string_view field_name() const { return m_field; }

    virtual void accept(ast_visitor &visitor) final { return visitor.visit(*this); }
  };

  class index : public expression {
    std::shared_ptr<expression> m_array;
    std::shared_ptr<expression> m_index;

  public:
    explicit index(
        core::source_info info, std::shared_ptr<expression> array, std::shared_ptr<expression> idx)
        : expression(kind::expression_index, std::move(info)),
          m_array(std::move(array)),
          m_index(std::move(idx)) {}

    [[nodiscard]] std::shared_ptr<expression> array() const { return m_array; }

    [[nodiscard]] std::shared_ptr<expression> idx() const { return m_index; }

    virtual void accept(ast_visitor &visitor) final { return visitor.visit(*this); }
  };

  class if_else : public expression {
    std::shared_ptr<expression> m_condition;
    std::shared_ptr<expression> m_true;
    std::shared_ptr<expression> m_false;

  public:
    explicit if_else(core::source_info info, std::shared_ptr<expression> cond,
        std::shared_ptr<expression> true_clause, std::shared_ptr<expression> else_clause)
        : expression(kind::expression_if_else, std::move(info)),
          m_condition(std::move(cond)),
          m_true(std::move(true_clause)),
          m_false(std::move(else_clause)) {}

    [[nodiscard]] std::shared_ptr<expression> condition() const { return m_condition; }

    [[nodiscard]] std::shared_ptr<expression> true_clause() const { return m_true; }

    [[nodiscard]] std::shared_ptr<expression> else_clause() const { return m_false; }

    virtual void accept(ast_visitor &visitor) final { return visitor.visit(*this); }
  };

  class block : public expression {
    std::vector<std::shared_ptr<statement>> m_statements;

  public:
    explicit block(core::source_info info, std::vector<std::shared_ptr<statement>> stmts)
        : expression(kind::expression_block, std::move(info)), m_statements(std::move(stmts)) {}

    [[nodiscard]] const std::vector<std::shared_ptr<statement>> &statements() const {
      return m_statements;
    }
  };

  class struct_init : public expression {
  public:
    struct pair {
      std::string field_name;
      std::shared_ptr<expression> value;
    };

  private:
    std::string m_struct_name;
    std::vector<pair> m_init;

  public:
    explicit struct_init(core::source_info info, std::string name, std::vector<pair> inits)
        : expression(kind::expression_struct, std::move(info)),
          m_struct_name(std::move(name)),
          m_init(std::move(inits)) {}

    [[nodiscard]] const std::vector<pair> &pairs() const { return m_init; }

    [[nodiscard]] std::string_view name() const { return m_struct_name; }

    virtual void accept(ast_visitor &visitor) final { return visitor.visit(*this); }
  };
} // namespace cascade::ast

#endif