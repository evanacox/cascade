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
 * ast/ast.hh:
 *   Master header file for the AST, includes all the needed headers
 *
 *---------------------------------------------------------------------------*/

#ifndef CASCADE_AST_AST_HH
#define CASCADE_AST_AST_HH

#include "cascade/ast/detail/declarations.hh"
#include "cascade/ast/detail/expressions.hh"
#include "cascade/ast/detail/literals.hh"
#include "cascade/ast/detail/nodes.hh"
#include "cascade/ast/detail/statements.hh"
#include "cascade/ast/detail/types.hh"
#include "cascade/util/mixins.hh"

namespace cascade::ast {
  class program : util::noncopyable {
    std::vector<std::unique_ptr<declaration>> m_decls;

  public:
    explicit program(std::vector<std::unique_ptr<declaration>> &&decls)
        : m_decls(std::move(decls)) {}

    [[nodiscard]] std::vector<std::unique_ptr<declaration>> &decls() { return m_decls; }
  };
} // namespace cascade::ast

#endif
