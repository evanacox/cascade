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
 * core/parser.cc:
 *   Implements the main parser for the program
 *
 *---------------------------------------------------------------------------*/

#include "cascade/core/parser.hh"

using namespace cascade;
using namespace core;

using expr_ptr = std::shared_ptr<ast::expression>;

using stmt_ptr = std::shared_ptr<ast::statement>;

using decl_ptr = std::shared_ptr<ast::declaration>;

class parser::impl {
  lexer::return_type m_toks;

  lexer::return_type::const_iterator m_it;

  [[nodiscard]] token &previous() const;

  [[nodiscard]] token &current() const;

  [[nodiscard]] std::optional<token> next() const;

  token consume();

  [[nodiscard]] bool is_at_end() const;

  [[nodiscard]] expr_ptr grouping();

  [[nodiscard]] expr_ptr primary();

  [[nodiscard]] expr_ptr call();

  [[nodiscard]] expr_ptr unary();

  [[nodiscard]] expr_ptr multiplication();

  [[nodiscard]] expr_ptr addition();

  [[nodiscard]] expr_ptr bitshift();

  [[nodiscard]] expr_ptr bitwise_and();

  [[nodiscard]] expr_ptr bitwise_xor();

  [[nodiscard]] expr_ptr bitwise_or();

  [[nodiscard]] expr_ptr relational();

  [[nodiscard]] expr_ptr logical_not();

  [[nodiscard]] expr_ptr logical_and();

  [[nodiscard]] expr_ptr logical_xor();

  [[nodiscard]] expr_ptr logical_or();

  [[nodiscard]] expr_ptr if_then();

  [[nodiscard]] expr_ptr assignment();

  [[nodiscard]] expr_ptr expression();

  [[nodiscard]] stmt_ptr statement();

  [[nodiscard]] decl_ptr declaration();

public:
  explicit impl(lexer::return_type tokens) : m_toks(std::move(tokens)) {}

  parser::return_type parse();
};

parser::return_type parser::impl::parse() {
  parser::return_type program;

  while (!is_at_end()) {
    program.add_declaration(declaration());
  }

  return program;
}

parser::parser(lexer::return_type toks) : m_impl(std::make_unique<parser::impl>(std::move(toks))) {}

parser::return_type parser::parse() { return m_impl->parse(); }