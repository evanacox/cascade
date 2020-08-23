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

#include "cascade/util/logging.hh"
#include "cascade/errors/error_lookup.hh"
#include "cascade/errors/error_visitor.hh"
#include "cascade/util/keywords.hh"
#include <fmt/format.h>
#include <fstream>

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
#else
  CONSOLE_SCREEN_BUFFER_INFO csbi;

  if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
    return {
        csbi.srWindow.Bottom - csbi.srWindow.Top + 1, csbi.srWindow.Right - csbi.srWindow.Left + 1};
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

void util::error(std::string_view message) {
  fmt::print("{} {} {}\n", formatted_exe_name(), formatted_error_tag(),
      colors::bold_white(std::string{message}));
}

void util::debug_print(std::vector<core::token> toks) {
  using namespace fmt::literals;

#ifndef NDEBUG // code is removed during dead-code elimination phase if the macro is defined
  auto size = util::string_from_kind(toks.begin()->type()).size();

  for (auto &tok : toks) {
    auto tok_size = util::string_from_kind(tok.type()).size();

    size = tok_size > size ? tok_size : size;
  }

  for (auto &tok : toks) {
    auto type = util::string_from_kind(tok.type());
    std::string padding(size - type.size(), ' ');

    auto padded = fmt::format("{}{}", type, padding);

    fmt::print("{{ type: {}, p/l/c: {:04}:{:04}:{:03}, raw: '{}' }}\n", padded, tok.position(),
        tok.line(), tok.column(), tok.raw());
  }
#else
  (void)toks;
#pragma message("util::debug_print: This function shouldn't be called in a non-debug build!")
#endif
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
  impl(std::string_view source) : m_source(std::move(source)) {}

  /** @brief Prints an error */
  void error(std::unique_ptr<errors::error> err);

  /** @brief Visits a token_error */
  virtual void visit(errors::token_error &error) final;

  /** @brief Visits a node_error */
  virtual void visit(errors::node_error &error) final;
};

logger::logger(std::string_view source) : m_impl(std::make_unique<logger::impl>(source)) {}

logger::~logger() = default;

void logger::error(std::unique_ptr<errors::error> err) { m_impl->error(std::move(err)); }

std::string logger::impl::pretty_path(const errors::error &err) const {
  using namespace fmt::literals;

  auto path_line_col = fmt::format("{path}:{line}:{col}", "path"_a = err.path().string(),
      "line"_a = err.line(), "col"_a = err.column());

  return colors::cyan(path_line_col);
}

void logger::impl::print_start(const errors::error &e) const {
  using namespace fmt::literals;
  using namespace errors;

  auto size = terminal_size();
  auto path = e.path().lexically_relative(fs::current_path()).string();
  auto msg = fmt::format("[E{:04}] {}!", to_num(e.code()), error_message_from_code(e.code()));

  // 10u is 7 characters for "error: " + 1 for ' '
  // if the path can fit on the current line without wrapping
  if (msg.size() + path.size() + 8u <= size.second) {
    // error: {msg} {path}
    fmt::print(
        "{} {} {}\n", formatted_error_tag(), colors::bold_white(msg), colors::bold_cyan(path));
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

  fmt::print(" {line} {pipe} {source}\n", "line"_a = err.line(), "pipe"_a = colors::bold_black("|"),
      "source"_a = m_source.substr(line_start, m_source.find('\n', line_start) - line_start));
}

void logger::impl::point_out(const errors::error &err) const {
  using namespace fmt::literals;

  std::string pipe_padding(number_of_digits(err.line()), ' ');
  std::string src_padding(err.column() - 1, ' ');

  // item could be multiple lines
  auto len = m_source.find('\n', err.position()) - err.position();
  auto shortest = len < err.length() ? len : err.length();

  auto point_out = colors::bold_red(shortest == 1 ? "^" : std::string(shortest, '~'));

  fmt::print(" {pipe_padding} {pipe} {source_padding}{point_out}\n",
      "pipe_padding"_a = pipe_padding, "pipe"_a = colors::bold_black("|"),
      "source_padding"_a = src_padding, "point_out"_a = point_out);
}

void logger::impl::print_note(const errors::error &err) const {
  using namespace fmt::literals;

  fmt::print("{} ", colors::cyan("note:"));

  if (err.note()) {
    fmt::print("{}\n", err.note().value());
  } else {
    auto result = errors::error_note_from_code(err.code());

    if (!result) {
      throw std::logic_error{"note not specified, and not found inside lookup"};
    }

    fmt::print("{}\n", result.value());
  }
}

void logger::impl::error(std::unique_ptr<errors::error> err) { err->accept(*this); }

void logger::impl::visit(errors::token_error &err) {
  using namespace fmt::literals;

  print_start(err);
  print_code(err);
  point_out(err);
  print_note(err);
  fmt::print("\n");
}

void logger::impl::visit(errors::node_error &err) {
  (void)err;
  throw std::logic_error{"Not implemented!"};
}