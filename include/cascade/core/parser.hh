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

#include "cascade/ast/ast.hh"
#include "cascade/core/lexer.hh"
#include <cstddef>
#include <memory>
#include <string_view>

namespace cascade::core {
  /** @brief Handles parsing a single file */
  class parser {
    /** @brief Opaque implementation of the parser */
    class impl;

    /** @brief Owning pointer to the implementation */
    std::unique_ptr<impl> m_impl;

  public:
    using return_type = ast::program;

    /**
     * @brief Creates a parser
     * @param source string_view to the entire source code for a file
     */
    explicit parser(lexer::return_type source);

    /** @brief Parses the program */
    [[nodiscard]] return_type parse();
  };
} // namespace cascade::core

#endif