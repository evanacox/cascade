#include "core/typechecker.hh"
#include "ast/ast.hh"
#include "core/lexer.hh"
#include <cassert>
#include <string>
#include <unordered_map>

using namespace cascade;
class scope;

template <class T>
using symbol_table = std::unordered_map<T, std::reference_wrapper<ast::type_base>>;
using optional_parent = std::optional<std::reference_wrapper<scope>>;
using kind = ast::kind;
using type_id = std::string;

class scope {
  symbol_table<std::string_view> m_table;

  optional_parent m_parent;

public:
  explicit scope(optional_parent parent = std::nullopt) : m_parent(std::move(parent)) {}

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
  ast::type_base &get(std::string_view name) {
    assert(has(name) && "attempting to get non-existent variable!");

    if (m_table.find(name) != m_table.end()) {
      return m_table.at(name).get();
    }

    return m_parent.value().get().get(name);
  }
};

class typechecker : ast::visitor<type_id> {
  /** @brief List of the ASTs */
  std::vector<ast::program> m_programs;

  /**
   * @brief List with size equal to m_program. m_program[0]'s scope is m_global_scope[0],
   * m_program[1]'s scope is m_global_scope[1], and so on
   */
  std::vector<scope> m_global_scopes;

  /** @brief Report function for errors */
  core::report_fn m_report;

public:
  /** @brief Creates a typechecker */
  explicit typechecker(std::vector<ast::program> progs, core::report_fn report);

  /** @brief Typechecks the program, sets up the main symbol table(s) */
  bool typecheck();

  /**
   * @brief Goes through all "global" symbols for a module and sets their type
   * @param prog The program to get symbols from
   */
  void register_global_symbols(ast::program &prog);

#define VISIT(type) virtual type_id visit(ast::type &) final

  CASCADE_VISIT_TYPES

#undef VISIT
};

#undef VISIT

typechecker::typechecker(std::vector<ast::program> progs, core::report_fn report)
    : m_programs(std::move(progs))
    , m_global_scopes(m_programs.size())
    , m_report(std::move(report)) {}

type_id typechecker::visit(ast::type_base &ref) {
  (void)ref;
  return "type_base";
}

type_id typechecker::visit(ast::const_decl &ref) {
  (void)ref;
  return "type_base";
}

type_id typechecker::visit(ast::static_decl &ref) {
  (void)ref;
  return "type_base";
}

type_id typechecker::visit(ast::argument &ref) {
  (void)ref;
  return "type_base";
}

type_id typechecker::visit(ast::fn &ref) {
  (void)ref;
  return "type_base";
}

type_id typechecker::visit(ast::module_decl &ref) {
  (void)ref;
  return "type_base";
}

type_id typechecker::visit(ast::import_decl &ref) {
  (void)ref;
  return "type_base";
}

type_id typechecker::visit(ast::export_decl &ref) {
  (void)ref;
  return "type_base";
}

type_id typechecker::visit(ast::char_literal &ref) {
  (void)ref;
  return "type_base";
}

type_id typechecker::visit(ast::string_literal &ref) {
  (void)ref;
  throw std::logic_error{"string literals not implemented! figure out how slices will work"};
}

type_id typechecker::visit(ast::int_literal &ref) {
  // todo: check for suffixes?
  (void)ref;
  return "i32";
}

type_id typechecker::visit(ast::float_literal &ref) {
  // todo: check for suffixes
  (void)ref;
  return "f32";
}

type_id typechecker::visit([[maybe_unused]] ast::bool_literal &ref) { return "bool"; }

type_id typechecker::visit(ast::identifier &ref) {
  (void)ref;
  return "type_base";
}

type_id typechecker::visit(ast::call &ref) {
  (void)ref;
  return "type_base";
}

type_id typechecker::visit(ast::binary &ref) {
  (void)ref;
  return "type_base";
}

type_id typechecker::visit(ast::unary &ref) {
  using opkind = core::token::kind;

  switch (ref.op()) {
    case opkind::symbol_at:
      return std::string("*mut") + ref.rhs().accept(*this);
    default:
      break;
  }
}

type_id typechecker::visit(ast::field_access &ref) {
  (void)ref;
  return "type_base";
}

type_id typechecker::visit(ast::index &ref) {
  (void)ref;
  return "type_base";
}

type_id typechecker::visit(ast::if_else &ref) {
  (void)ref;
  return "type_base";
}

type_id typechecker::visit(ast::struct_init &ref) {
  (void)ref;
  return "type_base";
}

type_id typechecker::visit(ast::block &ref) {
  (void)ref;
  return "type_base";
}

type_id typechecker::visit(ast::expression_statement &ref) {
  (void)ref;
  return "type_base";
}

type_id typechecker::visit(ast::let &ref) {
  (void)ref;
  return "type_base";
}

type_id typechecker::visit(ast::mut &ref) {
  (void)ref;
  return "type_base";
}

type_id typechecker::visit(ast::ret &ref) {
  (void)ref;
  return "type_base";
}

type_id typechecker::visit(ast::loop &ref) {
  (void)ref;
  return "type_base";
}

type_id typechecker::visit(ast::type_decl &ref) {
  (void)ref;
  return "type_base";
}

void typechecker::register_global_symbols(ast::program &prog) {
  for (auto &decl : prog.decls()) {
    (void)decl;
  }
}

bool typechecker::typecheck() {
  //

  return true;
}

bool core::typecheck(std::vector<ast::program> &programs, core::report_fn report) {
  typechecker checker(std::move(programs), std::move(report));

  return checker.typecheck();
}
