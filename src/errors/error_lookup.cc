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
 * errors/error_lookup.cc:
 *   Defines the error codes/error messages/structure of error messages
 *
 *---------------------------------------------------------------------------*/

#include "cascade/errors/error_lookup.hh"
#include <string_view>
#include <unordered_map>

using namespace cascade;
using namespace errors;

using ec = error_code;

static std::unordered_map<error_code, std::string_view> errs{
    {ec::unknown_char, "unknown character"},
    {ec::unterminated_str, "unterminated string literal"},
    {ec::unterminated_char, "unterminated character literal"},
    {ec::unexpected_tok, "unexpected token"},
    {ec::unterminated_str, "unterminated multiline comment"},
    {ec::number_literal_too_large, "number literal too large"},
    {ec::unclosed_paren, "expected closing parentheses"},
    {ec::expected_expression, "expected an expression"},
    {ec::expected_semi, "expected a semicolon"},
    {ec::expected_else_after_then, "expected 'else' in if-then expression"},
    {ec::unexpected_expression, "unexpected expression"},
    {ec::invalid_char_literal, "more than one character in char literal"},
    {ec::unmatched_brace, "expected a matching '}'"},
    {ec::unexpected_end_of_input, "unexpected end of input"},
    {ec::expected_comma, "expected a comma"},
    {ec::expected_closing_bracket, "expected a closing square bracket"},
    {ec::expected_opening_brace, "expected an opening curly brace"},
    {ec::expected_type, "expected a type"},
    {ec::expected_identifier, "expected an identifier"},
    {ec::expected_declaration, "expected a declaration"},
    {ec::duplicate_module, "file already has a module declaration"},
};

static std::unordered_map<error_code, std::string_view> notes{
    {ec::unknown_char, "This character isn't used in any part of the language."},
    {ec::unexpected_tok, "Did you leave out a space?"},
    {ec::unterminated_block_comment, "Did you leave out the terminator?"},
    {ec::unterminated_char, "Did you leave out the terminator?"},
    {ec::unterminated_str, "Did you leave out the terminator?"},
    {ec::number_literal_too_large, "Number literals are of type 'i32' and must fit inside that."},
    {ec::expected_semi, "All statements require a ';' after them, unless they end with a '}'."},
    {ec::expected_else_after_then, "If an 'if' expression has 'then', an 'else' is required."},
    {ec::invalid_char_literal,
        "Char literals can only contain a single UTF-8 code point, not a UTF-8 character. If it "
        "doesn't fit inside one byte, you cannot use it."},
    {ec::expected_opening_brace, "A block was expected to begin here."},
};

std::string_view errors::error_message_from_code(error_code code) { return errs[code]; }

std::optional<std::string_view> errors::error_note_from_code(error_code code) {
  auto result = notes[code];

  return result == "" ? std::nullopt : std::make_optional(result);
}
