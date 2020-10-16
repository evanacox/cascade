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
 * core/typechecker.cc
 *   Implements the type logic for the language, including various casting and
 *   promotion / conversion rules
 *
 *---------------------------------------------------------------------------*/

#include "core/typechecker.hh"
#include "ast/ast.hh"
#include "ast/detail/declarations.hh"
#include "ast/detail/types.hh"
#include "core/lexer.hh"
#include "errors/error.hh"
#include "fmt/format.h"
#include "util/types.hh"
#include <cassert>
#include <deque>
#include <iostream>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>

using namespace cascade;
class scope;

using kind = ast::kind;
using mods = ast::type_data::type_modifiers;
using base = ast::type_data::type_base;
using ec = errors::error_code;

class error_type : public ast::type_data {
public:
  error_type() : type_data({}, ast::type_data::type_base::error_type, 0) {}
};

static std::string expected_type(const ast::type_data &expected, const ast::type_data &got) {
  return fmt::format("Expected type '{}', got type '{}'.",
      util::to_string(expected),
      util::to_string(got));
}

class scope {
  /** @brief Variables mapped to their types */
  std::unordered_map<std::string_view, ast::type_data> m_table;

  /** @brief Type aliases mapped to actual types */
  std::unordered_map<std::string_view, ast::type_data> m_types;

  std::optional<std::reference_wrapper<scope>> m_parent;

public:
  explicit scope(std::optional<std::reference_wrapper<scope>> parent = std::nullopt)
      : m_parent(std::move(parent)) {}

  /**
   * @brief Whether or not the scope includes that symbol
   * @param name The name to check for
   */
  bool has(std::string_view name) {
    if (m_table.find(name) != m_table.end()) {
      return true;
    }

    if (m_parent) {
      return m_parent.value().get().has(name);
    }

    return false;
  }

  /**
   * @brief Gets the type associated with a name
   * @param name The name to get
   */
  ast::type_data &get(std::string_view name) {
    assert(has(name) && "attempting to get non-existent variable!");

    if (m_table.find(name) != m_table.end()) {
      return m_table.at(name);
    }

    return m_parent.value().get().get(name);
  }

  void set(std::string_view name, ast::type_data type) {
    m_table.insert_or_assign(name, std::move(type));
  }

  bool has_alias(std::string_view name) {
    if (m_types.find(name) != m_table.end()) {
      return true;
    }

    if (m_parent) {
      return m_parent.value().get().has_alias(name);
    }

    return false;
  }

  ast::type_data &get_alias(std::string_view name) {
    assert(has_alias(name) && "attempting to get non-existent variable!");

    if (m_types.find(name) != m_types.end()) {
      return m_types.at(name);
    }

    return m_parent.value().get().get_alias(name);
  }

  void set_alias(std::string_view name, ast::type_data type) {
    m_types.insert_or_assign(name, std::move(type));
  }

  std::unordered_map<std::string_view, ast::type_data> &table() { return m_table; };

  std::unordered_map<std::string_view, ast::type_data> &types() { return m_types; };
};

class typechecker : public ast::visitor<ast::type_data> {
  /** @brief List of the ASTs */
  std::vector<ast::program> &m_programs;

  /**
   * @brief List with size equal to m_program. m_program[0]'s scope is m_global_scope[0],
   * m_program[1]'s scope is m_global_scope[1], and so on
   */
  std::vector<scope> m_global_scopes;

  /** @brief Report function for errors */
  core::report_fn m_report;

  bool m_has_failed = false;

  std::string_view m_current_source;

  /** @brief VIEW into the current scope, MAY POINT TO STACK OBJECT. GETS REASSIGNED */
  std::reference_wrapper<scope> m_current_scope;

  /** @brief Name of the currently initializing variable, if any */
  std::string_view m_currently_initializing;

  /**
   * @brief Handles adding a declaration to the global scope initially
   * needs to be its own function so it can be recursive for exports
   */
  void handle_single_declaration(const ast::declaration &ref);

