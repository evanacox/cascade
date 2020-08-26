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
} // namespace cascade::ast

#endif