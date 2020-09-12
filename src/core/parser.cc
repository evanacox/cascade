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
 * core/parser.cc:
 *   Implements the main parser for the program
 *
 *---------------------------------------------------------------------------*/

#include "core/parser.hh"
#include "ast/ast.hh"
#include "ast/detail/expressions.hh"
#include "ast/detail/literals.hh"
#include "ast/detail/types.hh"
#include "util/logging.hh"
#include <charconv>
#include <fmt/format.h>
#include <memory>
#include <set>
#include <stack>
#include <string>

using namespace cascade;
using namespace core;

using register_fn = std::function<void(std::unique_ptr<errors::error>)>;
using srcinfo = source_info;
using node_ptr = std::unique_ptr<ast::node>;
using expr_ptr = std::unique_ptr<ast::expression>;
using stmt_ptr = std::unique_ptr<ast::statement>;
using decl_ptr = std::unique_ptr<ast::declaration>;
using type_ptr = std::unique_ptr<ast::type_base>;
using kind = token::kind;
using ec = errors::error_code;

static std::set<std::string_view> builtin_words{
    "i8",
    "i16",
    "i32",
    "i64",
    "u8",
    "u16",
    "u32",
    "u64",
    "f32",
    "f64",
    "bool",
};

static bool is_builtin(const token &identifier) {
  assert(identifier.is(kind::identifier) && "calling is_builtin on non-identifier!");

  return builtin_words.find(identifier.raw()) != builtin_words.end();
}

struct error_sentinel {};

class parser_impl {
  lexer::return_type m_toks;

  std::size_t m_index;

  register_fn m_report;

public:
  /**
   * @brief Template for parsing a binary expression
   * @param next_level A lambda / function pointer / functor that returns the next
   * precedence level
   * @param kinds The token types to check for inside the `while` loop
   * @return An expression pointer
   */
  template <class HigherLevel, class... Kinds>
  [[nodiscard]] expr_ptr parse_binary(HigherLevel next_level, Kinds... kinds) {
    expr_ptr expr = next_level();

    while (!is_at_end() && current().is_one_of(kinds...)) {
      auto op = consume();
      expr_ptr rhs = next_level();

      expr = std::make_unique<ast::binary>(srcinfo::from(expr->info(), rhs->info()),
          op.type(),
          std::move(expr),
          std::move(rhs));
    }

    return expr;
  }

  /**
   * @brief Template for parsing a unary expression
   * @param curr_lvl Function that returns the current precedence level
   * @param next_lvl Function that returns the next precedence level
   * @param kinds The kinds to check against in current().is_one_of
   * @return An expression pointer
   */
  template <class CurrentLevel, class HigherLevel, class... Kinds>
  [[nodiscard]] expr_ptr parse_unary(CurrentLevel curr_lvl, HigherLevel next_lvl, Kinds... kinds) {
    if (!is_at_end() && current().is_one_of(kinds...)) {
      auto op = consume();
      expr_ptr rhs = curr_lvl();

      return std::make_unique<ast::unary>(srcinfo::from(op.info(), rhs->info()),
          op.type(),
          std::move(rhs));
    }

    return next_lvl();
  }

  // utility methods
  [[nodiscard]] const token &previous() const;
  [[nodiscard]] const token &current() const;
  [[nodiscard]] const token &current_nothrow() const noexcept;
  [[nodiscard]] const token &next() const;
  [[nodiscard]] bool is_at_end() const;

  // actions
  token consume();
  void synchronize();
  void check_end(std::string note = "");
  void check_semi(std::string note = "");

  // error reporting
  [[noreturn]] void report_error(ec code, core::token tok, std::string note = "") const;
  [[noreturn]] void report_error(ec code, node_ptr node, std::string note = "") const;
  void report_nothrow(ec code, core::token tok, std::string note = "") const noexcept;
  void report_nothrow(ec code, node_ptr node, std::string note = "") const noexcept;

