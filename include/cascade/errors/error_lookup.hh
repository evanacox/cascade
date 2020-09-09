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
 * errors/error_lookup.hh:
 *   Defines the error codes/error messages/structure of error messages
 *
 *---------------------------------------------------------------------------*/

#ifndef CASCADE_ERRORS_ERROR_LOOKUP_HH
#define CASCADE_ERRORS_ERROR_LOOKUP_HH

#include <optional>
#include <string_view>

namespace cascade::errors {
  enum class error_code : short {
    unknown_char = 1,
    unterminated_str,
    unterminated_char,
    unexpected_tok,
    unterminated_block_comment,
    number_literal_too_large,
    unclosed_paren,
    expected_expression,
    unexpected_expression,
    expected_semi,
    expected_else_after_then,
    invalid_char_literal,
    unmatched_brace,
    unexpected_end_of_input,
    expected_comma,
    expected_closing_bracket,
    expected_opening_brace,
    expected_type,
    expected_identifier,
    expected_declaration,
    cannot_export_export,
    duplicate_module,
  };

  /**
   * @brief Turns an error code into an error message
   * @param code The code to get
   * @return An error message
   */
  std::string_view error_message_from_code(error_code code);

  /**
   * @brief Provides a "note" message for an error code, if there is one.
   * @param code The code to get
   * @return The note message
   */
  std::optional<std::string_view> error_note_from_code(error_code code);
} // namespace cascade::errors

#endif
