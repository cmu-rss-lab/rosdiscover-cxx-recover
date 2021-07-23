#pragma once

#include <clang/AST/ASTContext.h>

#include "Stmt.h"
#include "../RawStatement.h"
#include "../Value/Value.h"

namespace rosdiscover {
namespace symbolic {

class AnnotatedSymbolicStmt : public SymbolicStmt {
public:
  ~AnnotatedSymbolicStmt(){};

  void print(llvm::raw_ostream &os) const override {
    os << "(@ ";
    symbolicStmt->print(os);
    os << " " << location << ")";
  }

  SymbolicStmt* getSymbolicStmt() {
    return symbolicStmt.get();
  }

  clang::Stmt* getClangStmt() {
    return clangStmt;
  }

  nlohmann::json toJson() const override {
    auto j = symbolicStmt->toJson();
    j["source-location"] = location;
    return j;
  }

  static std::unique_ptr<AnnotatedSymbolicStmt> create(
      clang::ASTContext &context,
      std::unique_ptr<SymbolicStmt> symbolicStmt,
      RawStatement *rawStmt
  ) {
    auto *clangStmt = rawStmt->getUnderlyingStmt();
    return std::make_unique<AnnotatedSymbolicStmt>(
        std::move(symbolicStmt),
        rawStmt->getUnderlyingStmt(),
        clangStmt->getSourceRange().printToString(context.getSourceManager())
    );
  }

  AnnotatedSymbolicStmt(
    std::unique_ptr<SymbolicStmt> symbolicStmt,
    clang::Stmt* clangStmt,
    std::string const &location
  ) : symbolicStmt(std::move(symbolicStmt)), clangStmt(clangStmt), location(location)
  {}

private:
  // TODO this probably ought to be const?
  std::unique_ptr<SymbolicStmt> symbolicStmt;
  clang::Stmt *clangStmt;
  std::string const location;
};

} // rosdiscover::symbolic
} // rosdiscover