  // recursive-descent methods
  [[nodiscard]] expr_ptr block();
  [[nodiscard]] expr_ptr grouping();
  [[nodiscard]] expr_ptr primary();
  [[nodiscard]] expr_ptr call();
  [[nodiscard]] expr_ptr unary();
  [[nodiscard]] expr_ptr multiplication();
  [[nodiscard]] expr_ptr addition();
  [[nodiscard]] expr_ptr bitshift();
  [[nodiscard]] expr_ptr bitwise_and();
  [[nodiscard]] expr_ptr bitwise_xor();
  [[nodiscard]] expr_ptr bitwise_or();
  [[nodiscard]] expr_ptr relational();
  [[nodiscard]] expr_ptr equality();
  [[nodiscard]] expr_ptr logical_not();
  [[nodiscard]] expr_ptr logical_and();
  [[nodiscard]] expr_ptr logical_xor();
  [[nodiscard]] expr_ptr logical_or();
  [[nodiscard]] expr_ptr if_then();
  [[nodiscard]] expr_ptr assignment();

  // utility method for call parsing
  [[nodiscard]] expr_ptr finish_call(expr_ptr);

  // parse statements
  [[nodiscard]] type_ptr type_with_colon();
  [[nodiscard]] type_ptr type_without_colon();
  [[nodiscard]] type_ptr finish_type();
  [[nodiscard]] stmt_ptr variable();
  [[nodiscard]] stmt_ptr ret();
  [[nodiscard]] stmt_ptr loop();
  [[nodiscard]] stmt_ptr assert_stmt();
  [[nodiscard]] stmt_ptr break_continue();
  [[nodiscard]] stmt_ptr expr_statement();

  // parse declarations
  [[nodiscard]] decl_ptr module_decl();
  [[nodiscard]] decl_ptr import_decl();
  [[nodiscard]] decl_ptr export_decl();
  [[nodiscard]] decl_ptr const_static();
  [[nodiscard]] decl_ptr fn();
  [[nodiscard]] decl_ptr type_decl();

  // main parse methods
  [[nodiscard]] expr_ptr expression();
  [[nodiscard]] stmt_ptr statement();
  [[nodiscard]] decl_ptr declaration();

  explicit parser_impl(lexer::return_type tokens, register_fn report);

  ast::program parse();
};

parser_impl::parser_impl(lexer::return_type tokens, register_fn report)
    : m_toks(std::move(tokens))
    , m_index{0}
    , m_report(std::move(report)) {}

token parser_impl::consume() {
  assert(!is_at_end() && "program isn't at the end of the tokens and trying to consume()");
  return m_toks[m_index++];
}

const token &parser_impl::current() const {
  if (is_at_end()) {
    report_error(ec::unexpected_end_of_input, previous());
  }

  return m_toks[m_index];
}

const token &parser_impl::current_nothrow() const noexcept {
  assert(!is_at_end() && "current_nothrow() isn't being called on end");
  return m_toks[m_index];
}

const token &parser_impl::previous() const {
  assert(m_index > 0 && "previous() isn't being called before consume()");
  return m_toks[m_index - 1];
}

const token &parser_impl::next() const {
  assert(m_index + 1 < m_toks.size() && "next() isn't being called at second-to-last");
  return m_toks[m_index + 1];
}

bool parser_impl::is_at_end() const { return m_index == m_toks.size(); }

void parser_impl::synchronize() {
  while (!is_at_end()) {
    if (current().is(kind::symbol_semicolon)) {
      consume();

      return;
    }

    // clang-format off
    if (current().is_one_of(
      kind::keyword_if,     kind::keyword_else,      kind::keyword_then, 
      kind::keyword_fn,     kind::keyword_let,       kind::keyword_mut,
      kind::keyword_ret,    kind::keyword_import,    kind::keyword_export,
      kind::keyword_module, kind::keyword_as,        kind::keyword_pub,
      kind::keyword_assert, kind::symbol_closebrace, kind::symbol_closeparen,
      kind::symbol_closebracket)) 
    {
      return;
    }
    // clang-format on

    consume();
  }
}

void parser_impl::check_end(std::string note) {
  if (is_at_end()) {
    report_error(ec::unexpected_end_of_input, previous(), note);
  }
}

