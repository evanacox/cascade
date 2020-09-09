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
 * ast/detail/declarations.hh:
 *   Outlines the declaration AST nodes
 *
 *---------------------------------------------------------------------------*/

#ifndef CASCADE_AST_DETAIL_DECLARATIONS_HH
#define CASCADE_AST_DETAIL_DECLARATIONS_HH

#include "ast/ast_visitor.hh"
#include "ast/detail/expressions.hh"
#include "ast/detail/nodes.hh"
#include "core/lexer.hh"

namespace cascade::ast {
  /** @brief Represents a `const` declaration */
  class const_decl : public declaration {
    std::string m_name;
    std::unique_ptr<expression> m_initializer;
    std::unique_ptr<type_base> m_type;

  public:
    /**
     * @brief Creates a const declaration
     * @param info Source info for the declaration
     * @param name The name given to the declaration
     * @param init The initializer
     * @param type The type of the declaration
     */
    explicit const_decl(core::source_info info, std::string name, std::unique_ptr<expression> init,
        std::unique_ptr<type_base> type)
        : declaration(kind::declaration_const, std::move(info)),
          m_name(std::move(name)),
          m_initializer(std::move(init)),
          m_type(std::move(type)) {}

    /** @brief Gets the name of the declaration */
    [[nodiscard]] std::string_view name() const { return m_name; }

    /** @brief Gets the expression that initializes the declaration */
    [[nodiscard]] expression &initializer() const { return *m_initializer; }

    /** @brief Gets the type of the declaration */
    [[nodiscard]] type_base &type() const { return *m_type; }

    /** @brief Accepts a visitor */
    virtual void accept(ast_visitor &visitor) { return visitor.visit(*this); }
  };

  /** @brief Represents a `static` declaration */
  class static_decl : public declaration {
    std::string m_name;
    std::unique_ptr<expression> m_initializer;
    std::unique_ptr<type_base> m_type;

  public:
    /**
     * @brief Creates a static declaration
     * @param info Source info for the declaration
     * @param name The name given to the declaration
     * @param init The initializer
     * @param type The type of the declaration
     */
    explicit static_decl(core::source_info info, std::string name, std::unique_ptr<expression> init,
        std::unique_ptr<type_base> type)
        : declaration(kind::declaration_static, std::move(info)),
          m_name(std::move(name)),
          m_initializer(std::move(init)),
          m_type(std::move(type)) {}

    /** @brief Gets the name of the declaration */
    [[nodiscard]] std::string_view name() const { return m_name; }

    /** @brief Gets the expression that initializes the declaration */
    [[nodiscard]] expression &initializer() const { return *m_initializer; }

    /** @brief Gets the type of the declaration */
    [[nodiscard]] type_base &type() const { return *m_type; }

    virtual void accept(ast_visitor &visitor) final { return visitor.visit(*this); }
  };

  /** @brief Represents a single argument declaration for a function */
  class argument : public declaration {
    std::string m_name;
    std::unique_ptr<type_base> m_type;

  public:
    explicit argument(core::source_info info, std::string name, std::unique_ptr<type_base> type)
        : declaration(kind::declaration_argument, std::move(info)),
          m_name(std::move(name)),
          m_type(std::move(type)) {}

    /** @brief Returns the name of the argument */
    [[nodiscard]] std::string_view name() const { return m_name; }

    /** @brief Returns a pointer to the argument's type signature */
    [[nodiscard]] type_base &type() const { return *m_type; }

    virtual void accept(ast_visitor &visitor) final { return visitor.visit(*this); }
  };

  /** @brief Represents a function */
  class fn : public declaration {
    std::string m_name;
    std::vector<argument> m_args;
    std::unique_ptr<type_base> m_return_type;
    std::unique_ptr<expression> m_block;

