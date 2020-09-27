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
 * util/logging.cc:
 *   Implements the logging utility defined in logging.hh
 *
 *---------------------------------------------------------------------------*/

#include "util/logging.hh"
#include "ast/detail/declarations.hh"
#include "ast/detail/types.hh"
#include "ast/visitor.hh"
#include "errors/error_lookup.hh"
#include "errors/error_visitor.hh"
#include "util/keywords.hh"
#include "util/types.hh"
#include <fmt/core.h>
#include <fmt/format.h>
#include <fstream>
#include <iostream>

#if defined(PLATFORM_POSIX) || defined(__linux__) || defined(__unix__)
#define CASCADE_IS_POSIX
#include <sys/ioctl.h>
#elif defined(_WIN32)
#define CASCADE_IS_WIN32
#include <windows.h>
#else
static_assert(false, "Unrecognized platform!")
#endif

using namespace cascade;
using namespace util;

namespace fs = std::filesystem;

/**
 * @brief Gets the executable name from the OS and returns it as a string
 * @return The executable name
 */
static std::string executable_name() {
#ifdef CASCADE_IS_POSIX
  std::string sp;
  std::ifstream("/proc/self/comm") >> sp;
  return sp;
#elif defined(CASCADE_IS_WIN32)
  std::string str(' ', MAX_PATH);
  GetModuleFileNameA(nullptr, &str[0], MAX_PATH);
  return buf;
#endif
}

/**
 * @brief Returns an std::pair holding the rows and columns of the terminal
 * @return std::pair<rows, columns>
 */
[[maybe_unused]] static std::pair<unsigned, unsigned> terminal_size() {
#ifdef CASCADE_IS_POSIX
  winsize w;
  ioctl(0, TIOCGWINSZ, &w);
  return {w.ws_row, w.ws_col};
#elif defined(CASCADE_IS_WIN32)
  CONSOLE_SCREEN_BUFFER_INFO csbi;

  if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
    return {csbi.srWindow.Bottom - csbi.srWindow.Top + 1,
        csbi.srWindow.Right - csbi.srWindow.Left + 1};
  }

  return {-1, -1};
#endif
}

/**
 * @brief Turns an enum class into an integer when needed
 */
template <class T> std::underlying_type_t<T> to_num(T item) {
  return static_cast<std::underlying_type_t<T>>(item);
}

/**
 * @brief Returns the executable name
 * @return The current executable name in cyan
 */
static std::string formatted_exe_name() { return colors::cyan(executable_name()); }

/**
 * @brief Returns a bold-red "error: " string
 * @return an "error: " tag with bold-red ansi codes
 */
static std::string formatted_error_tag() { return colors::bold_red("error:"); }

/**
 * Calculates the number of digits in a base 10 number
 * @param n The number to calculate
 * @return The number of digits in the number
 */
static constexpr unsigned number_of_digits(int n) {
  assert(n >= 0 && "Expected the number to be positive or 0!");

  auto digits = 1;

  while (n /= 10) {
    ++digits;
  }

  return digits;
}

/** @brief Visits AST nodes to print them out */
struct printer : public ast::visitor<void> {
  std::string m_prefix = "";

  void accept_with_prefix(ast::node &node);

#define VISIT(type) virtual void visit(ast::type &) final

  CASCADE_VISIT_TYPES

#undef VISIT
};

using kind = ast::kind;

void printer::accept_with_prefix(ast::node &node) {
  m_prefix += "  ";
  node.accept(*this);
  m_prefix = m_prefix.substr(0, m_prefix.size() - 2);
}

void printer::visit(ast::type &node) { std::cout << util::to_string(node.data()) << "\n"; }

void printer::visit(ast::type_decl &decl) {
  std::cout << "type alias {\n";
  fmt::print("{}  type: ", m_prefix);
  decl.type().accept(*this);
  fmt::print("{}  name: {}\n", m_prefix, decl.name());
  fmt::print("{}}}\n", m_prefix);
}

