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
 * core/typechecker.hh:
 *   Outlines the typechecker's public interface
 *
 *---------------------------------------------------------------------------*/

#ifndef CASCADE_CORE_TYPECHECKER_HH
#define CASCADE_CORE_TYPECHECKER_HH

#include "ast/ast.hh"
#include <memory>
#include <utility>

namespace cascade::core {
  /** @brief Represents the typechecker, handles the symbol table, and resolving types */
  class typechecker {
    /** @brief Opaque implementation class */
    class impl;

    /** @brief Pointer to the implementation */
    std::unique_ptr<impl> m_impl;

  public:
    /**
     * @brief Creates a typechecker
     * @param programs The list of programs to typecheck
     */
    explicit typechecker(const std::vector<ast::program> &programs,
        std::function<void(std::unique_ptr<errors::error>)> report);
  };
} // namespace cascade::core

#endif
