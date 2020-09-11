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

#include "util/source_reader.hh"
#include "util/logging.hh"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <stdexcept>

using namespace cascade::util;
using opt_file_list = file_reader::opt_file_list;
using options = file_reader::options;
namespace fs = std::filesystem;

void detail::normalize(detail::file_source &ref) {
  ref.m_path = ref.m_path.lexically_normal().lexically_relative(fs::current_path());

  // transform CRLF into LF
  ref.m_source.erase(
      std::remove(ref.m_source.begin(), ref.m_source.end(), '\r'), ref.m_source.end());
}

/**
 * @brief Checks if an input string is valid UTF-8
 * @param str A string_view to the string to check
 * @return Whether or not the string is valid UTF-8
 */
[[nodiscard]] static bool is_valid_utf8(std::string_view str) {
#pragma message("is_valid_utf8: always returns true!")

  (void)str;

  return true;
}

#include <iostream>

opt_file_list file_reader::read(options &opts) {
  std::vector<detail::file_source> sources;

  // if any files have an error, no file contents are returned
  auto had_error = false;

  for (auto &file_path : opts.files()) {
    fs::path path(fs::absolute(file_path));

    // user could pass a non-existent path
    if (!fs::exists(path)) {
      had_error = true;
      util::error(file_path + ": No such file or directory!");

      continue;
    }

    // compiler doesn't deal w/ binary files, or with symlinks/pipes/whatever
    if (!fs::is_regular_file(path)) {
      had_error = true;
      util::error(file_path + ": File is not a regular file!");

      continue;
    }

    std::ifstream stream(file_path);

    if (!stream.is_open()) {
      had_error = true;
      util::error(file_path + ": Unable to open file!");
      stream.close();

      continue;
    }

    std::string str;
    stream.seekg(0, std::ios_base::end);
    auto length = stream.tellg();
    str.resize(length);

    std::cout << "length: " << length << "\n";

    stream.seekg(0, std::ios::beg);
    stream.read(str.data(), length);

    if (!is_valid_utf8(str)) {
      had_error = true;
      util::error(file_path + ": File is not valid UTF-8!");

      continue;
    }

    sources.emplace_back(std::move(path), std::move(str));
  }

  if (had_error) {
    return std::nullopt;
  } else {
    return sources;
  }
}

opt_file_list pipe_reader::read([[maybe_unused]] options &opts) {
  throw std::logic_error{"Not implemented!"};
}