void parser_impl::check_semi(std::string note) {
  if (is_at_end()) {
    report_error(ec::unexpected_end_of_input, previous(), "Expected a ';'!");
  }

  if (current().is_not(kind::symbol_semicolon)) {
    report_error(ec::expected_semi, consume(), note);
  }
}

void parser_impl::report_error(ec code, core::token tok, std::string note) const {
  m_report(std::make_unique<errors::token_error>(code,
      tok,
      note == "" ? std::nullopt : std::make_optional(note)));

  throw error_sentinel{};
}

void parser_impl::report_error(ec code, node_ptr node, std::string note) const {
  m_report(std::make_unique<errors::ast_error>(code,
      std::move(node),
      note == "" ? std::nullopt : std::make_optional(note)));

  throw error_sentinel{};
}

void parser_impl::report_nothrow(ec code, core::token tok, std::string note) const noexcept {
  m_report(std::make_unique<errors::token_error>(code,
      tok,
      note == "" ? std::nullopt : std::make_optional(note)));
}

void parser_impl::report_nothrow(ec code, node_ptr node, std::string note) const noexcept {
  m_report(std::make_unique<errors::ast_error>(code,
      std::move(node),
      note == "" ? std::nullopt : std::make_optional(note)));
}

expr_ptr parser_impl::finish_call(expr_ptr callee) {
  consume();
  std::vector<expr_ptr> args;

  while (!is_at_end() && current().is_not(kind::symbol_closeparen)) {
    args.emplace_back(expression());

    if (current().is_one_of(kind::symbol_comma, kind::symbol_closeparen)) {
      if (current().is(kind::symbol_comma)) {
        consume();
      }

      continue;
    }

    // if previous two failed, and this catches theres an issue
    report_error(ec::expected_comma, consume(), "Expected a ',' or a ')' after argument!");
  }

  auto close = consume();

  return std::make_unique<ast::call>(srcinfo::from(callee->info(), close.info()),
      std::move(callee),
      std::move(args));
}

expr_ptr parser_impl::grouping() {
  if (current().is(kind::symbol_openparen)) {
    auto begin = consume();

    // if expr is a nullopt, might as well consume the other ) to make it
    // easy for the rest of the parser
    auto expr = expression();

    if (current().is_not(kind::symbol_closeparen)) {
      auto unexpected = expression();

      report_error(ec::unclosed_paren, begin, "Did you forget a ')'? ");
    }

    auto end = consume();

    return expr;
  }

  if (current().is(kind::symbol_openbrace)) {
    return block();
  }

  report_error(ec::expected_expression, consume());
}

expr_ptr parser_impl::block() {
  if (current().is_not(kind::symbol_openbrace)) {
    report_error(ec::expected_opening_brace, consume());
  }

  auto start = consume();

  std::vector<stmt_ptr> statements;

  while (!is_at_end() && current().is_not(kind::symbol_closebrace)) {
    try {
      statements.emplace_back(statement());
    } catch (error_sentinel &) {
      synchronize();
    }
  }

  auto close = consume();

  return std::make_unique<ast::block>(srcinfo::from(start.info(), close.info()),
      std::move(statements),
      std::make_unique<ast::implied>(srcinfo::from(start.info(), 1)));
}

