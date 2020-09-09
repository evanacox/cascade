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

#include "ast/detail/nodes.hh"
#include "core/lexer.hh"
#include <memory>

namespace cascade::ast {
  /** @brief Represents the "type" of reference, meaning `&T` vs `&mut T` */
  enum class reference_type : bool { ref, mut_ref };

  /** @brief Represents the type of pointer it is (meaning *T vs *mut T) */
  enum class pointer_type : bool { ptr, mut_ptr };

  /** @brief What type of number a builtin is */
  enum class numeric_type : char { boolean, integer, unsigned_integer, floating_point };

  /** @brief Represents a reference type */
  class reference : public type_base {
    /** @brief The type of reference */
    reference_type m_reftype;

    /** @brief The type the reference refers to */
    std::unique_ptr<type_base> m_type;

  public:
    /**
     * @brief Creates a reference type
     * @param info The source info for this reference specifically
     * @param reftype The type of reference it is
     * @param held Pointer to the type being referenced
     */
    explicit reference(
        core::source_info info, reference_type reftype, std::unique_ptr<type_base> held)
        : type_base(kind::type_ref, std::move(info)), m_reftype(reftype), m_type(std::move(held)) {}

    /**
     * @brief Returns the type of reference it is
     * @return The type of reference
     */
    [[nodiscard]] reference_type ref_type() const { return m_reftype; }

    /**
     * @brief Returns the type being referenced by the ref
     * @return The type being referenced
     */
    [[nodiscard]] type_base &held() const { return *m_type; }
  };

  /** @brief Represents a pointer type */
  class pointer : public type_base {
    pointer_type m_ptrtype;

    /** @brief The type the pointer points to */
    std::unique_ptr<type_base> m_type;

  public:
    /**
     * @brief Creates the pointer type
     * @param info The source info for this pointer specifically
     * @param ptrtype What type of pointer it is
     * @param held Pointer to the type being pointed to
     */
    explicit pointer(core::source_info info, pointer_type ptrtype, std::unique_ptr<type_base> held)
        : type_base(kind::type_ptr, std::move(info)), m_ptrtype(ptrtype), m_type(std::move(held)) {}

    /**
     * @brief Returns the type of pointer it is
     * @return The type of pointer
     */
    [[nodiscard]] pointer_type ptr_type() const { return m_ptrtype; }

    /**
     * @brief Returns the type being pointed to
     * @return The type being pointed to
     */
    [[nodiscard]] type_base &held() const { return *m_type; }
  };

  /** @brief Represents an array type */
  class array : public type_base {
    /** @brief The number of elements in the array, if 0 it's not  */
    std::size_t m_length;

    std::unique_ptr<type_base> m_type;

  public:
    /**
     * @brief Creates the array type
     * @param info The source info for this array specifically
     * @param len The length of the array
     * @param held Pointer to the type being pointed to
     */
    explicit array(core::source_info info, std::size_t len, std::unique_ptr<type_base> held)
        : type_base(kind::type_array, std::move(info)), m_length(len), m_type(std::move(held)) {}

    /**
     * @brief Gets the length of the array, if any
     * @return The length
     */
    [[nodiscard]] std::size_t length() const { return m_length; }

    /**
     * @brief Sets the length of the array
     * @param n The new length
     */
    void length(std::size_t n) { m_length = n; }

    /**
     * @brief Returns the type the array members are
     * @return The type of the array members
     */
    [[nodiscard]] type_base &held() const { return *m_type; }
  };

  /** @brief Represents a builtin type */
  class builtin : public type_base {
    std::size_t m_width;

    numeric_type m_numeric_type;

  public:
    /**
     * @brief Creates a builtin type
     * @param info The source info for the builtin type signature
     * @param width The bitwise width of the builtin
     * @param n_type What type of builtin it is
     */
    explicit builtin(core::source_info info, std::size_t width, numeric_type n_type)
        : type_base(kind::type_builtin, std::move(info)), m_width(width), m_numeric_type(n_type) {}

    /**
     * @brief Returns the width of the builtin
     * @return The width
     */
    [[nodiscard]] std::size_t width() const { return m_width; }

    /**
     * @brief Returns what type of number the bits should be interpreted as
     * @return The type of builtin
     */
    [[nodiscard]] numeric_type num_type() const { return m_numeric_type; }
  };

  /** @brief Represents a UDT */
  class user_defined : public type_base {
    std::string m_name;

  public:
    /**
     * @brief Creates a UDT
     * @param info Source info for the type signature
     * @param name The name of the UDT
     */
    explicit user_defined(core::source_info info, std::string name)
        : type_base(kind::type_userdef, std::move(info)), m_name(std::move(name)) {}

    /**
     * @brief Returns the name of the UDT
     * @return A string_view to the name
     */
    [[nodiscard]] std::string_view name() const { return m_name; }
  };

  /** @brief Serves as a marker for a type that the user left implied */
  class implied : public type_base {
  public:
    /**
     * @brief Creates an implied instance
     * @param info The source info for the location where the type **would be**.
     */
    explicit implied(core::source_info info) : type_base(kind::type_implied, std::move(info)) {}
  };

  /** @brief Serves as a marker for something that doesn't really *have* a type */
  class void_type : public type_base {
  public:
    /**
     * @brief Creates a void_type instance
     * @param info The source info for the location where the type **would be**.
     */
    explicit void_type(core::source_info info) : type_base(kind::type_void, std::move(info)) {}
  };
} // namespace cascade::ast

#endif
