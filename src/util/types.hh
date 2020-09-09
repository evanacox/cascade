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
 * util/types.hh:
 *   Declares some helper functions for working on AST types
 *
 *---------------------------------------------------------------------------*/

#include "ast/ast.hh"
#include <utility>

namespace cascade::util {
  /**
   * @brief Utility function for traversing a type
   * @param node The original type node
   * @param ptr_fn The function to call on a pointer
   * @param ref_fn The function to call on a reference
   * @param array_fn The function to call on arrays
   * @param builtin_fn The function to call on builtins
   * @param userdef_fn The function to call on user-def types
   */
  void traverse_type(ast::type_base &node, std::function<void(ast::pointer &)> ptr_fn,
      std::function<void(ast::reference &)> ref_fn, std::function<void(ast::array &)> array_fn,
      std::function<void(ast::builtin &)> builtin_fn,
      std::function<void(ast::user_defined &)> userdef_fn);
} // namespace cascade::util
