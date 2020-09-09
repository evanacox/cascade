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
 * util/keywords.hh:
 *   Defines some helper functions for converting raw strings to enum classes,
 *   and back again
 *
 *---------------------------------------------------------------------------*/

#ifndef CASCADE_UTIL_KEYWORDS_HH
#define CASCADE_UTIL_KEYWORDS_HH

#include "core/lexer.hh"

namespace cascade::util {
  /**
   * @brief Returns whether or not a string is convertible to a token::kind
   * @return True if it is, false otherwise
   */
  bool is_kind(std::string_view raw);

  /**
   * @brief Returns a token::kind for a raw string if possible
   * @details Throws an std::exception derivative if the string isn't
   * found in the symbol mappings or keyword mappings. This shouldn't
   * be caught, just check is_kind() beforehand.
   * @return The kind of token it is
   */
  core::token::kind kind_from_string(std::string_view raw);

  /**
   * @brief Returns a string_view from a token::kind
   * @return The string representation of a token
   */
  std::string_view string_from_kind(core::token::kind kind);
} // namespace cascade::util

#endif
