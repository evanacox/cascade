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

#ifndef CASCADE_ERRORS_ERROR_HH
#define CASCADE_ERRORS_ERROR_HH

#include "ast/ast.hh"
#include "core/lexer.hh"
#include "errors/error_lookup.hh"
#include "errors/error_visitor.hh"
#include <filesystem>
#include <memory>
#include <optional>

namespace cascade::errors {
  /** @brief Abstract error type */
  class error {
  public:
    /**
     * @brief Creates an error from a token
     * @param code The error's lookup code
     * @param tok The token to base the error off of
     * @return An error
     */
    static std::unique_ptr<error> from(error_code code, core::token tok, std::string note = "");

    /**
     * @brief Returns the error lookup code
     * @return The code as a short
     */
    [[nodiscard]] virtual error_code code() const = 0;

    /**
     * @brief Returns the error's offset in the source
     * @return An offset
     */
    [[nodiscard]] virtual std::size_t position() const = 0;

    /**
     * @brief Returns the line the error appears on
     * @return The line of the error
     */
    [[nodiscard]] virtual std::size_t line() const = 0;

    /**
     * @brief Returns the column of the error
     * @return The column number
     */
    [[nodiscard]] virtual std::size_t column() const = 0;

    /**
     * @brief Returns the number of characters in the source code for the error
     * @return The number of characters
     */
    [[nodiscard]] virtual std::size_t length() const = 0;

    /**
     * @brief Returns the path of the file the error originated from
     * @return The path of the error's file
     */
    [[nodiscard]] virtual std::filesystem::path path() const = 0;

    /**
     * @brief Returns a "note" to put at the bottom of the error
     * @return The note to put (if its there)
     */
    [[nodiscard]] virtual std::optional<std::string> note() const = 0;

    /**
     * @brief Allows visiting an error node
     * @param visitor The visitor to use
     */
    virtual void accept(error_visitor &visitor) = 0;

    /** @brief Pure virtual destructor */
    virtual ~error(){};
  };

  /** @brief Represents an error from the lexer */
  class token_error : public error {
    /** @brief The error code to get messages for */
    error_code m_code;

    /** @brief The line info used to print out the error */
    core::token m_token;

    /** @brief A helpful message to show under the error */
    std::optional<std::string> m_note;

  public:
    /**
     * @brief Creates a new error
     * @param code The error code
     * @param tok The offending token
     */
    explicit token_error(
        error_code code, core::token tok, std::optional<std::string> note = std::nullopt)
        : m_code(code), m_token(std::move(tok)), m_note(std::move(note)) {}

    /**
     * @brief Returns the error lookup code
     * @return The code as a short
     */
    [[nodiscard]] virtual error_code code() const final { return m_code; }

    /**
     * @brief Returns the error's offset in the source
     * @return An offset
     */
    [[nodiscard]] virtual std::size_t position() const final { return m_token.position(); }

    /**
     * @brief Returns the line the error appears on
     * @return The line of the error
     */
    [[nodiscard]] virtual std::size_t line() const final { return m_token.line(); }

    /**
     * @brief Returns the column of the error
     * @return The column number
     */
    [[nodiscard]] virtual std::size_t column() const final { return m_token.column(); }

    /**
     * @brief Returns the number of characters in the error
     * @return The number of characters
     */
    [[nodiscard]] virtual std::size_t length() const final { return m_token.length(); }

    /**
     * @brief Returns the entire source string causing the error
     * @param source The source code to get a substr of
     * @return A substring of @p source
     */
    [[nodiscard]] std::string_view raw() const { return m_token.raw(); }

    /**
     * @brief Returns the path of the error
     * @return The path of the file the error is in
     */
    [[nodiscard]] virtual std::filesystem::path path() const { return m_token.path(); }

    /**
     * @brief Returns the "note" message
     * @return The note message
     */
    [[nodiscard]] virtual std::optional<std::string> note() const { return m_note; }

    /**
     * @brief Accepts a visitor
     * @param visitor The visitor to visit
     */
    virtual void accept(error_visitor &visitor) final { visitor.visit(*this); }
  };

  class ast_error : public error {
    /** @brief Code of the error being printed */
    error_code m_code;

    /** @brief Pointer to the node */
    std::unique_ptr<ast::node> m_node;

    /** @brief A helpful message to show under the error */
    std::optional<std::string> m_note;

  public:
    /**
     * @brief Creates a new error
     * @param code The error code
     * @param tok The offending token
     */
    explicit ast_error(error_code code, std::unique_ptr<ast::node> node,
        std::optional<std::string> note = std::nullopt)
        : m_code(code), m_node(std::move(node)), m_note(std::move(note)) {}

    /**
     * @brief Returns the error lookup code
     * @return The code as a short
     */
    [[nodiscard]] virtual error_code code() const final { return m_code; }

    /**
     * @brief Returns the error's offset in the source
     * @return An offset
     */
    [[nodiscard]] virtual std::size_t position() const final { return m_node->info().position(); }

    /**
     * @brief Returns the line the error appears on
     * @return The line of the error
     */
    [[nodiscard]] virtual std::size_t line() const final { return m_node->info().line(); }

    /**
     * @brief Returns the column of the error
     * @return The column number
     */
    [[nodiscard]] virtual std::size_t column() const final { return m_node->info().column(); }

    /**
     * @brief Returns the number of characters in the error
     * @return The number of characters
     */
    [[nodiscard]] virtual std::size_t length() const final { return m_node->info().length(); }

    /**
     * @brief Returns the entire source string causing the error
     * @param source The source code to get a substr of
     * @return A substring of @p source
     */
    [[nodiscard]] std::string_view raw(std::string_view source) const {
      return source.substr(m_node->info().position(), m_node->info().length());
    }

    /**
     * @brief Returns the path of the error
     * @return The path of the file the error is in
     */
    [[nodiscard]] virtual std::filesystem::path path() const { return m_node->info().path(); }

    /**
     * @brief Returns the "note" message
     * @return The note message
     */
    [[nodiscard]] virtual std::optional<std::string> note() const { return m_note; }

    /**
     * @brief Accepts a visitor
     * @param visitor The visitor to visit
     */
    virtual void accept(error_visitor &visitor) final { visitor.visit(*this); }
  };
} // namespace cascade::errors

#endif