void printer::visit(ast::const_decl &decl) {
  std::cout << "const decl {\n";
  fmt::print("{}  type: ", m_prefix);

  decl.type().accept(*this);

  fmt::print("{}  name: {}\n", m_prefix, decl.name());
  fmt::print("{}  init: ", m_prefix);

  accept_with_prefix(decl.initializer());

  fmt::print("{}}}\n", m_prefix);
}

void printer::visit(ast::static_decl &decl) {
  std::cout << "static decl {\n";
  fmt::print("{}  type: ", m_prefix);

  decl.type().accept(*this);

  fmt::print("{}  name: {}\n", m_prefix, decl.name());
  fmt::print("{}  init: ", m_prefix);

  accept_with_prefix(decl.initializer());

  fmt::print("{}}}\n", m_prefix);
}

void printer::visit(ast::argument &arg) {
  std::cout << "argument {\n";
  fmt::print("{}  name: {}\n", m_prefix, arg.name());
  fmt::print("{}  type: ", m_prefix);

  arg.type().accept(*this);
  fmt::print("{}}}\n", m_prefix);
}

void printer::visit(ast::fn &fn) {
  std::cout << "fn {\n";
  fmt::print("{}  name: {}\n", m_prefix, fn.name());
  fmt::print("{}  type: ", m_prefix);
  fn.type().accept(*this);

  if (fn.args().size() != 0) {
    // hack to get first argument to print at right level
    fmt::print("{}  args: [\n{}    ", m_prefix, m_prefix);

    m_prefix += "  ";
    for (auto &arg : fn.args()) {
      accept_with_prefix(arg);
    }
    m_prefix = m_prefix.substr(0, m_prefix.length() - 2);

    fmt::print("{}  ]\n", m_prefix);
  } else {
    fmt::print("{}  args: []\n", m_prefix);
  }

  fmt::print("{}  body: ", m_prefix);
  accept_with_prefix(fn.body());
  fmt::print("{}}}\n", m_prefix);
}

void printer::visit(ast::module_decl &mod) { fmt::print("module: {}\n", mod.name()); }

void printer::visit(ast::import_decl &impt) {
  std::cout << "import {\n";
  fmt::print("{}  from: {}\n", m_prefix, impt.name());
  fmt::print("{}  items: [\n", m_prefix);

  for (auto &item : impt.items()) {
    fmt::print("{}    {}\n,", m_prefix, item);
  }

  fmt::print("{}  ]\n", m_prefix);
  fmt::print("{}}}\n", m_prefix);
}

void printer::visit(ast::export_decl &expt) {
  std::cout << "(exported) ";

  expt.exported().accept(*this);
}

void printer::visit(ast::char_literal &c) { fmt::print("char literal: '{}'\n", c.value()); }

void printer::visit(ast::string_literal &s) { fmt::print("string literal: \"{}\"\n", s.value()); }

void printer::visit(ast::int_literal &d) { fmt::print("integer literal: {}\n", d.value()); }

void printer::visit(ast::float_literal &f) { fmt::print("float literal: {}\n", f.value()); }

void printer::visit(ast::bool_literal &b) { fmt::print("bool literal: {}\n", b.value()); }

void printer::visit(ast::identifier &id) { fmt::print("identifier: '{}'\n", id.name()); }

void printer::visit(ast::call &call) {
  std::cout << "call {\n";
  fmt::print("{}  callee: ", m_prefix);
  accept_with_prefix(call.callee());

  if (call.args().size() > 0) {
    fmt::print("{}  args: [\n", m_prefix);

    m_prefix += "  ";
    for (auto &arg : call.args()) {
      fmt::print("{}  arg: ", m_prefix);
      accept_with_prefix(*arg);
    }
    m_prefix = m_prefix.substr(0, m_prefix.size() - 2);

    fmt::print("{}  ]\n", m_prefix);
  } else {
    fmt::print("{}  args: [ ]\n", m_prefix);
  }
  fmt::print("{}}}\n", m_prefix);
}

