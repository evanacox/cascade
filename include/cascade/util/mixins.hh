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
 * util/mixins.hh:
 *   Defines some mixins that classes throughout the project can use
 *
 *---------------------------------------------------------------------------*/

#ifndef CASCADE_UTIL_MIXINS_HH
#define CASCADE_UTIL_MIXINS_HH

namespace cascade::util {
  /** @brief Disables copying on an object */
  class noncopyable {
  protected:
    constexpr noncopyable() = default;

    ~noncopyable() = default;

    noncopyable(const noncopyable &) = delete;

    noncopyable &operator=(const noncopyable &) = delete;
  };

  /** @brief Disables moving on an object */
  class nonmovable {
  protected:
    constexpr nonmovable() = default;

    ~nonmovable() = default;

    nonmovable(nonmovable &&) = delete;

    nonmovable &operator=(nonmovable &&) = delete;
  };
} // namespace cascade::util

#endif