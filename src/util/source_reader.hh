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
 * util/source_reader.hh:
 *   Defines the `source_reader` class, and the different policies on how to
 *   do the reading
 *
 *---------------------------------------------------------------------------*/

#ifndef CASCADE_UTIL_SOURCE_READER_HH
#define CASCADE_UTIL_SOURCE_READER_HH

#include "util/argument_parser.hh"
#include <filesystem>
#include <optional>
#include <string>
#include <type_traits>
#include <vector>

namespace cascade::util {
  namespace detail {
    /** @brief Represents a file that was successfully read */
    class file_source {
      std::filesystem::path m_path;
      std::string m_source;

      friend void normalize(file_source &);

    public:
      file_source(std::filesystem::path path, std::string source)
          : m_path(std::move(path))
          , m_source(std::move(source)) {}

      /**
       * @brief Returns the source code
       * @return A string_view to the source code for the file
       */
      std::string_view source() const { return m_source; }

      /**
       * @brief Returns the path
       * @return A copy of the path
       */
      std::filesystem::path path() const { return m_path; }
    };

    /**
     * @brief "Normalizes" a string by turning it into UTF-8 LF
     * @param ref The file to normalize
     */
    void normalize(file_source &ref);
  } // namespace detail

  /**
   * @brief Marker type for a type that can be used to read source code.
   * @details Requires a static method called "read" that returns an
   * std::string.
   */
  template <class T> class source_reading_policy {
  public:
    using opt_file_list = std::optional<std::vector<detail::file_source>>;
    using options = argument_parser::options;

    /**
     * @brief The function that util::read_source calls
     * @details Calls `read` on the type T and returns the result
     * @param options The options to pass to the policy's `read` method
     * @return The result of the policy's `read` method
     */
    static opt_file_list read_source(options &options) { return T::read(options); }
  };

  /** @brief Reads spirce from a file */
  class file_reader : public source_reading_policy<file_reader> {
  public:
    /**
     * @brief Opens a file, reads the source from it, and returns it
     * @param options The program options
     * @return The source string(s)
     */
    static opt_file_list read(options &options);
  };

  /** @brief Reads source from input piped into the program */
  class pipe_reader : public source_reading_policy<pipe_reader> {
  public:
    /**
     * @brief Reads from the stdin pipe
     * @param options The program arguments
     * @return The source string(s)
     */
    static opt_file_list read(options &options);
  };

  /**
   * @brief Attempts to read a file (or list of files) based off of the options
   * @param options The options given to the program
   * @return A list of source code strings
   */
  template <class T>
  std::optional<std::vector<detail::file_source>> read_source(argument_parser::options &options) {
    // only types that actually implement `read` can be used here
    static_assert(std::is_base_of_v<source_reading_policy<T>, T>,
        "T must fufill the sourcereader policy!");

    auto source = source_reading_policy<T>::read_source(options);

    if (!source) {
      return std::nullopt;
    }

    return source;
  }
} // namespace cascade::util

#endif
