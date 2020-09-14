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
 * ast/ast_visitor.hh:
 *   Defines the visitor type used to visit AST nodes
 *
 *---------------------------------------------------------------------------*/

#ifndef CASCADE_AST_AST_VISITOR_HH
#define CASCADE_AST_AST_VISITOR_HH

namespace cascade::ast {
  class type_base;
  class const_decl;
  class static_decl;
  class argument;
  class fn;
  class module_decl;
  class import_decl;
  class export_decl;
  class char_literal;
  class string_literal;
  class int_literal;
  class float_literal;
  class bool_literal;
  class identifier;
  class call;
  class binary;
  class unary;
  class field_access;
  class index;
  class if_else;
  class struct_init;
  class block;
  class expression_statement;
  class let;
  class mut;
  class ret;
  class loop;
  class type_decl;

// CASCADE_VISIT_TYPE is left defined intentionally so visitors can use it as well
// the intention is that the implementor defines VISIT(type) and then uses this macro
#define CASCADE_VISIT_TYPES                                                                        \
  VISIT(type_base);                                                                                \
  VISIT(const_decl);                                                                               \
  VISIT(static_decl);                                                                              \
  VISIT(argument);                                                                                 \
  VISIT(fn);                                                                                       \
  VISIT(module_decl);                                                                              \
  VISIT(import_decl);                                                                              \
  VISIT(export_decl);                                                                              \
  VISIT(char_literal);                                                                             \
  VISIT(string_literal);                                                                           \
  VISIT(int_literal);                                                                              \
  VISIT(float_literal);                                                                            \
  VISIT(bool_literal);                                                                             \
  VISIT(identifier);                                                                               \
  VISIT(call);                                                                                     \
  VISIT(binary);                                                                                   \
  VISIT(unary);                                                                                    \
  VISIT(field_access);                                                                             \
  VISIT(index);                                                                                    \
  VISIT(if_else);                                                                                  \
  VISIT(struct_init);                                                                              \
  VISIT(block);                                                                                    \
  VISIT(expression_statement);                                                                     \
  VISIT(let);                                                                                      \
  VISIT(mut);                                                                                      \
  VISIT(ret);                                                                                      \
  VISIT(loop);                                                                                     \
  VISIT(type_decl);

  /** @brief Abstract AST visitor type */
  template <class T> class visitor {
  public:
#define VISIT(type) virtual T visit(type &) = 0

    CASCADE_VISIT_TYPES

#undef VISIT
    virtual ~visitor() {}
  };

  template <> class visitor<void> {
  public:
#define VISIT(type) virtual void visit(type &) = 0

    CASCADE_VISIT_TYPES

#undef VISIT
    virtual ~visitor() {}
  };
} // namespace cascade::ast

#endif
