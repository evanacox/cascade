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
 * util/source_reader.cc:
 *   Provides implementations for the different policies and the `normalize` fn
 *
 *---------------------------------------------------------------------------*/

#include "cascade/util/source_reader.hh"
#include <algorithm>
#include <stdexcept>

using namespace cascade::util;

using options = argument_parser::options;

void detail::normalize(std::string &ref) {
  ref.erase(std::remove(ref.begin(), ref.end(), '\r'), ref.end());
}

std::vector<std::string> file_reader::read(options &opts) {
  (void)opts;

  return {""};
}

std::vector<std::string> pipe_reader::read(options &opts) {
  if (opts.files().size() != 0) {
    throw std::logic_error{"Expected 0 files on a pipe input!"};
  }

  return {""};
}