  public:
    /**
     * @brief Creates a function
     * @param info The source info for the whole function
     * @param name The name of the function
     * @param args List of arguments and their type signatures
     * @param block The body of the function
     */
    explicit fn(core::source_info info, std::string name, std::vector<argument> args,
        std::unique_ptr<type_base> type, std::unique_ptr<expression> block)
        : declaration(kind::declaration_fn, std::move(info)),
          m_name(std::move(name)),
          m_args(std::move(args)),
          m_return_type(std::move(type)),
          m_block(std::move(block)) {}

    /** @brief Returns the name of the argument */
    [[nodiscard]] std::string_view name() const { return m_name; }

    /** @brief Returns a reference to the arguments */
    [[nodiscard]] std::vector<argument> &args() { return m_args; }

    /** @brief Returns a pointer to the argument's type signature */
    [[nodiscard]] type_base &type() const { return *m_return_type; }

    /** @brief Returns a pointer to the body of the fn */
    [[nodiscard]] expression &body() const { return *m_block; }

    virtual void accept(ast_visitor &visitor) final { return visitor.visit(*this); }
  };

  /** @brief Represents a module declaration for a file */
  class module_decl : public declaration {
    std::string m_name;

  public:
    /**
     * @brief Creates a module
     * @param info The source info
     * @param name The full module path
     */
    explicit module_decl(core::source_info info, std::string name)
        : declaration(kind::declaration_module, std::move(info)), m_name(std::move(name)) {}

    /** @brief Returns the module name */
    [[nodiscard]] std::string_view name() const { return m_name; }

    virtual void accept(ast_visitor &visitor) final { return visitor.visit(*this); }
  };

  /** @brief Represents a module declaration for a file */
  class import_decl : public declaration {
    std::string m_name;
    std::vector<std::string> m_items;
    std::optional<std::string> m_alias;

  public:
    /**
     * @brief Creates a module
     * @param info The source info
     * @param name The full module path
     * @param items The list of items to import (if using `from`)
     */
    explicit import_decl(core::source_info info, std::string name, std::vector<std::string> items,
        std::optional<std::string> alias)
        : declaration(kind::declaration_import, std::move(info)),
          m_name(std::move(name)),
          m_items(std::move(items)),
          m_alias(std::move(alias)) {}

    /** @brief Returns the module name being imported */
    [[nodiscard]] std::string_view name() const { return m_name; }

    /** @brief Returns the items being imported */
    [[nodiscard]] const std::vector<std::string> &items() const { return m_items; }

    /** @brief Returns the alias of the imported module, if there is one */
    [[nodiscard]] std::optional<std::string_view> alias() const { return m_alias; }

    virtual void accept(ast_visitor &visitor) final { return visitor.visit(*this); }
  };

  /** @brief Represents an exported entity */
  class export_decl : public declaration {
    std::unique_ptr<declaration> m_exported;

  public:
    /**
     * @brief Create an exported entity
     * @param info The source information
     * @param exported The entity being exported
     */
    explicit export_decl(core::source_info info, std::unique_ptr<declaration> exported)
        : declaration(kind::declaration_export, std::move(info)), m_exported(std::move(exported)) {}

    /** @brief Returns a pointer to the item being exported */
    [[nodiscard]] declaration &exported() const { return *m_exported; }

    virtual void accept(ast_visitor &visitor) final { return visitor.visit(*this); }
  };

  class type_decl : public declaration {
    std::unique_ptr<type_base> m_type;
    std::string m_name;

  public:
    /**
     * @brief Create a type declaration
     * @param info The source information
     * @param type The type being aliased
     * @param name The name of the alias
     */
    explicit type_decl(core::source_info info, std::unique_ptr<type_base> type, std::string name)
        : declaration(kind::declaration_export, std::move(info)),
          m_type(std::move(type)),
          m_name(std::move(name)) {}

    /** @brief Returns a pointer to the item being exported */
    [[nodiscard]] type_base &type() const { return *m_type; }

    /** @brief Returns a string_view to the alias given to the type */
    [[nodiscard]] std::string_view name() const { return m_name; }

    virtual void accept(ast_visitor &visitor) final { return visitor.visit(*this); }
  };
} // namespace cascade::ast

#endif
