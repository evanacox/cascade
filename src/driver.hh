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
 * driver.hh:
 *   Defines the `driver` class that runs the compiler.
 *
 *---------------------------------------------------------------------------*/

#ifndef CASCADE_DRIVER_HH
#define CASCADE_DRIVER_HH

#include "ast/ast.hh"
#include "util/argument_parser.hh"
#include "util/mixins.hh"
#include <filesystem>
#include <optional>

namespace cascade {
  /**
   * @brief Class that "drives" the compiler,
   * @details Takes the results from the various stages of compiilation
   * and passes them to the next stage.
   */
  class driver : util::noncopyable {
    using stdpath = std::filesystem::path;

    util::argument_parser m_arg_parser;

    /**
     * @brief Attempts to parse a source string
     * @param path Path to the file being parsed
     * @param source The source code
     * @return An ast::program
     */
    [[nodiscard]] std::optional<ast::program> parse(stdpath path, std::string_view source);

    /**
     * @brief Attempts to compile an AST
     * @param path Path to the file being compiled
     * @param prog The AST of the program being compiled
     */
    void compile(stdpath path, ast::program prog);

  public:
    /** @brief Disallow default construction */
    driver() = delete;

    /**
     * @brief Creates a new driver
     * @param argc The number of arguments passed to the program
     * @param argv Pointer to the argument list given to `main`
     */
    explicit driver(int argc, const char **argv);

    /**
     * @brief The entry-point to the program, meant to be immediately called by
     * main
     * @details This function acts as the *real* point of entry to the program
     * that main immediately calls after gaining control
     * @return Returns an exit code that `main` can return
     */
    [[nodiscard]] int run();
  };
} // namespace cascade

#endif