void printer::visit(ast::binary &binop) {
  std::cout << "binary {\n";
  fmt::print("{}  op: {}\n", m_prefix, string_from_kind(binop.op()));
  fmt::print("{}  lhs: ", m_prefix);
  accept_with_prefix(binop.lhs());
  fmt::print("{}  rhs: ", m_prefix);
  accept_with_prefix(binop.rhs());
  fmt::print("{}}}\n", m_prefix);
}

void printer::visit(ast::unary &unop) {
  std::cout << "unary {\n";
  fmt::print("{}  op: {}\n", m_prefix, string_from_kind(unop.op()));
  fmt::print("{}  rhs: ", m_prefix);
  accept_with_prefix(unop.rhs());
  fmt::print("{}}}\n", m_prefix);
}

void printer::visit(ast::field_access &field) {
  std::cout << "field access {\n";
  fmt::print("{}  object: ", m_prefix);
  accept_with_prefix(field.accessed());
  fmt::print("{}  field: {}\n", m_prefix, field.field_name());
  fmt::print("{}}}\n", m_prefix);
}

void printer::visit(ast::index &idx) {
  std::cout << "index access {\n";
  fmt::print("{}  object: ", m_prefix);
  accept_with_prefix(idx.array());
  fmt::print("{}  index: ", m_prefix);
  accept_with_prefix(idx.idx());
  fmt::print("{}}}\n", m_prefix);
}

void printer::visit(ast::if_else &ifelse) {
  std::cout << "if {\n";
  fmt::print("{}  condition: ", m_prefix);
  accept_with_prefix(ifelse.condition());
  fmt::print("{}  true block: ", m_prefix);
  accept_with_prefix(ifelse.true_clause());

  if (ifelse.else_clause()) {
    fmt::print("{}  false block: ", m_prefix);
    accept_with_prefix(ifelse.else_clause().value());
  }

  fmt::print("{}}}\n", m_prefix);
}

void printer::visit(ast::struct_init &) { throw std::logic_error{"Not implemented!"}; }

void printer::visit(ast::block &block) {
  std::cout << "block {\n";
  fmt::print("{}  return_type: ", m_prefix);
  block.type().accept(*this);

  if (block.statements().size() != 0) {
    fmt::print("{}  items: [\n", m_prefix);

    m_prefix += "  ";
    for (auto &item : block.statements()) {
      fmt::print("{}  ", m_prefix);
      accept_with_prefix(*item);
    }
    m_prefix = m_prefix.substr(0, m_prefix.size() - 2);

    fmt::print("{}  ]\n", m_prefix);
  } else {
    fmt::print("{}  items: []\n", m_prefix);
  }

  fmt::print("{}}}\n", m_prefix);
}

void printer::visit(ast::expression_statement &stmt) {
  std::cout << "expr statement: ";
  stmt.expr().accept(*this);
}

void printer::visit(ast::let &stmt) {
  std::cout << "let {\n";
  fmt::print("{}  type: ", m_prefix);
  stmt.type().accept(*this);
  fmt::print("{}  name: '{}'\n", m_prefix, stmt.name());
  fmt::print("{}  initializer: ", m_prefix);
  accept_with_prefix(stmt.initializer());
  fmt::print("{}}}\n", m_prefix);
}

void printer::visit(ast::mut &stmt) {
  std::cout << "mut {\n";
  fmt::print("{}  type: ", m_prefix);
  stmt.type().accept(*this);
  fmt::print("{}  name: '{}'\n", m_prefix, stmt.name());
  fmt::print("{}  initializer: ", m_prefix);
  accept_with_prefix(stmt.initializer());
  fmt::print("{}}}\n", m_prefix);
}

void printer::visit(ast::ret &ret) {
  std::cout << "ret {\n";
  fmt::print("{}  return value: ", m_prefix);

  if (ret.return_value()) {
    accept_with_prefix(ret.return_value().value());
  } else {
    std::cout << "none\n";
  }

  fmt::print("{}}}\n", m_prefix);
}