expr_ptr parser_impl::primary() {
  if (current().is(kind::literal_number)) {
    auto tok = consume();
    auto number = 0;

    auto [_, error_code] = std::from_chars(tok.raw().begin(), tok.raw().end(), number);

    if (error_code == std::errc::invalid_argument) {
      throw std::invalid_argument(
          fmt::format("bad argument to std::from_chars. token being parsed: '{}'", tok.raw()));
    }

    if (error_code == std::errc::result_out_of_range) {
      report_error(ec::number_literal_too_large, std::move(tok));
    }

    return std::make_unique<ast::int_literal>(tok.info(), number);
  }

  if (current().is(kind::literal_float)) {
    auto tok = consume();

    try {
      // std::from_chars on libstdc++ doesn't support floats/doubles
      auto number = std::stof(std::string{tok.raw()});

      return std::make_unique<ast::int_literal>(tok.info(), number);
    } // stof can throw on a non-integer string
    catch (std::invalid_argument &) {
      throw std::invalid_argument(fmt::format("bad argument to std::stof. tok: '{}'", tok.raw()));
    } // stof can throw if the string doesn't fit
    catch (std::out_of_range &) {
      report_error(ec::number_literal_too_large,
          std::move(tok),
          "float literals are of type 'f32' and must fit inside that");
    }
  }

  if (current().is(kind::literal_bool)) {
    auto tok = consume();
    auto value = tok.raw() == "true";

    return std::make_unique<ast::bool_literal>(tok.info(), value);
  }

  if (current().is(kind::literal_char)) {
    auto tok = consume();
    auto chars = tok.raw().substr(1, tok.raw().size() - 2); // need to cut out 's
    std::stringstream ss{std::string{chars}};

    char c;
    ss >> c;

    if (ss.rdbuf()->in_avail() != 0) {
      report_error(ec::invalid_char_literal, std::move(tok));
    }

    return std::make_unique<ast::char_literal>(tok.info(), c);
  }

  if (current().is(kind::literal_string)) {
    auto tok = consume();
    auto str = tok.raw().substr(1, tok.raw().size() - 2);

    return std::make_unique<ast::string_literal>(tok.info(), std::move(str));
  }

  if (current().is(kind::identifier)) {
    auto tok = consume();

    return std::make_unique<ast::identifier>(tok.info(), std::string{tok.raw()});
  }

  return grouping();
}

expr_ptr parser_impl::call() {
  auto expr = primary();

  while (!is_at_end()) {
    if (current().is(kind::symbol_openparen)) {
      expr = finish_call(std::move(expr));

      continue;
    }

    if (current().is(kind::symbol_openbracket)) {
      consume();

      auto index = expression();

      if (is_at_end()) {
        report_error(ec::unexpected_end_of_input, previous(), "Expected a closing ']'.");
      }

      if (current().is(kind::symbol_closebracket)) {
        auto close = consume();

        expr = std::make_unique<ast::index>(srcinfo::from(expr->info(), close.info()),
            std::move(expr),
            std::move(index));

        continue;
      }

      report_error(ec::expected_closing_bracket,
          consume(),
          "Expected a ']' to finish index access expression");
    }

    if (current().is(kind::symbol_dot)) {
      auto dot = consume();

      if (is_at_end()) {
        report_error(ec::unexpected_end_of_input,
            previous(),
            "Expected a field or method name, but got EOF.");
      }

      if (current().is_not(kind::identifier)) {
        report_error(ec::unexpected_tok, consume(), "Expected a field name or a method name.");
      }

      auto id = consume();

      expr = std::make_unique<ast::field_access>(srcinfo::from(expr->info(), id.info()),
          std::move(expr),
          std::string{id.raw()});

      continue;
    }

    break;
  }

  return expr;
}

expr_ptr parser_impl::unary() {
  return parse_unary([this]() { return unary(); },
      [this]() { return call(); },
      kind::symbol_tilde,
      kind::symbol_star,
      kind::symbol_pound,
      kind::symbol_at,
      kind::symbol_plus,
      kind::symbol_hyphen,
      kind::keyword_clone);
}

expr_ptr parser_impl::multiplication() {
  return parse_binary([this]() { return unary(); },
      kind::symbol_star,
      kind::symbol_forwardslash,
      kind::symbol_percent);
}

expr_ptr parser_impl::addition() {
  return parse_binary([this]() { return multiplication(); },
      kind::symbol_plus,
      kind::symbol_hyphen);
}

expr_ptr parser_impl::bitshift() {
  return parse_binary([this]() { return addition(); }, kind::symbol_gtgt, kind::symbol_ltlt);
}

expr_ptr parser_impl::bitwise_and() {
  return parse_binary([this]() { return bitshift(); }, kind::symbol_pound);
}

expr_ptr parser_impl::bitwise_xor() {
  return parse_binary([this]() { return bitwise_and(); }, kind::symbol_caret);
}

expr_ptr parser_impl::bitwise_or() {
  return parse_binary([this]() { return bitwise_xor(); }, kind::symbol_pipe);
}

