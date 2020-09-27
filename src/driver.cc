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
#include "core/typechecker.hh"
#include "errors/error.hh"
#include "util/logging.hh"
#include "util/source_reader.hh"
#include <algorithm>
#include <iterator>
#include <memory>
#include <queue>
#include <type_traits>

using namespace cascade;
namespace fs = std::filesystem;

static void log_errors(std::vector<std::unique_ptr<errors::error>> errs, util::logger logger) {
  using err_ptr = std::unique_ptr<errors::error>;
  using moveit = std::move_iterator<std::vector<err_ptr>::iterator>;

  // can't use a normal for..in to move objects out
  std::for_each(moveit(errs.begin()), moveit(errs.end()), [&logger](err_ptr err) {
    // aren't move iterators beautiful?
    logger.error(std::move(err));
  });
}

driver::driver(int argc, const char **argv) : m_options(util::parse(argc, argv)) {}

std::optional<ast::program> driver::parse(stdpath path, std::string_view source) {
  std::vector<std::unique_ptr<errors::error>> errs;

  auto report_err = [&errs](std::unique_ptr<errors::error> err) {
    // errors get passed in by the class calling this lambda
    errs.emplace_back(std::move(err));
  };

  auto tokens = core::lexer(source, path, report_err).lex();
  util::debug_print(tokens);
  auto parsed = core::parse(std::move(tokens), report_err);

  log_errors(std::move(errs), util::logger(source));

  return (errs.size() == 0) ? std::make_optional(std::move(parsed)) : std::nullopt;
}

bool driver::parse(const std::vector<util::file_source> &files) {
  auto has_failed = false;

  for (const auto &file : files) {
    m_sources.push_back(file.source());

    auto parsed = parse(file.path(), file.source());

    if (parsed) {
      util::debug_print(parsed.value());

      m_programs.emplace_back(std::move(parsed.value()));
    } else {
      has_failed = true;
    }
  }

  return has_failed;
}

bool driver::typecheck() {
  std::vector<std::unique_ptr<errors::error>> errs;

  core::typecheck(m_programs, m_sources, [&errs](std::unique_ptr<errors::error> err) {
    // commenting to disallow some terrible formatting
    errs.emplace_back(std::move(err));
  });

  auto err_count = errs.size();

  using err_ptr = std::unique_ptr<errors::error>;
  using moveit = std::move_iterator<std::vector<err_ptr>::iterator>;

  // can't use a normal for..in to move objects out
  std::for_each(moveit(errs.begin()), moveit(errs.end()), [](err_ptr err) {
    auto error = static_cast<errors::type_error *>(err.get());

    // hacky solution to a problem I didn't anticipate having to deal with,
    // and one that would take a bunch of effort to shift things around
    // to work with
    util::logger(error->source()).error(std::move(err));
  });

  return err_count != 0;
}

void driver::compile(stdpath path, ast::program prog) {
  (void)path;
  (void)prog;
}

int driver::run() {
  // the arg parser will log an error if there was an issue
  if (!m_options) {
    return -1;
  }

  auto args = m_options.value();

  // if no files are passed in, input is read from stdin
  auto sources = args.files().size() == 0 ? util::read_source<util::pipe_reader>(args)
                                          : util::read_source<util::file_reader>(args);

  // same w/ source readers, they will inform on the issue if they have one
  if (!sources) {
    return -1;
  }

  if (parse(sources.value())) {
    return -2;
  }

  if (typecheck()) {
    return -3;
  }

  return 0;
}