void printer::visit(ast::loop &loop) {
  std::cout << "loop {\n";
  fmt::print("{}  condition: ", m_prefix);

  if (loop.condition()) {
    accept_with_prefix(loop.condition().value());
  } else {
    std::cout << "none\n";
  }

  fmt::print("{}  body: ", m_prefix);
  accept_with_prefix(loop.body());

  fmt::print("{}}}\n", m_prefix);
}

void util::error(std::string_view message) {
  fmt::print("{} {} {}\n",
      formatted_exe_name(),
      formatted_error_tag(),
      colors::bold_white(std::string{message}));
}

class logger::impl : public errors::error_visitor {
  std::string_view m_source;

  /** @brief Returns the path:line:col thing */
  std::string pretty_path(const errors::error &err) const;

  /** @brief Puts the path:line:col error: [Enumber] error message thing up */
  void print_start(const errors::error &err) const;

  /** @brief Prints out the code line in the error */
  void print_code(const errors::error &err) const;

  /** @brief Puts a ~~~ or ^ under the source code for an error */
  void point_out(const errors::error &err) const;

  /** @brief Prints out the err'rs note, if it has one */
  void print_note(const errors::error &err) const;

public:
  impl(std::string_view source) : m_source(std::move(source)) {
#ifdef CASCADE_IS_WIN32
    HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD console_mode;
    GetConsoleMode(console_handle, &console_mode);
    console_mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

    if (!SetConsoleMode(console_handle, console_mode)) {
      auto e = GetLastError();

      fmt::print("Error while enabling ANSI support: {}\n", e);
    }
#endif
  }

  /** @brief Prints an error */
  void error(std::unique_ptr<errors::error> err);

  virtual void visit(errors::token_error &error) final;

  virtual void visit(errors::ast_error &error) final;

  virtual void visit(errors::type_error &error) final;
};

logger::logger(std::string_view source) : m_impl(std::make_unique<logger::impl>(source)) {}

logger::~logger() = default;

void logger::error(std::unique_ptr<errors::error> err) { m_impl->error(std::move(err)); }

std::string logger::impl::pretty_path(const errors::error &err) const {
  using namespace fmt::literals;

  auto path_line_col = fmt::format("{path}:{line}:{col}",
      "path"_a = err.path().string(),
      "line"_a = err.line(),
      "col"_a = err.column());

  return colors::cyan(path_line_col);
}

void logger::impl::print_start(const errors::error &e) const {
  using namespace fmt::literals;
  using namespace errors;

  auto [_, cols] = terminal_size();
  auto path = e.path().lexically_relative(fs::current_path()).string();
  auto msg = fmt::format("[E{:04}] {}!", to_num(e.code()), error_message_from_code(e.code()));

  // 10u is 7 characters for "error: " + 1 for ' '
  // if the path can fit on the current line without wrapping
  if (msg.size() + path.size() + 8u <= cols) {
    // error: {msg} {path}
    fmt::print("{} {} {}\n",
        formatted_error_tag(),
        colors::bold_white(msg),
        colors::bold_cyan(path));
  } else {
    fmt::print("{} {}\n", formatted_error_tag(), colors::bold_white(msg));
    fmt::print(" -> {}\n", colors::bold_cyan(path));
  }
}

void logger::impl::print_code(const errors::error &err) const {
  using namespace fmt::literals;

  // the lines without the number need to line up with the one that has it
  std::string padding(number_of_digits(err.line()), ' ');

  fmt::print(" {padding} {pipe}\n", "padding"_a = padding, "pipe"_a = colors::bold_black("|"));

  // err.position starts at 0, tok.column starts at 1. hence the +1
  auto line_start = (err.position() + 1) - err.column();

  fmt::print(" {line} {pipe} {source}\n",
      "line"_a = err.line(),
      "pipe"_a = colors::bold_black("|"),
      "source"_a = m_source.substr(line_start, m_source.find('\n', line_start) - line_start));
}