expr_ptr parser_impl::relational() {
  return parse_binary([this]() { return bitwise_or(); },
      kind::symbol_gt,
      kind::symbol_geq,
      kind::symbol_lt,
      kind::symbol_leq);
}

expr_ptr parser_impl::equality() {
  return parse_binary([this]() { return relational(); },
      kind::symbol_equalequal,
      kind::symbol_bangequal);
}

expr_ptr parser_impl::logical_not() {
  return parse_unary([this]() { return logical_not(); },
      [this]() { return equality(); },
      kind::keyword_not);
}

expr_ptr parser_impl::logical_and() {
  return parse_binary([this]() { return logical_not(); }, kind::keyword_and);
}

expr_ptr parser_impl::logical_xor() {
  return parse_binary([this]() { return logical_and(); }, kind::keyword_xor);
}

expr_ptr parser_impl::logical_or() {
  return parse_binary([this]() { return logical_xor(); }, kind::keyword_or);
}

expr_ptr parser_impl::if_then() {
  if (current().is(kind::keyword_if)) {
    auto keyword_if = consume();
    auto condition = if_then();

    // if `then` was present, if and else both need to be parsed slightly differently
    auto is_then = current().is(kind::keyword_then);

    // if `then`, it needs to be consumed and an expression parsed. (',' evaluates lhs and
    // returns rhs) if not, return a block
    auto true_clause = (is_then) ? consume(), if_then() : block();

    if (current().is(kind::keyword_else)) {
      consume();
      auto false_clause = (is_then) ? if_then() : block();

      return std::make_unique<ast::if_else>(srcinfo::from(keyword_if.info(), false_clause->info()),
          std::move(condition),
          std::move(true_clause),
          std::move(false_clause));
    }

    if (is_then) {
      report_error(ec::expected_else_after_then,
          std::make_unique<ast::if_else>(srcinfo::from(keyword_if.info(), true_clause->info()),
              std::move(condition),
              std::move(true_clause),
              std::nullopt));
    }

    return std::make_unique<ast::if_else>(srcinfo::from(keyword_if.info(), true_clause->info()),
        std::move(condition),
        std::move(true_clause),
        std::nullopt);
  }

  return logical_or();
}

expr_ptr parser_impl::assignment() {
  auto expr = if_then();

  while (!is_at_end() && current().is_assignment()) {
    auto op = consume();
    auto rhs = assignment();

    expr = std::make_unique<ast::binary>(srcinfo::from(expr->info(), rhs->info()),
        op.type(),
        std::move(expr),
        std::move(rhs));
  }

  return expr;
}

expr_ptr parser_impl::expression() { return assignment(); }

type_ptr parser_impl::finish_type() {
  if (current().is(kind::symbol_star)) {
    auto star = consume();

    if (current().is(kind::keyword_mut)) {
      consume();

      auto type = finish_type();

      return std::make_unique<ast::pointer>(srcinfo::from(star.info(), type->info()),
          ast::pointer_type::mut_ptr,
          std::move(type));
    }

    auto type = finish_type();

    return std::make_unique<ast::pointer>(srcinfo::from(star.info(), type->info()),
        ast::pointer_type::ptr,
        std::move(type));
  }

  if (current().is(kind::symbol_openbracket)) {
    auto open = consume();

    if (current().is_not(kind::symbol_closebracket)) {
      report_error(ec::unexpected_tok, consume(), "Expected a ']' to match opening '['");
    }

    consume();

    auto type = finish_type();

    return std::make_unique<ast::array>(srcinfo::from(open.info(), type->info()),
        0,
        std::move(type));
  }

  if (current().is(kind::identifier)) {
    auto id = consume();

    if (id.raw() == "bool") {
      return std::make_unique<ast::builtin>(id.info(), 1, ast::numeric_type::boolean);
    }

    if (id.raw()[0] == 'i' || id.raw()[0] == 'u') {
      auto width = id.raw().substr(1);
      auto type = (id.raw()[0] == 'i') // iN is signed, uN is unsigned
                      ? ast::numeric_type::integer
                      : ast::numeric_type::unsigned_integer;

      if (width == "8") {
        return std::make_unique<ast::builtin>(id.info(), 8, type);
      } else if (width == "16") {
        return std::make_unique<ast::builtin>(id.info(), 16, type);

      } else if (width == "32") {
        return std::make_unique<ast::builtin>(id.info(), 32, type);

      } else if (width == "64") {
        return std::make_unique<ast::builtin>(id.info(), 64, type);
      } else {
        // i12 is a perfectly valid struct name, no matter how much I may dislike it
        return std::make_unique<ast::user_defined>(id.info(), std::string{id.raw()});
      }
    }

    if (id.raw()[0] == 'f') {
      auto width = id.raw().substr(1);

      if (width == "32") {
        return std::make_unique<ast::builtin>(id.info(), 32, ast::numeric_type::floating_point);

      } else if (width == "64") {
        return std::make_unique<ast::builtin>(id.info(), 64, ast::numeric_type::floating_point);
      } else {
        // i12 is a perfectly valid struct name, no matter how much I may dislike it
        return std::make_unique<ast::user_defined>(id.info(), std::string{id.raw()});
      }
    }

    return std::make_unique<ast::user_defined>(id.info(), std::string{id.raw()});
  }

  report_error(ec::expected_type, current(), "An identifier, *, *mut or [] was expected.");
}

