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
  class type;
  class const_decl;
  class argument;
  class fn;
  class module_decl;
  class import_decl;
  class export_decl;

#define VISIT(type) virtual void visit(type &) = 0

  /** @brief Abstract AST visitor type */
  class ast_visitor {
  public:
    VISIT(type);
    VISIT(const_decl);
    VISIT(argument);
    VISIT(fn);
    VISIT(module_decl);
    VISIT(import_decl);
    VISIT(export_decl);

    virtual ~ast_visitor() {}
  };

#undef VISIT
} // namespace cascade::ast

#endif