  /**
   * @brief Goes through all "global" symbols for a module and sets their type
   * @param prog The program to get symbols from
   */
  void register_global_symbols(ast::program &prog);

  /**
   * @brief Typechecks a single program
   * @param prog The program to check
   */
  bool typecheck_program(ast::program &prog);

  /**
   * @brief Reports an error and sets the flags to go with it
   * @param node The node to report on
   * @param code The error code
   * @param message The message to go with the error
   */
  void report(const ast::node &node, ec code, std::string message = "");

  /**
   * @brief Checks if @p from can be promoted to @p to
   * @param from The starting type (e.g 'i8')
   * @param to The ending type (e.g 'i64')
   * @return Whether the implicit conversion is valid
   */
  bool can_promote(const ast::type_data &from, const ast::type_data &to);

  /**
   * @brief Attempts to get the result of an arithmetic binary expression from two types
   * @param lhs Type of the LHS value
   * @param rhs Type of the RHS value
   * @return A type, if possible
   */
  ast::type_data binary_convert(const ast::binary &node);

  ast::type_data promote(const ast::type_data &to_upcast, const ast::type_data &upcast_to);

public:
  /** @brief Creates a typechecker */
  explicit typechecker(std::vector<ast::program> &progs, core::report_fn report);

  /** @brief Typechecks the programs, sets up the main symbol table(s) */
  bool typecheck(const std::vector<std::string_view> &sources);

#define VISIT(type) virtual ast::type_data visit(ast::type &) final

  CASCADE_VISIT_TYPES

#undef VISIT
};

typechecker::typechecker(std::vector<ast::program> &progs, core::report_fn report)
    : m_programs(progs)
    , m_global_scopes(m_programs.size())
    , m_report(std::move(report))
    , m_current_scope(m_global_scopes.front()) {}

void typechecker::report(const ast::node &node, ec code, std::string message) {
  auto err = std::make_unique<errors::type_error>(code,
      node,
      m_current_source,
      message == "" ? std::nullopt : std::make_optional(message));

  m_report(std::move(err));

  m_has_failed = true;
}

ast::type_data typechecker::binary_convert(const ast::binary &node) {
  auto lhs_type = node.lhs().accept(*this);
  auto rhs_type = node.rhs().accept(*this);

  if (can_promote(lhs_type, rhs_type)) {
    return rhs_type;
  }

  if (can_promote(rhs_type, lhs_type)) {
    return lhs_type;
  }

  report(node,
      ec::mismatched_types,
      fmt::format("Unable to operate on '{}' and '{}'. Can't promote one to the other implicitly!",
          util::to_string(lhs_type),
          util::to_string(rhs_type)));

  return error_type();
}

bool typechecker::can_promote(const ast::type_data &from, const ast::type_data &to) {
  // no implicit conversions between f to u or i, or i to u
  if (from.is_builtin()) {
    // only same base types can be promoted, and only widening
    // promotions are allowed. e.g `f32 -> f64` or `u8 -> u8` is allowed, but
    // `i64` -> `i32` is not.
    if (from.base() == to.base()) {
      return from.precision() <= to.precision();
    }
  }

  return false;
}

ast::type_data typechecker::visit(ast::type &ref) { return ref.data(); }

ast::type_data typechecker::visit(ast::const_decl &ref) {
  m_currently_initializing = ref.name();
  auto initializer_type = ref.initializer().accept(*this);

  // e.g `const x = 5;`
  if (ref.type().data().is(ast::type::type_base::implied)) {
    // update AST value and the typechecker's representation
    ref.type().data() = std::move(initializer_type);
    m_global_scopes.back().set(ref.name(), ref.type().data());
  }

  // e.g `const x: i32 = 3.5;`
  else if (initializer_type != ref.type().data()) {
    report(ref, ec::mismatched_types, expected_type(ref.type().data(), initializer_type));
  }

  m_currently_initializing = "";
  return ref.type().data();
}

