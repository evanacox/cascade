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

#include "detail/declarations.hh"
#include "detail/expressions.hh"
#include "detail/literals.hh"
#include "detail/nodes.hh"
#include "detail/statements.hh"
#include "detail/types.hh"
#include "util/mixins.hh"

namespace cascade::ast {
  class program : util::noncopyable {
    std::vector<std::unique_ptr<declaration>> m_decls;

  public:
    explicit program(std::vector<std::unique_ptr<declaration>> &&decls)
        : m_decls(std::move(decls)) {}

    [[nodiscard]] std::vector<std::unique_ptr<declaration>> &decls() { return m_decls; }
  };

  template <class T> auto node::accept(ast_visitor<T> &visitor) {
    // considering it gets turned into a jump table even in -O0
    // I think we're ok
    switch (m_type) {
      case kind::literal_char:
        reinterpret_cast<char_literal &>(*this).visit_accept(visitor);
        break;
      case kind::literal_string:
        reinterpret_cast<string_literal &>(*this).visit_accept(visitor);
        break;
      case kind::literal_number:
        reinterpret_cast<int_literal &>(*this).visit_accept(visitor);
        break;
      case kind::literal_bool:
        reinterpret_cast<bool_literal &>(*this).visit_accept(visitor);
        break;
      case kind::literal_float:
        reinterpret_cast<float_literal &>(*this).visit_accept(visitor);
        break;
      case kind::identifier:
        reinterpret_cast<identifier &>(*this).visit_accept(visitor);
        break;
      case kind::declaration_const:
        reinterpret_cast<const_decl &>(*this).visit_accept(visitor);
        break;
      case kind::declaration_static:
        reinterpret_cast<static_decl &>(*this).visit_accept(visitor);
        break;
      case kind::declaration_fn:
        reinterpret_cast<fn &>(*this).visit_accept(visitor);
        break;
      case kind::declaration_struct:
        throw std::logic_error{"wtf"};
        break;
      case kind::declaration_module:
        reinterpret_cast<module_decl &>(*this).visit_accept(visitor);
        break;
      case kind::declaration_import:
        reinterpret_cast<import_decl &>(*this).visit_accept(visitor);
        break;
      case kind::declaration_export:
        reinterpret_cast<export_decl &>(*this).visit_accept(visitor);
        break;
      case kind::declaration_argument:
        reinterpret_cast<argument &>(*this).visit_accept(visitor);
        break;
      case kind::declaration_type:
        reinterpret_cast<type_decl &>(*this).visit_accept(visitor);
        break;
      case kind::expression_call:
        reinterpret_cast<call &>(*this).visit_accept(visitor);
        break;
      case kind::expression_binary:
        reinterpret_cast<binary &>(*this).visit_accept(visitor);
        break;
      case kind::expression_unary:
        reinterpret_cast<unary &>(*this).visit_accept(visitor);
        break;
      case kind::expression_field_access:
        reinterpret_cast<field_access &>(*this).visit_accept(visitor);
        break;
      case kind::expression_index:
        reinterpret_cast<index &>(*this).visit_accept(visitor);
        break;
      case kind::expression_if_else:
        reinterpret_cast<if_else &>(*this).visit_accept(visitor);
        break;
      case kind::expression_block:
        reinterpret_cast<block &>(*this).visit_accept(visitor);
        break;
      case kind::expression_array:
        reinterpret_cast<array &>(*this).visit_accept(visitor);
        break;
      case kind::expression_struct:
        reinterpret_cast<struct_init &>(*this).visit_accept(visitor);
        break;
      case kind::statement_expression:
        reinterpret_cast<expression_statement &>(*this).visit_accept(visitor);
        break;
      case kind::statement_let:
        reinterpret_cast<let &>(*this).visit_accept(visitor);
        break;
      case kind::statement_mut:
        reinterpret_cast<mut &>(*this).visit_accept(visitor);
        break;
      case kind::statement_ret:
        reinterpret_cast<ret &>(*this).visit_accept(visitor);
        break;
      case kind::type_ptr:
      case kind::type_ref:
      case kind::type_array:
      case kind::type_builtin:
      case kind::type_userdef:
      case kind::type_implied:
      case kind::type_void:
        reinterpret_cast<type_base &>(*this).visit_accept(visitor);
        break;
    }
  }
} // namespace cascade::ast

#endif
