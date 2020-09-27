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
 * util/types.cc:
 *   Implements the helper functions for working on AST types
 *
 *---------------------------------------------------------------------------*/

#include "util/types.hh"
#include "ast/detail/types.hh"
#include <cassert>
#include <fmt/format.h>
#include <iostream>
#include <type_traits>
#include <variant>

using namespace cascade;
using kind = ast::kind;

template <class> inline constexpr bool always_false_v = false;

std::string util::to_string(const ast::type_data &node) {
  using type_base = ast::type_data::type_base;

  if (node.is(type_base::implied)) {
    return "<implied>";
  } else if (node.is(type_base::void_type)) {
    return "<void>";
  } else if (node.is(type_base::error_type)) {
    return "<error-type>";
  }

  std::string str;

  for (auto mod : node.modifiers()) {
    using modif = ast::type::type_modifiers;

    switch (mod) {
      case modif::ref:
        str += "&";
        break;
      case modif::mut_ref:
        str += "&mut ";
        break;
      case modif::ptr:
        str += "*";
        break;
      case modif::mut_ptr:
        str += "*mut ";
        break;
      case modif::array:
        str += "[]";
        break;
    }
  }

  // clang-format off
  std::visit([&](auto &&data) {
    using T = std::decay_t<decltype(data)>;
    using base = ast::type_data::type_base;

    if constexpr (std::is_same_v<T, std::size_t>) {               
      // C++ needs pattern matching
      switch (node.base()) {
        case base::integer:
          str += "i";
          str += std::to_string(data);
          break;
        case base::unsigned_integer:
          str += "u";
          str += std::to_string(data);
          break;
        case base::boolean:
          assert(data == 1 && "bool shouldn't have precision");
          str += "bool";
          break;
        case base::floating_point:
          str += "f";
          str += std::to_string(data);
          break;
        default:
          assert(false && "how exactly did we get here");
      }
    } else if constexpr (std::is_same_v<T, std::string>) {
      assert(node.base() == base::user_defined);
      str += data;
    } else {
      // will show the type that caused the failure
      static_assert(always_false_v<T>, "util::to_string: non-exhaustive visitor");
    }
  }, node.data());
  // clang-format on

  return str;
}

std::size_t util::hash(const ast::type_data &node) {
  return std::hash<std::string>{}(util::to_string(node));
}
