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

using namespace cascade;
using kind = ast::kind;

void util::traverse_type(ast::type_base &node, std::function<void(ast::pointer &)> ptr_fn,
    std::function<void(ast::reference &)> ref_fn, std::function<void(ast::array &)> array_fn,
    std::function<void(ast::builtin &)> builtin_fn,
    std::function<void(ast::user_defined &)> userdef_fn) {

  auto *first = &node;

  while (!first->is_one_of(kind::type_builtin, kind::type_userdef)) {
    switch (first->raw_kind()) {
      case kind::type_ptr: {
        auto &ptr = static_cast<ast::pointer &>(*first);

        ptr_fn(ptr);

        assert(first != &ptr.held());

        first = &ptr.held();
        break;
      }
      case kind::type_ref: {
        auto &ref = static_cast<ast::reference &>(*first);

        ref_fn(ref);

        assert(first != &ref.held());

        first = &ref.held();
        break;
      }
      case kind::type_array: {
        auto &arr = static_cast<ast::array &>(*first);

        array_fn(arr);

        assert(first != &arr.held());

        first = &arr.held();
        break;
      }
      default:
        assert(false);
    }
  }

  if (first->is(kind::type_builtin)) {
    builtin_fn(static_cast<ast::builtin &>(*first));
  } else if (first->is(kind::type_userdef)) {
    userdef_fn(static_cast<ast::user_defined &>(*first));
  } else {
    assert(false);
  }
}
