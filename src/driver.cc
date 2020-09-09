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
 * driver.cc:
 *   Provides definitions for the methods declared in driver.hh
 *
 *---------------------------------------------------------------------------*/

#include "driver.hh"
#include "core/lexer.hh"
#include "core/parser.hh"
#include "errors/error.hh"
#include "util/logging.hh"
#include "util/source_reader.hh"
#include <algorithm>
#include <iterator>
#include <queue>
#include <type_traits>

using namespace cascade;
namespace fs = std::filesystem;

driver::driver(int argc, const char **argv) : m_arg_parser(argc, argv) {}

std::optional<ast::program> driver::parse(stdpath path, std::string_view source) {
  std::vector<std::unique_ptr<errors::error>> errs;
  util::logger logger(source);

  auto report_err = [&errs](std::unique_ptr<errors::error> err) {
    // errors get passed in by the class calling this lambda
    errs.push_back(std::move(err));
  };

  auto tokens = core::lexer(source, path, report_err).lex();

#ifndef NDEBUG
  util::debug_print(tokens);

  auto parsed = core::parser(tokens, report_err).parse();
#else
  auto parsed = core::parser(std::move(tokens), report_err).parse();
#endif

  if (errs.size() != 0) {
    using err_ptr = std::unique_ptr<errors::error>;
    using moveit = std::move_iterator<std::vector<err_ptr>::iterator>;

    // can't use a normal for..in with non-copyable objects
    std::for_each(moveit(errs.begin()), moveit(errs.end()), [&logger](err_ptr err) {
      // aren't move iterators beautiful?
      logger.error(std::move(err));
    });
  }

  return (errs.size() == 0) ? std::make_optional(std::move(parsed)) : std::nullopt;
}

void driver::compile(stdpath path, ast::program prog) {
  (void)path;
  (void)prog;
}

int driver::run() {
  auto args_option = m_arg_parser.parse();

  // the arg parser will log an error if there was an issue
  if (!args_option) {
    return -1;
  }

  auto args = args_option.value();

  // if no files are passed in, input is read from stdin
  auto sources = args.files().size() == 0 ? util::read_source<util::pipe_reader>(args)
                                          : util::read_source<util::file_reader>(args);

  // same w/ source readers, they will inform on the issue if they have one
  if (!sources) {
    return -1;
  }

  std::vector<ast::program> programs;
  auto has_failed = false;

  for (auto &file : sources.value()) {
    auto parsed = parse(file.path(), file.source());

    if (parsed) {
      util::debug_print(parsed.value());

      programs.push_back(std::move(parsed.value()));
    } else {
      has_failed = true;
    }
  }

  return has_failed;
}
