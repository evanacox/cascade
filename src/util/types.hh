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
#include <string>
#include <utility>

namespace cascade::util {
  /**
   * @brief Turns the type into a string, function used throughout codebase for consistency
   * @param node The type to transform
   * @return A string of the type
   */
  std::string to_string(const ast::type_data &node);

  /**
   * @brief Hashes a type AST object
   * @param node The node to hash
   */
  std::size_t hash(const ast::type_data &node);
} // namespace cascade::util

namespace std {
  template <> struct hash<cascade::ast::type_data> {
    std::size_t operator()(const cascade::ast::type_data &node) {
      return cascade::util::hash(node);
    }
  };
} // namespace std