type_ptr parser_impl::type_with_colon() {
  if (current().is_not(kind::symbol_colon)) {
    report_error(ec::unexpected_tok, consume(), "Expected a ':' before type!");
  }

  consume();

  // references can only appear at the very beginning
  if (current().is(kind::symbol_pound)) {
    auto pound = consume();

    if (current().is(kind::keyword_mut)) {
      consume();

      auto type = finish_type();

      return std::make_unique<ast::reference>(srcinfo::from(pound.info(), type->info()),
          ast::reference_type::mut_ref,
          std::move(type));
    }

    auto type = finish_type();

    return std::make_unique<ast::reference>(srcinfo::from(pound.info(), type->info()),
        ast::reference_type::ref,
        std::move(type));
  }

  return finish_type();
}

type_ptr parser_impl::type_without_colon() {
  // references can only appear at the very beginning
  if (current().is(kind::symbol_pound)) {
    auto pound = consume();

    if (current().is(kind::keyword_mut)) {
      consume();

      auto type = finish_type();

      return std::make_unique<ast::reference>(srcinfo::from(pound.info(), type->info()),
          ast::reference_type::mut_ref,
          std::move(type));
    }

    auto type = finish_type();

    return std::make_unique<ast::reference>(srcinfo::from(pound.info(), type->info()),
        ast::reference_type::ref,
        std::move(type));
  }

  return finish_type();
}

stmt_ptr parser_impl::variable() {
  auto begin = consume();

  if (current().is_not(kind::identifier)) {
    report_error(ec::expected_identifier,
        consume(),
        fmt::format("Expected an identifier after keyword '{}'.", begin.raw()));
  }

  auto id = consume();

  // type has to be explicitly the base
  type_ptr var_type = (current().is(kind::symbol_colon))
                          ? type_with_colon()
                          : std::make_unique<ast::implied>(id.info());

  if (current().is_not(kind::symbol_equal)) {
    report_error(ec::unexpected_tok, consume(), "Expected an '=' for variable initializer!");
  }

  consume();

  auto expr = expression();

  check_semi("Expected a ';' after initializer!");

  auto semi = consume();

  if (begin.is(kind::keyword_let)) {
    return std::make_unique<ast::let>(srcinfo::from(begin.info(), semi.info()),
        std::move(expr),
        std::move(var_type),
        std::string{id.raw()});
  } else {
    return std::make_unique<ast::mut>(srcinfo::from(begin.info(), semi.info()),
        std::move(expr),
        std::move(var_type),
        std::string{id.raw()});
  }
}

