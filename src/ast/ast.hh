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

  template <class T> T node::accept(visitor<T> &visitor) {
    // considering it gets turned into a jump table even in -O0
    // I think we're ok
    switch (m_type) {
      case kind::literal_char:
        return reinterpret_cast<char_literal &>(*this).visit_accept(visitor);
      case kind::literal_string:
        return reinterpret_cast<string_literal &>(*this).visit_accept(visitor);
      case kind::literal_number:
        return reinterpret_cast<int_literal &>(*this).visit_accept(visitor);
      case kind::literal_bool:
        return reinterpret_cast<bool_literal &>(*this).visit_accept(visitor);
      case kind::literal_float:
        return reinterpret_cast<float_literal &>(*this).visit_accept(visitor);
      case kind::identifier:
        return reinterpret_cast<identifier &>(*this).visit_accept(visitor);
      case kind::declaration_const:
        return reinterpret_cast<const_decl &>(*this).visit_accept(visitor);
      case kind::declaration_static:
        return reinterpret_cast<static_decl &>(*this).visit_accept(visitor);
      case kind::declaration_fn:
        return reinterpret_cast<fn &>(*this).visit_accept(visitor);
      case kind::declaration_struct:
        throw std::logic_error{"wtf"};
      case kind::declaration_module:
        return reinterpret_cast<module_decl &>(*this).visit_accept(visitor);
      case kind::declaration_import:
        return reinterpret_cast<import_decl &>(*this).visit_accept(visitor);
      case kind::declaration_export:
        return reinterpret_cast<export_decl &>(*this).visit_accept(visitor);
      case kind::declaration_argument:
        return reinterpret_cast<argument &>(*this).visit_accept(visitor);
      case kind::declaration_type:
        return reinterpret_cast<type_decl &>(*this).visit_accept(visitor);
      case kind::expression_call:
        return reinterpret_cast<call &>(*this).visit_accept(visitor);
      case kind::expression_binary:
        return reinterpret_cast<binary &>(*this).visit_accept(visitor);
      case kind::expression_unary:
        return reinterpret_cast<unary &>(*this).visit_accept(visitor);
      case kind::expression_field_access:
        return reinterpret_cast<field_access &>(*this).visit_accept(visitor);
      case kind::expression_index:
        return reinterpret_cast<index &>(*this).visit_accept(visitor);
      case kind::expression_if_else:
        return reinterpret_cast<if_else &>(*this).visit_accept(visitor);
      case kind::expression_block:
        return reinterpret_cast<block &>(*this).visit_accept(visitor);
      case kind::expression_array:
        throw std::logic_error{"unimplemented"};
      case kind::expression_struct:
        return reinterpret_cast<struct_init &>(*this).visit_accept(visitor);
      case kind::statement_expression:
        return reinterpret_cast<expression_statement &>(*this).visit_accept(visitor);
      case kind::statement_let:
        return reinterpret_cast<let &>(*this).visit_accept(visitor);
      case kind::statement_mut:
        return reinterpret_cast<mut &>(*this).visit_accept(visitor);
      case kind::statement_ret:
        return reinterpret_cast<ret &>(*this).visit_accept(visitor);
      case kind::type:
      case kind::type_implied:
      case kind::type_void:
        return reinterpret_cast<type &>(*this).visit_accept(visitor);
    }
  }
} // namespace cascade::ast

#endif
