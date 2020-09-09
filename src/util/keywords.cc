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

#include "util/keywords.hh"
#include <cassert>
#include <unordered_map>

using namespace cascade;
using kind = core::token::kind;

// this gets reversed for string_to_kind
// every k/v pair get put back into string_to_kind with string_to_kind[v] = k;
static std::unordered_map<kind, std::string_view> kind_to_string{
    {kind::keyword_const, "const"},
    {kind::keyword_static, "static"},
    {kind::keyword_fn, "fn"},
    {kind::keyword_struct, "struct"},
    {kind::keyword_pub, "pub"},
    {kind::keyword_let, "let"},
    {kind::keyword_mut, "mut"},
    {kind::keyword_loop, "loop"},
    {kind::keyword_while, "while"},
    {kind::keyword_for, "for"},
    {kind::keyword_in, "in"},
    {kind::keyword_break, "break"},
    {kind::keyword_continue, "continue"},
    {kind::keyword_ret, "ret"},
    {kind::keyword_assert, "assert"},
    {kind::keyword_module, "module"},
    {kind::keyword_import, "import"},
    {kind::keyword_as, "as"},
    {kind::keyword_from, "from"},
    {kind::keyword_export, "export"},
    {kind::keyword_if, "if"},
    {kind::keyword_then, "then"},
    {kind::keyword_else, "else"},
    {kind::keyword_and, "and"},
    {kind::keyword_or, "or"},
    {kind::keyword_xor, "xor"},
    {kind::keyword_not, "not"},
    {kind::keyword_clone, "clone"},
    {kind::keyword_type, "type"},
    {kind::symbol_equal, "="},
    {kind::symbol_colon, ":"},
    {kind::symbol_coloncolon, "::"},
    {kind::symbol_star, "*"},
    {kind::symbol_pound, "&"},
    {kind::symbol_openbracket, "["},
    {kind::symbol_closebracket, "]"},
    {kind::symbol_at, "@"},
    {kind::symbol_dot, "."},
    {kind::symbol_openbrace, "{"},
    {kind::symbol_closebrace, "}"},
    {kind::symbol_openparen, "("},
    {kind::symbol_closeparen, ")"},
    {kind::symbol_semicolon, ";"},
    {kind::symbol_pipe, "|"},
    {kind::symbol_caret, "^"},
    {kind::symbol_plus, "+"},
    {kind::symbol_hyphen, "-"},
    {kind::symbol_forwardslash, "/"},
    {kind::symbol_percent, "%"},
    {kind::symbol_lt, "<"},
    {kind::symbol_leq, "<="},
    {kind::symbol_gt, ">"},
    {kind::symbol_geq, ">="},
    {kind::symbol_gtgt, ">>"},
    {kind::symbol_ltlt, "<<"},
    {kind::symbol_equalequal, "=="},
    {kind::symbol_bangequal, "!="},
    {kind::symbol_gtgtequal, ">>="},
    {kind::symbol_ltltequal, "<<="},
    {kind::symbol_poundequal, "&="},
    {kind::symbol_pipeequal, "|="},
    {kind::symbol_caretequal, "^="},
    {kind::symbol_percentequal, "%="},
    {kind::symbol_forwardslashequal, "/="},
    {kind::symbol_starequal, "*="},
    {kind::symbol_hyphenequal, "-="},
    {kind::symbol_plusequal, "+="},
    {kind::symbol_comma, ","},
    {kind::symbol_tilde, "~"},
};

static std::unordered_map<std::string_view, kind> string_to_kind{};

// allows "standards compliant" __attribute__((constructor)), so this would work on MSVC as well
// it simply initializes the strings_to_kind type with flipped k/vs to what kind_to_string has
static struct init {
  init() {
    for (auto &pair : kind_to_string) {
      string_to_kind[pair.second] = pair.first;
    }

    // these need to **not** be included in string_to_kind, so they're added here
    kind_to_string[kind::identifier] = "identifier";
    kind_to_string[kind::literal_number] = "number literal";
    kind_to_string[kind::literal_float] = "float literal";
    kind_to_string[kind::literal_bool] = "bool literal";
    kind_to_string[kind::literal_char] = "char literal";
    kind_to_string[kind::literal_string] = "string literal";
    kind_to_string[kind::unknown] = "unknown";
    kind_to_string[kind::error] = "error";

    // same for string_to_kind
    string_to_kind["true"] = kind::literal_bool;
    string_to_kind["false"] = kind::literal_bool;
  }
} init;

bool util::is_kind(std::string_view raw) {
  return string_to_kind.find(raw) != string_to_kind.end();
}

kind util::kind_from_string(std::string_view raw) { return string_to_kind[raw]; }

std::string_view util::string_from_kind(kind k) { return kind_to_string[k]; }