ast::type_data typechecker::visit(ast::static_decl &ref) {
  m_currently_initializing = ref.name();
  auto initializer_type = ref.initializer().accept(*this);

  // e.g `static x = 5;`
  if (ref.type().data().is(ast::type::type_base::implied)) {
    ref.type().data() = std::move(initializer_type);
    m_global_scopes.back().set(ref.name(), ref.type().data());
  }

  // e.g `static x: i32 = 3.5;`
  else if (initializer_type != ref.type().data()) {
    report(ref, ec::mismatched_types, expected_type(ref.type().data(), initializer_type));
  }

  m_currently_initializing = "";
  return ref.type().data();
}

ast::type_data typechecker::visit(ast::argument &ref) {
  (void)ref;
  assert(false && "Not implemented");
}

ast::type_data typechecker::visit(ast::fn &ref) {
  (void)ref;
  assert(false && "Not implemented");
}

ast::type_data typechecker::visit(ast::module_decl &ref) {
  (void)ref;
  assert(false && "Not implemented");
}

ast::type_data typechecker::visit(ast::import_decl &ref) {
  (void)ref;
  assert(false && "Not implemented");
}

ast::type_data typechecker::visit(ast::export_decl &ref) { return ref.exported().accept(*this); }

ast::type_data typechecker::visit(ast::char_literal &ref) {
  (void)ref;
  return ast::type_data({}, base::integer, 8);
}

ast::type_data typechecker::visit(ast::string_literal &ref) {
  (void)ref;
  assert(false && "Not implemented");
}

ast::type_data typechecker::visit(ast::int_literal &ref) {
  // todo: check for suffixes?
  (void)ref;
  return ast::type_data({}, base::integer, 32);
}

ast::type_data typechecker::visit(ast::float_literal &ref) {
  // todo: check for suffixes
  (void)ref;
  return ast::type_data({}, base::floating_point, 64);
}

ast::type_data typechecker::visit(ast::bool_literal &ref) {
  (void)ref;
  return ast::type_data({}, base::boolean, 1);
}

ast::type_data typechecker::visit(ast::identifier &ref) {
  if (ref.name() == m_currently_initializing) {
    report(ref, ec::using_variable_in_initializer);

    return error_type();
  }

  return m_current_scope.get().get(ref.name());
}

ast::type_data typechecker::visit(ast::call &ref) {
  (void)ref;
  assert(false && "Not implemented");
}

ast::type_data typechecker::visit(ast::binary &ref) {
  using opkind = core::token::kind;

  switch (ref.op()) {
    case opkind::symbol_plus:
    case opkind::symbol_hyphen:
    case opkind::symbol_star:
    case opkind::symbol_forwardslash:
    case opkind::symbol_percent:
    case opkind::symbol_pound:
    case opkind::symbol_caret:
    case opkind::symbol_pipe:
    case opkind::symbol_gtgt:
    case opkind::symbol_ltlt:
    case opkind::symbol_gt:
    case opkind::symbol_geq:
    case opkind::symbol_lt:
    case opkind::symbol_leq:
    case opkind::symbol_equalequal:
    case opkind::keyword_and:
    case opkind::keyword_or:
    case opkind::keyword_xor:
      return binary_convert(ref);
    default:
      break;
  }

  assert(false && "How did we get here?");
}

ast::type_data typechecker::visit(ast::unary &ref) {
  using opkind = core::token::kind;

  switch (ref.op()) {
    case opkind::symbol_at: {
      auto type = ref.rhs().accept(*this);
      type.modifiers().push_front(mods::mut_ptr);
      return type;
    }
    case opkind::symbol_star: {
      auto type = ref.rhs().accept(*this);

      if (type.modifiers().front() != mods::mut_ptr || type.modifiers().front() != mods::ptr) {
        report(ref,
            ec::dereference_requires_pointer_type,
            fmt::format("Expected a pointer type, got type '{}'", util::to_string(type)));
      }

      return type;
    }
    case opkind::symbol_pound: {
      auto type = ref.rhs().accept(*this);
      type.modifiers().push_front(mods::mut_ref);
      return type;
    }
    case opkind::symbol_hyphen:
      return ref.rhs().accept(*this);
    default:
      break;
  }

  assert(false && "How did we get here?");
}

