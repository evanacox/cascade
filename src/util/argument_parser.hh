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
 * util/argument_parser.hh:
 *   Defines the `argument_parser` class used by the driver to guide compilation
 *
 *---------------------------------------------------------------------------*/

#ifndef CASCADE_UTIL_ARGUMENT_PARSER_HH
#define CASCADE_UTIL_ARGUMENT_PARSER_HH

#include "util/mixins.hh"
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace cascade::util {
  /** @brief Represents the optimization levels */
  enum class optimization_level {
    /** @brief Optimizations disabled. Think -O0 */
    disabled,
    /** @brief Optimizations enabled, but not aggresive ones. Think -O1 */
    enabled,
    /** @brief Optimizations enabled, and are very aggressive. Think -O3 */
    aggressive,
    /** @brief Optimizations enabled, including unsafe ones. Think -Ofast */
    unsafe
  };

  /** @brief Represents the type of output the compiler will generate */
  enum class emitted {
    /** @brief Emits LLVM IR from the compiler */
    llvm_ir,
    /** @brief Emits LLVM bitcode from the compiler */
    llvm_bc,
    /** @brief Emits assembly syntax */
    assembly,
    /** @brief Emits an object file */
    object,
    /** @brief Emits an executable */
    executable
  };

  /** @brief Represents the options passed to the compiler */
  class compilation_options {
    /**
     * @brief List of files to compile.
     * IF THE LIST IS EMPTY, PIPED INPUT IS ASSUMED
     */
    std::vector<std::string> m_files;

    /** @brief The optimization level given */
    optimization_level m_opt_level;

    /** @brief Whether or not to include debug symbols */
    bool m_debug_symbols = false;

    /** @brief What time of output the compiler is generating */
    emitted m_to_emit;

    /** @brief The LLVM target triple to generate for */
    std::string m_target_triple;

    /** @brief The output file */
    std::string m_output;

  public:
    /**
     * @brief Creates a new compilation_options object
     * @param files The list of files to compile
     * @param opt_level The optimization level
     * @param debug_symbols Whether or not debug symbols are enabled
     * @param emitted The form to emit the output in
     * @param triple The target triple
     * @param output The file to output to
     */
    explicit compilation_options(std::vector<std::string> files,
        optimization_level opt_level,
        bool debug_symbols,
        emitted to_emit,
        std::string triple,
        std::string output);

    /**
     * @brief Returns a list of files to compile. If the list is empty,
     * input is from a pipe
     * @return The list of files
     */
    const std::vector<std::string> &files() const { return m_files; }

    /**
     * @brief Returns the optimization level
     * @return The optimization level
     */
    optimization_level opt_level() const { return m_opt_level; }

    /**
     * @brief Returns whether debug symbols are enabled
     * @return Whether debug symbols are enabled
     */
    bool debug_symbols() const { return m_debug_symbols; }

    /**
     * @brief Returns the form code is emitted in
     * @return The form code is emitted in
     */
    emitted to_emit() const { return m_to_emit; }

    /**
     * @brief Returns the LLVM target triple
     * @return a string_view to the target triple
     */
    std::string_view target() const { return m_target_triple; }

    /**
     * @brief Returns the output file
     * @return The output file
     */
    std::string_view output() const { return m_output; }
  };

  /**
   * @brief Handles parsing the arguments for the program, and turns it into an
   * easier to use format
   */
  class argument_parser {
    /** @brief The number of arguments given to the program */
    int m_argc;

    /** @brief Pointer to the argument list */
    const char **m_argv;

  public:
    /** @brief The type returned by the `parse` function */
    using options = compilation_options;

    /**
     * @brief Creates a new argument parser
     * @param argc The number of arguments given to the program
     * @param argv Pointer to the argument list
     */
    explicit argument_parser(int argc, const char **argv);

    /**
     * @brief Parses the program arguments
     * @return The options given to the compiler
     */
    std::optional<options> parse();
  };
} // namespace cascade::util

#endif