stmt_ptr parser_impl::ret() {
  auto ret = consume();
  std::optional<expr_ptr> expr = std::nullopt;

  if (current().is_not(kind::symbol_semicolon)) {
    expr = expression();
  }

  if (current().is_not(kind::symbol_semicolon)) {
    report_error(ec::expected_semi, consume());
  }

  auto semi = consume();

  return std::make_unique<ast::ret>(srcinfo::from(ret.info(), semi.info()), std::move(expr));
}

stmt_ptr parser_impl::loop() {
  auto begin = consume();

  if (begin.is(kind::keyword_loop)) {
    auto body = expression();

    return std::make_unique<ast::loop>(srcinfo::from(begin.info(), body->info()),
        std::nullopt,
        std::move(body));
  } else {
    auto condition = expression();
    auto body = expression();

    return std::make_unique<ast::loop>(srcinfo::from(begin.info(), body->info()),
        std::move(condition),
        std::move(body));
  }
}

stmt_ptr parser_impl::assert_stmt() { return nullptr; }

stmt_ptr parser_impl::break_continue() { return nullptr; }

stmt_ptr parser_impl::expr_statement() {
  auto expr = expression();

  if (current().is(kind::symbol_semicolon)) {
    auto semi = consume();

    return std::make_unique<ast::expression_statement>(srcinfo::from(expr->info(), semi.info()),
        std::move(expr));
  }

  // expressions w/ blocks don't need semis
  if (previous().is(kind::symbol_closebrace)) {
    return std::make_unique<ast::expression_statement>(expr->info(), std::move(expr));
  }

  report_error(ec::expected_semi, current(), "Expected a ';' after the expression");
}

stmt_ptr parser_impl::statement() {
  if (current().is_one_of(kind::keyword_let, kind::keyword_mut)) {
    return variable();
  }

  if (current().is_one_of(kind::keyword_while, kind::keyword_for, kind::keyword_loop)) {
    return loop();
  }

  if (current().is(kind::keyword_ret)) {
    return ret();
  }

  if (current().is_one_of(kind::keyword_break, kind::keyword_continue)) {
    return break_continue();
  }

  if (current().is(kind::keyword_assert)) {
    return assert_stmt();
  }

  return expr_statement();
}

decl_ptr parser_impl::import_decl() { return nullptr; }

decl_ptr parser_impl::module_decl() {
  auto begin = consume();

  if (current().is_not(kind::identifier)) {
    report_error(ec::expected_identifier, consume(), "Expected a module name!");
  }

  auto name = consume();

  if (is_builtin(name)) {
    report_nothrow(ec::unexpected_builtin,
        name,
        "Expected a module name, got a reserved builtin name!");
  }

  check_semi("Expected a ';' after initializer!");

  auto semi = consume();

  return std::make_unique<ast::module_decl>(srcinfo::from(begin.info(), semi.info()),
      std::string{name.raw()});
}

decl_ptr parser_impl::export_decl() {
  auto begin = consume();
  auto decl = declaration();

  if (decl->is(ast::kind::declaration_export)) {
    report_error(ec::cannot_export_export, std::move(decl), "Cannot export an export declaration!");
  }

  return std::make_unique<ast::export_decl>(srcinfo::from(begin.info(), decl->info()),
      std::move(decl));
}

decl_ptr parser_impl::const_static() {
  auto begin = consume();

  if (current().is_not(kind::identifier)) {
    report_error(ec::expected_identifier,
        consume(),
        fmt::format("Expected an identifier after keyword '{}'!", begin.raw()));
  }

  auto id = consume();

  if (is_builtin(id)) {
    report_nothrow(ec::unexpected_builtin,
        id,
        "Expected a variable name, got a reserved builtin name!");
  }

  // type has to be explicitly the base
  type_ptr var_type = (current().is(kind::symbol_colon))
                          ? type_with_colon()
                          : std::make_unique<ast::implied>(id.info());

  if (current().is_not(kind::symbol_equal)) {
    report_error(ec::unexpected_tok, consume(), "Expected an '=' for variable initializer!");
  }

  consume();

  auto expr = expression();

  check_semi("Expected a ';' after initializer!");

  auto semi = consume();

  if (begin.is(kind::keyword_const)) {
    return std::make_unique<ast::const_decl>(srcinfo::from(begin.info(), semi.info()),
        std::string{id.raw()},
        std::move(expr),
        std::move(var_type));
  } else {
    return std::make_unique<ast::static_decl>(srcinfo::from(begin.info(), semi.info()),
        std::string{id.raw()},
        std::move(expr),
        std::move(var_type));
  }
}