ast::type_data typechecker::visit(ast::field_access &ref) {
  (void)ref;
  assert(false && "Not implemented");
}

ast::type_data typechecker::visit(ast::index &ref) {
  (void)ref;
  assert(false && "Not implemented");
}

ast::type_data typechecker::visit(ast::if_else &ref) {
  (void)ref;
  assert(false && "Not implemented");
}

ast::type_data typechecker::visit(ast::struct_init &ref) {
  (void)ref;
  assert(false && "Not implemented");
}

ast::type_data typechecker::visit(ast::block &ref) {
  (void)ref;
  assert(false && "Not implemented");
}

ast::type_data typechecker::visit(ast::expression_statement &ref) {
  (void)ref;
  assert(false && "Not implemented");
}

ast::type_data typechecker::visit(ast::let &ref) {
  m_currently_initializing = ref.name();
  (void)ref;
  assert(false && "Not implemented");
}

ast::type_data typechecker::visit(ast::mut &ref) {
  m_currently_initializing = ref.name();
  (void)ref;
  assert(false && "Not implemented");
}

ast::type_data typechecker::visit(ast::ret &ref) {
  (void)ref;
  assert(false && "Not implemented");
}

ast::type_data typechecker::visit(ast::loop &ref) {
  (void)ref;
  assert(false && "Not implemented");
}

ast::type_data typechecker::visit(ast::type_decl &ref) { return ref.type().data(); }

void typechecker::handle_single_declaration(const ast::declaration &decl) {
  switch (decl.raw_kind()) {
    case kind::declaration_const: {
      const auto &ref = static_cast<const ast::const_decl &>(decl);
      m_global_scopes.back().set(ref.name(), ref.type().data());
      break;
    }
    case kind::declaration_static: {
      const auto &ref = static_cast<const ast::static_decl &>(decl);
      m_global_scopes.back().set(ref.name(), ref.type().data());
      break;
    }
    case kind::declaration_export: {
      const auto &ref = static_cast<const ast::export_decl &>(decl);
      handle_single_declaration(ref.exported());
      break;
    }
    case kind::declaration_fn: {
      const auto &ref = static_cast<const ast::fn &>(decl);
      m_global_scopes.back().set(ref.name(), ref.type().data());
      break;
    }
    case kind::declaration_type: {
      const auto &ref = static_cast<const ast::type_decl &>(decl);
      m_global_scopes.back().set_alias(ref.name(), ref.type().data());
      break;
    }
    default:
      assert(false && "How did we get here?");
  }
}

void typechecker::register_global_symbols(ast::program &prog) {
  for (const auto &decl : prog.decls()) {
    handle_single_declaration(static_cast<const ast::const_decl &>(*decl));
  }
}

bool typechecker::typecheck_program(ast::program &prog) {
  // initializes a new "global scope" for each module
  m_global_scopes.emplace_back();

  register_global_symbols(prog);

  m_current_scope = m_global_scopes.back();

  for (auto &decl : prog.decls()) {
    decl->accept(*this);
  }

  std::cout << "== symbol types ==\n";
  for (auto &[k, v] : m_global_scopes.back().table()) {
    fmt::print("{{ name: {}, value: {} }}\n", k, util::to_string(v));
  }

  std::cout << "== type aliases ==\n";
  for (auto &[k, v] : m_global_scopes.back().types()) {
    fmt::print("{{ name: {}, value: {} }}\n", k, util::to_string(v));
  }

  return m_has_failed;
}

bool typechecker::typecheck(const std::vector<std::string_view> &sources) {
  auto src_it = sources.begin();

  for (auto &prog : m_programs) {
    m_current_source = *src_it++;
    typecheck_program(prog);
  }

  return m_has_failed;
}

bool core::typecheck(std::vector<ast::program> &programs,
    const std::vector<std::string_view> &sources,
    core::report_fn report) {
  typechecker checker(programs, std::move(report));

  return checker.typecheck(sources);
}
