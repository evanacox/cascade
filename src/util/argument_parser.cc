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
 * util/argument_parser.cc:
 *   Implements the `argument_parser` class used by the driver to guide
 *   compilation
 *
 *---------------------------------------------------------------------------*/

#include "cascade/util/argument_parser.hh"
#include "cascade/util/logging.hh"
#include "cxxopts.hpp"
#include "llvm/Support/Host.h"

using namespace cascade::util;

namespace fs = std::filesystem;
using ap = argument_parser;
using options = ap::options;

static std::optional<emitted> emitted_from_string(const std::string &input) {
  if (input == "llvm_ir")
    return emitted::llvm_ir;
  else if (input == "exe")
    return emitted::executable;
  else if (input == "att_asm")
    return emitted::att_asm;
  else if (input == "intel_asm")
    return emitted::intel_asm;
  else if (input == "default_asm")
    return emitted::default_asm;
  else if (input == "object")
    return emitted::object;
  else
    return std::nullopt;
}

static std::optional<optimization_level> optimization_from_int(int input) {
  switch (input) {
    case 0:
      return optimization_level::disabled;
    case 1:
      return optimization_level::enabled;
    case 2:
      return optimization_level::aggressive;
    case 3:
      return optimization_level::unsafe;
    default:
      return std::nullopt;
  }
}

#ifdef _WIN32
static constexpr auto default_output = "main.exe";
#else
static constexpr auto default_output = "main";
#endif

options::compilation_options(std::vector<std::string> paths,
    optimization_level opt_level, bool debug_symbols, emitted emitted,
    std::string triple, std::string output)
    : m_files(std::move(paths)),
      m_opt_level(opt_level),
      m_debug_symbols(debug_symbols),
      m_to_emit(emitted),
      m_target_triple(std::move(triple)),
      m_output(std::move(output)) {}

ap::argument_parser(int argc, const char **argv) : m_argc{argc}, m_argv{argv} {}

std::optional<options> ap::parse() {
  auto default_triple = llvm::sys::getDefaultTargetTriple();

  cxxopts::Options console_options(
      "cascade", "Compiler for the Cascade language");
  console_options.add_options()
      //
      ("d,debug", "Whether or not to include debug symbols",
          cxxopts::value<bool>()->default_value("false"))
      //
      ("o,optimize",
          "What level of optimization to do. Levels: 0 (disabled), 1 "
          "(non-aggressive), 2 (aggressive), 3 (extremely aggressive, enables "
          "unsafe optimizations). Defaults to 0",
          cxxopts::value<int>()->default_value("0"))
      //
      ("e,emitted",
          "What kind of output to emit (Options: 'llvm_ir', 'att_asm', "
          "'intel_asm', 'object', 'exe', 'default_asm'. Defaults to 'llvm_ir')",
          cxxopts::value<std::string>()->default_value("llvm_ir"))
      //
      ("output", "File to put the output in (defaults to 'main' or 'main.exe')",
          cxxopts::value<std::string>()->default_value(default_output))
      //
      ("target", "The LLVM target triple to target",
          cxxopts::value<std::string>()->default_value(default_triple))
      //
      ("h,help", "Print usage")
      //
      ("input-files", "The files to compile",
          cxxopts::value<std::vector<std::string>>(), "input-files");

  try {
    console_options.parse_positional({"input-files"});
    auto result = console_options.parse(m_argc, m_argv);

    if (result.count("help")) {
      std::cout << console_options.help() << "\n";

      return std::nullopt;
    }

    auto debug = result["debug"].as<bool>();
    auto opt_level = optimization_from_int(result["optimize"].as<int>());

    if (!opt_level) {
      util::error(
          "Unknown optimization level! Accepted options: '0', '1', '2', '3'");

      return std::nullopt;
    }

    auto emitted = emitted_from_string(result["emitted"].as<std::string>());

    if (!emitted) {
      util::error(
          "Unknown output form! Accepted options: 'llvm_ir', 'att_asm', "
          "'intel_asm', 'object', 'exe', 'default_asm'");

      return std::nullopt;
    }

    auto output = result["output"].as<std::string>();
    auto target = result["target"].as<std::string>();

    try {
      auto files = result["input-files"].as<std::vector<std::string>>();

      return std::make_optional<options>(options(
          files, opt_level.value(), debug, emitted.value(), target, output));
    } catch (cxxopts::OptionException &e) {
      util::error("No input files!");

      return std::nullopt;
    }
  } catch (const cxxopts::OptionException &err) {
    util::error(std::string("Error while parsing options: ") + err.what());
    return std::nullopt;
  }
}