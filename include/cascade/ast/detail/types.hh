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
 * ast/detail/types.hh:
 *   Outlines the declaration AST nodes
 *
 *---------------------------------------------------------------------------*/

#ifndef CASCADE_AST_DETAIL_TYPES_HH
#define CASCADE_AST_DETAIL_TYPES_HH

#include "cascade/ast/detail/nodes.hh"
#include "cascade/core/lexer.hh"
#include <memory>

namespace cascade::ast {
  /** @brief Represents the "type" of reference, meaning `&T` vs `&mut T` */
  enum class reference_type : bool { ref, mut_ref };

  /** @brief Represents the type of pointer it is (meaning *T vs *mut T) */
  enum class pointer_type : bool { ptr, mut_ptr };

  /** @brief Represents a reference type */
  class reference : public type {
    /** @brief The type of reference */
    reference_type m_reftype;

    /** @brief The type the reference refers to */
    std::shared_ptr<type> m_type;

  public:
    explicit reference(core::source_info info, reference_type reftype, std::shared_ptr<type> held)
        : type(kind::type_ptr, std::move(info)), m_reftype(reftype), m_type(std::move(held)) {}

    /**
     * @brief Returns the type of reference it is
     * @return The type of reference
     */
    reference_type ref_type() const { return m_reftype; }

    /**
     * @brief Returns the type being referenced by the ref
     * @return The type being referenced
     */
    std::shared_ptr<type> held() const { return m_type; }
  };

  /** @brief Represents a pointer type */
  class pointer : public type {
    pointer_type m_ptrtype;

    /** @brief The type the pointer points to */
    std::shared_ptr<type> m_type;

  public:
    /**
     * @brief Creates the pointer type
     * @param info The source info for this pointer specifically
     * @param ptrtype What type of pointer it is
     * @param held Pointer to the type being pointed to
     */
    explicit pointer(core::source_info info, pointer_type ptrtype, std::shared_ptr<type> held)
        : type(kind::type_ptr, std::move(info)), m_ptrtype(ptrtype), m_type(std::move(held)) {}

    /**
     * @brief Returns the type of pointer it is
     * @return The type of pointer
     */
    pointer_type ptr_type() const { return m_ptrtype; }

    /**
     * @brief Returns the type being referenced by the ref
     * @return The type being referenced
     */
    std::shared_ptr<type> held() const { return m_type; }
  };
} // namespace cascade::ast

#endif