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

#include <string>

namespace cascade::util {
  namespace colors {
    /** @brief Bright red color code */
    constexpr auto code_bold_red = "\u001b[31;1m";

    /** @brief Red color code */
    constexpr auto code_red = "\u001b[31m";

    /** @brief Bright blue color code */
    constexpr auto code_bold_blue = "\u001b[34;1m";

    /** @brief Bright white color code */
    constexpr auto code_bold_white = "\u001b[37;1m";

    /** @brief Code to reset colors to default */
    constexpr auto code_reset = "\u001b[0m";

    /** @brief Code to a dark gray color */
    constexpr auto code_dark_gray = "\u001b[30;1m"; // "\u001b[38;5;240m";

    /** @brief Code to the standard cyan color */
    constexpr auto code_cyan = "\u001b[36m";

    /** @brief Code to the bold yellow color */
    constexpr auto code_bold_yellow = "\u001b[33;1m";

    /**
     * @brief Returns the string passed but with red and reset codes
     * added to the beginning and end.
     * @param message The message to modify
     * @return The message, but with ANSI codes for red added
     */
    inline std::string red(std::string message) {
      return code_red + message + code_reset;
    }

    /**
     * @brief Returns the string passed but with red and reset codes
     * added to the beginning and end.
     * @param message The message to modify
     * @return The message, but with ANSI codes for bold red added
     */
    inline std::string bold_red(std::string message) {
      return code_bold_red + message + code_reset;
    }

    /**
     * @brief Returns the string passed but with blue and reset codes
     * added to the beginning and end.
     * @param message The message to modify
     * @return The message, but with ANSI codes for bold blue added
     */
    inline std::string bold_blue(std::string message) {
      return code_bold_blue + message + code_reset;
    }

    /**
     * @brief Returns the string passed but with white and reset codes
     * added to the beginning and end.
     * @param message The message to modify
     * @return The message, but with ANSI codes for bold white added
     */
    inline std::string bold_white(std::string message) {
      return code_bold_white + message + code_reset;
    }

    /**
     * @brief Returns the string passed but with gray and reset codes
     * added to the beginning and end.
     * @param message The message to modify
     * @return The message, but with ANSI codes for dark gray added
     */
    inline std::string dark_gray(std::string message) {
      return code_dark_gray + message + code_reset;
    }

    /**
     * @brief Returns the string passed but with cyan and reset codes
     * added to the beginning and end.
     * @param message The message to modify
     * @return The message, but with ANSI codes for cyan added
     */
    inline std::string cyan(std::string message) {
      return code_cyan + message + code_reset;
    }

    /**
     * @brief Returns the string passed but with bold yellow and reset codes
     * added to the beginning and end.
     * @param message The message to modify
     * @return The message, but with ANSI codes for bold yellow added
     */
    inline std::string bold_yellow(std::string message) {
      return code_bold_yellow + message + code_reset;
    }
  } // namespace colors

  /**
   * @brief Prints an error message not associated with code
   * @param message The message to print
   */
  void error(std::string message);
} // namespace cascade::util