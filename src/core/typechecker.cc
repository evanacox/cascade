#include "core/typechecker.hh"
#include "ast/ast.hh"

using namespace cascade;
using progs = std::vector<ast::program>;

class typechecker {
  const progs &m_programs;
  core::report_fn m_report;

public:
  /** @brief Creates a typechecker */
  explicit typechecker(const progs &progs, core::report_fn report);

  /** @brief Typechecks the program, sets up the main symbol table(s) */
  bool typecheck();
};

typechecker::typechecker(const progs &progs, core::report_fn report)
    : m_programs(progs), m_report(std::move(report)) {}

bool typechecker::typecheck() {
  (void)m_programs;
  (void)m_report;

  return true;
}

bool core::typecheck(const progs &programs, core::report_fn report) {
  typechecker checker(programs, std::move(report));

  return checker.typecheck();
}