decl_ptr parser_impl::type_decl() {
  auto begin = consume();

  if (current().is_not(kind::identifier)) {
    report_error(ec::expected_identifier, consume(), "Expected an identifier for the type alias!");
  }

  auto name = consume();

  if (is_builtin(name)) {
    report_nothrow(ec::unexpected_builtin,
        name,
        "Expected a type alias name, got a reserved builtin name!");
  }

  if (current().is_not(kind::symbol_equal)) {
    report_error(ec::unexpected_tok, consume(), "Expected an '=' and a type for type alias!");
  }

  consume();

  auto type = type_without_colon();

  check_semi("Expected a ';' after type alias!");

  auto semi = consume();

  return std::make_unique<ast::type_decl>(srcinfo::from(begin.info(), semi.info()),
      std::move(type),
      std::string{name.raw()});
}

decl_ptr parser_impl::fn() {
  auto begin = consume();

  if (current().is_not(kind::identifier)) {
    report_error(ec::expected_identifier,
        consume(),
        "Expected an identifier for the function name!");
  }

  auto name = consume();

  if (is_builtin(name)) {
    report_nothrow(ec::unexpected_builtin, name, "Expected an fn name, got reserved builtin name!");
  }

  if (current().is_not(kind::symbol_openparen)) {
    report_error(ec::unexpected_tok, consume(), "Expected a '(' to begin fn argument list!");
  }

  consume();

  std::vector<ast::argument> args;

  while (current().is_not(kind::symbol_closeparen)) {
    if (current().is_not(kind::identifier)) {
      report_error(ec::expected_identifier, consume(), "Expected an argument name!");
    }

    auto arg_name = consume();

    if (current().is_not(kind::symbol_colon)) {
      report_error(ec::unexpected_tok, consume(), "Expected a ':' for argument type!");
    }

    auto arg_type = type_with_colon();

    args.emplace_back(srcinfo::from(arg_name.info(), arg_type->info()),
        std::string{arg_name.raw()},
        std::move(arg_type));

    if (current().is_not(kind::symbol_closeparen)) {
      if (current().is_not(kind::symbol_comma)) {
        report_error(ec::expected_comma, consume(), "Expected a comma between arguments!");
      }

      consume();
    }
  }

  consume();

  auto return_type = (current().is(kind::symbol_colon))
                         ? type_with_colon()
                         : std::make_unique<ast::void_type>(previous().info());

  auto body = block();

  return std::make_unique<ast::fn>(srcinfo::from(begin.info(), body->info()),
      std::string{name.raw()},
      std::move(args),
      std::move(return_type),
      std::move(body));
}

decl_ptr parser_impl::declaration() {
  switch (current().type()) {
    case kind::keyword_const:
    case kind::keyword_static:
      return const_static();
    case kind::keyword_fn:
      return fn();
    case kind::keyword_import:
      return import_decl();
    case kind::keyword_module:
      return module_decl();
    case kind::keyword_export:
      return export_decl();
    case kind::keyword_type:
      return type_decl();
    default:
      report_error(ec::expected_declaration, consume());
  }
}

ast::program parser_impl::parse() {
  std::vector<decl_ptr> decls;
  auto has_module = false;

  while (!is_at_end()) {
    // util::debug_print(expression());
    try {
      auto decl = declaration();

      if (decl->is(ast::kind::declaration_module)) {
        if (has_module) {
          report_error(ec::duplicate_module,
              std::move(decl),
              "You can only have one module declaration per file.");
        }

        has_module = true;
      }

      decls.emplace_back(std::move(decl));
    } catch (error_sentinel &) {
      synchronize();
    }
  }

  return ast::program(std::move(decls));
}

ast::program core::parse(std::vector<token> source, register_fn report) {
  parser_impl parser(std::move(source), std::move(report));

  return parser.parse();
}
