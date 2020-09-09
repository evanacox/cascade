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
 * errors/error.hh:
 *   Defines A generic "error" object.
 *
 *---------------------------------------------------------------------------*/

#ifndef CASCADE_ERRORS_ERROR_VISITOR_HH
#define CASCADE_ERRORS_ERROR_VISITOR_HH

namespace cascade::errors {
  class error;
  class token_error;
  class ast_error;

  /** @brief Represents a visitor for errors */
  class error_visitor {
  public:
    /**
     * @brief Visits a token error
     * @param tok_error The token to visit
     */
    virtual void visit(token_error &tok_error) = 0;

    /**
     * @brief Visits a node error
     * @param node_err The node to visit
     */
    virtual void visit(ast_error &node_err) = 0;

    virtual ~error_visitor() {}
  };
} // namespace cascade::errors

#endif