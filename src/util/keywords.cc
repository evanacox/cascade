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
 * util/keywords.cc:
 *   Implements the helper functions defined in keywords.hh
 *
 *---------------------------------------------------------------------------*/

#include "cascade/util/keywords.hh"
#include <cassert>
#include <unordered_map>

using namespace cascade;

// this gets reversed for string_to_kind
// every k/v pair get put back into string_to_kind with string_to_kind[v] = k;
static std::unordered_map<core::token::kind, std::string_view> kind_to_string{
    {core::token::kind::keyword_const, "const"},
    {core::token::kind::keyword_static, "static"},
    {core::token::kind::keyword_fn, "fn"},
    {core::token::kind::keyword_struct, "struct"},
    {core::token::kind::keyword_pub, "pub"},
    {core::token::kind::keyword_let, "let"},
    {core::token::kind::keyword_mut, "mut"},
    {core::token::kind::keyword_loop, "loop"},
    {core::token::kind::keyword_while, "while"},
    {core::token::kind::keyword_for, "for"},
    {core::token::kind::keyword_in, "in"},
    {core::token::kind::keyword_break, "break"},
    {core::token::kind::keyword_continue, "continue"},
    {core::token::kind::keyword_ret, "ret"},
    {core::token::kind::keyword_assert, "assert"},
    {core::token::kind::keyword_module, "module"},
    {core::token::kind::keyword_import, "import"},
    {core::token::kind::keyword_as, "as"},
    {core::token::kind::keyword_from, "from"},
    {core::token::kind::keyword_export, "export"},
    {core::token::kind::keyword_if, "if"},
    {core::token::kind::keyword_then, "then"},
    {core::token::kind::keyword_else, "else"},
    {core::token::kind::keyword_and, "and"},
    {core::token::kind::keyword_or, "or"},
    {core::token::kind::keyword_xor, "xor"},
    {core::token::kind::keyword_not, "not"},
    {core::token::kind::symbol_equal, "="},
    {core::token::kind::symbol_colon, ":"},
    {core::token::kind::symbol_coloncolon, "::"},
    {core::token::kind::symbol_star, "*"},
    {core::token::kind::symbol_pound, "&"},
    {core::token::kind::symbol_openbracket, "["},
    {core::token::kind::symbol_closebracket, "]"},
    {core::token::kind::symbol_at, "@"},
    {core::token::kind::symbol_dot, "."},
    {core::token::kind::symbol_openbrace, "{"},
    {core::token::kind::symbol_closebrace, "}"},
    {core::token::kind::symbol_openparen, "("},
    {core::token::kind::symbol_closeparen, ")"},
    {core::token::kind::symbol_semicolon, ";"},
    {core::token::kind::symbol_pipe, "|"},
    {core::token::kind::symbol_caret, "^"},
    {core::token::kind::symbol_plus, "+"},
    {core::token::kind::symbol_hyphen, "-"},
    {core::token::kind::symbol_forwardslash, "/"},
    {core::token::kind::symbol_percent, "%"},
    {core::token::kind::symbol_lt, "<"},
    {core::token::kind::symbol_leq, "<="},
    {core::token::kind::symbol_gt, ">"},
    {core::token::kind::symbol_geq, ">="},
    {core::token::kind::symbol_gtgt, ">>"},
    {core::token::kind::symbol_ltlt, "<<"},
    {core::token::kind::symbol_equalequal, "=="},
    {core::token::kind::symbol_bangequal, "!="},
    {core::token::kind::symbol_gtgtequal, ">>="},
    {core::token::kind::symbol_ltltequal, "<<="},
    {core::token::kind::symbol_poundequal, "&="},
    {core::token::kind::symbol_pipeequal, "|="},
    {core::token::kind::symbol_caretequal, "^="},
    {core::token::kind::symbol_percentequal, "%="},
    {core::token::kind::symbol_forwardslashequal, "/="},
    {core::token::kind::symbol_starequal, "*="},
    {core::token::kind::symbol_hyphenequal, "-="},
    {core::token::kind::symbol_plusequal, "+="},
    {core::token::kind::symbol_comma, ","},
};

static std::unordered_map<std::string_view, core::token::kind> string_to_kind{};

// allows "standard" __attribute__((constructor)), so this would work on MSVC as well
// it simply initializes the strings_to_kind type with flipped k/vs to what kind_to_string has
static struct init {
  init() {
    for (auto &pair : kind_to_string) {
      string_to_kind[pair.second] = pair.first;
    }

    // these need to **not** be included in string_to_kind, so they're added here
    kind_to_string[core::token::kind::identifier] = "identifier";
    kind_to_string[core::token::kind::literal_number] = "number literal";
    kind_to_string[core::token::kind::literal_bool] = "bool literal";
    kind_to_string[core::token::kind::literal_char] = "char literal";
    kind_to_string[core::token::kind::literal_string] = "string literal";
    kind_to_string[core::token::kind::unknown] = "unknown";
    kind_to_string[core::token::kind::error] = "error";

    // same for string_to_kind
    string_to_kind["true"] = core::token::kind::literal_bool;
    string_to_kind["false"] = core::token::kind::literal_bool;
  }
} init;

bool util::is_kind(std::string_view raw) {
  return string_to_kind.find(raw) != string_to_kind.end();
}

core::token::kind util::kind_from_string(std::string_view raw) { return string_to_kind[raw]; }

std::string_view util::string_from_kind(core::token::kind kind) { return kind_to_string[kind]; }