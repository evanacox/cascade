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
 * core/parser.hh:
 *   Defines the `parser` class that handles parsing files into ASTs.
 *
 *---------------------------------------------------------------------------*/

#ifndef CASCADE_CORE_PARSER_HH
#define CASCADE_CORE_PARSER_HH

#include "ast/ast.hh"
#include "core/lexer.hh"
#include "errors/error.hh"
#include <cstddef>
#include <memory>
#include <string_view>
#include <utility>

namespace cascade::core {
  /**
   * @brief Parses a program
   * @param source List of tokens for a file
   * @param report The function that gets called on any errors
   * @return An AST
   */
  ast::program parse(
      lexer::return_type source, std::function<void(std::unique_ptr<errors::error>)> report);
} // namespace cascade::core

#endif
