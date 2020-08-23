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
 * util/logging.hh:
 *   Defines a logging utility for the rest of the compiler to use
 *
 *---------------------------------------------------------------------------*/

#ifndef CASCADE_UTIL_LOGGING_HH
#define CASCADE_UTIL_LOGGING_HH

#include "cascade/errors/error.hh"
#include "cascade/errors/error_visitor.hh"
#include <memory>
#include <string>
#include <vector>

namespace cascade::util {
  namespace colors {
    /** @brief Black ANSI code */
    constexpr auto code_black = "\u001b[30m";

    /** @brief Red ANSI code */
    constexpr auto code_red = "\u001b[31m";

    /** @brief Green ANSI code */
    constexpr auto code_green = "\u001b[32m";

    /** @brief Yellow ANSI code */
    constexpr auto code_yellow = "\u001b[33m";

    /** @brief Blue ANSI code */
    constexpr auto code_blue = "\u001b[34m";

    /** @brief Magenta ANSI code */
    constexpr auto code_magenta = "\u001b[35m";

    /** @brief Cyan ANSI code */
    constexpr auto code_cyan = "\u001b[36m";

    /** @brief White ANSI code */
    constexpr auto code_white = "\u001b[37m";

    /** @brief Reset ANSI code */
    constexpr auto code_reset = "\u001b[0m";

    /** @brief Bright Black ANSI code */
    constexpr auto code_bold_black = "\u001b[30;1m";

    /** @brief Bright Red ANSI code */
    constexpr auto code_bold_red = "\u001b[31;1m";

    /** @brief Bright Green ANSI code */
    constexpr auto code_bold_green = "\u001b[32;1m";

    /** @brief Bright Yellow ANSI code */
    constexpr auto code_bold_yellow = "\u001b[33;1m";

    /** @brief Bright Blue ANSI code */
    constexpr auto code_bold_blue = "\u001b[34;1m";

    /** @brief Bright Magenta ANSI code */
    constexpr auto code_bold_magenta = "\u001b[35;1m";

    /** @brief Bright Cyan ANSI code */
    constexpr auto code_bold_cyan = "\u001b[36;1m";

    /** @brief Bright White ANSI code */
    constexpr auto code_bold_white = "\u001b[37;1m";

#define COLOR_FUNC(color)                                                                          \
  inline std::string color(std::string message) { return code_##color + message + code_reset; }    \
  inline std::string bold_##color(std::string message) {                                           \
    return code_bold_##color + message + code_reset;                                               \
  }

    COLOR_FUNC(black)
    COLOR_FUNC(red)
    COLOR_FUNC(green)
    COLOR_FUNC(yellow)
    COLOR_FUNC(blue)
    COLOR_FUNC(magenta)
    COLOR_FUNC(cyan)
    COLOR_FUNC(white)

#undef COLOR_FUNC
  } // namespace colors

  class logger {
    class impl;

    std::unique_ptr<impl> m_impl;

  public:
    /**
     * @brief Creates a logger instance
     * @param source The source code to use to pretty-print errors
     */
    logger(std::string_view source);

    /**
     * @brief Pretty-prints an error
     * @param error The error to print
     */
    void error(std::unique_ptr<errors::error> error);

    ~logger();
  };

  /**
   * @brief Prints an error message not associated with code
   * @param message The message to print
   */
  void error(std::string_view message);

  /**
   * @brief Pretty-prints a list of tokens
   * @param toks The list to print
   */
  void debug_print(std::vector<core::token> toks);
} // namespace cascade::util

#endif