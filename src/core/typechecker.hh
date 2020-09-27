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
  using report_fn = std::function<void(std::unique_ptr<errors::error>)>;

  /**
   * @brief Typechecks a list of programs
   * @param programs All the modules to attempt to combine
   * @param report The function to call for each error
   */
  bool typecheck(std::vector<ast::program> &programs,
      const std::vector<std::string_view> &files,
      report_fn report);
} // namespace cascade::core

#endif
