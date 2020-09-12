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
 * errors/error.cc:
 *   Implements the "error" objects.
 *
 *---------------------------------------------------------------------------*/

#include "errors/error.hh"

using namespace cascade;
using namespace errors;

std::unique_ptr<error> error::from(errors::error_code code, core::token tok, std::string note) {
  // I would change the return type to unique_ptr<token_error>
  // but it might change
  return std::make_unique<token_error>(code,
      tok,
      note == "" ? std::nullopt : std::make_optional(note));
}
