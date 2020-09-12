#include "core/typechecker.hh"
#include "ast/ast.hh"
#include <cassert>
#include <unordered_map>

using namespace cascade;
class scope;
using progs = std::vector<ast::program>;
using symbol_table = std::unordered_map<std::string_view, std::reference_wrapper<ast::type_base>>;
using optional_parent = std::optional<std::reference_wrapper<scope>>;

class scope {
  symbol_table m_table;

  optional_parent m_parent;

public:
  explicit scope(optional_parent parent) : m_parent(std::move(parent)) {}

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
      return m_table[name];
    }

    return m_parent.value().get().get(name);
  }
};

class typechecker {
  const progs &m_programs;
  core::report_fn m_report;
  scope m_global_scope;

public:
  /** @brief Creates a typechecker */
  explicit typechecker(const progs &progs, core::report_fn report);

  /** @brief Typechecks the program, sets up the main symbol table(s) */
  bool typecheck();
};

typechecker::typechecker(const progs &progs, core::report_fn report)
    : m_programs(progs)
    , m_report(std::move(report))
    , m_global_scope(std::nullopt) {}

bool typechecker::typecheck() {
  (void)m_programs;
  (void)m_report;

  return true;
}

bool core::typecheck(const progs &programs, core::report_fn report) {
  typechecker checker(programs, std::move(report));

  return checker.typecheck();
}
