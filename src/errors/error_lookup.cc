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
};

static std::unordered_map<error_code, std::string_view> notes{
    {ec::unknown_char, "This character isn't used in any part of the language."},
    {ec::unexpected_tok, "Did you leave out a space?"},
    {ec::unterminated_block_comment, "Did you leave out the terminator?"},
    {ec::unterminated_char, "Did you leave out the terminator?"},
    {ec::unterminated_str, "Did you leave out the terminator?"},
};

std::string_view errors::error_message_from_code(error_code code) { return errs[code]; }

std::optional<std::string_view> errors::error_note_from_code(error_code code) {
  auto result = notes[code];

  return result == "" ? std::nullopt : std::make_optional(result);
}