void logger::impl::point_out(const errors::error &err) const {
  using namespace fmt::literals;

  std::string pipe_padding(number_of_digits(err.line()), ' ');

  auto src_padding_len = err.column() - 1;

  if (err.code() == errors::error_code::unexpected_end_of_input) {
    src_padding_len += 1;
  }

  std::string src_padding(src_padding_len, ' ');

  // item could be multiple lines, so it gets the position of the line break
  auto len = m_source.find('\n', err.position()) - err.position();
  auto line = std::string{m_source.substr(err.position(), len)};

  // clang-format off
  line.erase(std::find_if(line.rbegin(), line.rend(), [](char c) {
    return !std::isspace(c);
  }).base(), line.end());
  // clang-format on

  // if it's only one line and m_source.find() returns string_view::npos, it gets
  // err.length()
  auto shortest = line.size() < err.length() ? line.size() : err.length();

  // if the length is 1, a ^ is used. Otherwise, ~~~s are put
  auto point_out = colors::bold_red(shortest == 1 ? "^" : std::string(shortest, '~'));

  fmt::print(" {pipe_padding} {pipe} {source_padding}{point_out}\n",
      "pipe_padding"_a = pipe_padding,
      "pipe"_a = colors::bold_black("|"),
      "source_padding"_a = src_padding,
      "point_out"_a = point_out);
}

void logger::impl::print_note(const errors::error &err) const {
  if (err.note()) {
    fmt::print("{} {}\n", colors::cyan("note:"), err.note().value());
  } else {
    auto result = errors::error_note_from_code(err.code());

    if (result) {
      fmt::print("{} {}\n", colors::cyan("note:"), result.value());
    }
  }
}

void logger::impl::error(std::unique_ptr<errors::error> err) { err->accept(*this); }

void logger::impl::visit(errors::token_error &err) {
  print_start(err);
  print_code(err);
  point_out(err);
  print_note(err);
  std::cout << "\n";
}

void logger::impl::visit(errors::ast_error &err) {
  print_start(err);
  print_code(err);
  point_out(err);
  print_note(err);
  std::cout << "\n";
}

void logger::impl::visit(errors::type_error &err) {
  print_start(err);
  print_code(err);
  point_out(err);
  print_note(err);
  std::cout << "\n";
}

void util::debug_print(std::vector<core::token> toks) {
  using namespace fmt::literals;

#ifndef NDEBUG // code is removed during dead-code elimination phase if the macro is defined
  if (toks.size() == 0) {
    return;
  }

  auto size = util::string_from_kind(toks.begin()->type()).size();

  for (auto &tok : toks) {
    auto tok_size = util::string_from_kind(tok.type()).size();

    size = tok_size > size ? tok_size : size;
  }

  for (auto &tok : toks) {
    auto type = util::string_from_kind(tok.type());

    // the fill constructor for std::string is truly magical
    std::string padding(size - type.size(), ' ');

    auto padded = fmt::format("{}{}", type, padding);

    fmt::print("{{ type: {}, p/l/c: {:04}:{:04}:{:03}, raw: '{}' }}\n",
        padded,
        tok.position(),
        tok.line(),
        tok.column(),
        tok.raw());
  }
#else
  (void)toks;
#endif
}

void util::debug_print(ast::node &node) {
#ifndef NDEBUG
  printer printer;

  node.accept(printer);
#else
  (void)node;
#endif
}

void util::debug_print(ast::program &prog) {
#ifndef NDEBUG
  std::cout << "program: {\n";

  printer printer;
  printer.m_prefix += "  ";

  for (auto &&decl : prog.decls()) {
    // need an initial prefix for all the nodes, since they assume they
    // get printed at the right column
    std::cout << "  ";
    decl->accept(printer);
  }

  std::cout << "}\n";
#else
  (void)prog;
#endif
}
