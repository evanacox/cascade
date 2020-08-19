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
#include <fstream>
#include <iostream>

using namespace cascade::util;

static std::string executable_name() {
#if defined(PLATFORM_POSIX) || defined(__linux__) || defined(__unix__)
  std::string sp;
  std::ifstream("/proc/self/comm") >> sp;
  return sp;
#elif defined(_WIN32)
  std::string str(' ', MAX_PATH);
  GetModuleFileNameA(nullptr, &str[0], MAX_PATH);
  return buf;
#else
  static_assert(false, "Unrecognized platform!");
#endif
}

static void output_exe_name() {
  std::cout << colors::cyan(executable_name()) << ' ';
}

static void output_error_tag() { std::cout << colors::bold_red("error: "); }

[[maybe_unused]] static void output_warning_tag() {
  std::cout << colors::bold_yellow("warning: ");
}

void cascade::util::error(std::string message) {
  output_exe_name();

  output_error_tag();

  std::cout << colors::bold_white(message) << "\n";
}