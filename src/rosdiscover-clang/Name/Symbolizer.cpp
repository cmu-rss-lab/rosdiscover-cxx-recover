#include <rosdiscover-clang/Name/Symbolizer.h>

#include <clang/AST/ASTTypeTraits.h>

rosdiscover::name::NameExpr* rosdiscover::name::NameSymbolizer::symbolize(clang::Expr *nameExpr) {
  using namespace rosdiscover::name;

  // clang::DynTypedNode const &node = clang::DynTypedNode::create(*nameExpr);

  llvm::outs() << "symbolizing: ";
  nameExpr->dumpColor();
  llvm::outs() << "\n";

  if (clang::DeclRefExpr *declRefExpr = dyn_cast<clang::DeclRefExpr>(nameExpr)) {
    return symbolize(declRefExpr);
  } else if (clang::StringLiteral *literal = dyn_cast<clang::StringLiteral>(nameExpr)) {
    return symbolize(literal);
  } else if (clang::ImplicitCastExpr *implicitCastExpr = dyn_cast<clang::ImplicitCastExpr>(nameExpr)) {
    return symbolize(implicitCastExpr);
  } else if (clang::CXXConstructExpr *constructExpr = dyn_cast<clang::CXXConstructExpr>(nameExpr)) {
    return symbolize(constructExpr);
  }

  return new Unknown();
}

rosdiscover::name::NameExpr* rosdiscover::name::NameSymbolizer::symbolize(clang::CXXConstructExpr *constructExpr) {
   // does this call the std::string constructor?
  // FIXME this is a bit hacky and may break when other libc++ versions are used
  if (constructExpr->getConstructor()->getParent()->getQualifiedNameAsString() != "std::__cxx11::basic_string") {
    return symbolize(constructExpr->getArg(0));
  }

  return new Unknown();
}

rosdiscover::name::NameExpr* rosdiscover::name::NameSymbolizer::symbolize(clang::ImplicitCastExpr *castExpr) {
  // TODO check that we're dealing with strings or char[]
  return symbolize(castExpr->getSubExpr());
}

rosdiscover::name::NameExpr* rosdiscover::name::NameSymbolizer::symbolize(clang::DeclRefExpr *nameExpr) {
  // FIXME trying to access the decl causes the program to crash :-(
  auto *decl = nameExpr->getDecl();

  // is this a var decl?
  if (clang::VarDecl const *var_decl = dyn_cast<clang::VarDecl>(decl)) {
    llvm::outs() << "VAR DECL!\n";

    // TODO: check isLocalVarDecl?
    auto const *var_def = var_decl->getDefinition();
    if (var_def == nullptr) {
      llvm::outs() << "TRASH PANDA";
      return new Unknown();
    }

    llvm::outs() << "DEFINITION: ";
    var_def->dumpColor();
    llvm::outs() << "\n";

    return new Unknown();
  }

  auto *underlying_decl = decl->getUnderlyingDecl();
  llvm::outs() << "trying to dump the decl\n";
  underlying_decl->printQualifiedName(llvm::outs());
  underlying_decl->dump();
  llvm::outs() << "dumped the decl\n";

  // TODO implement
  return new Unknown();
}

rosdiscover::name::NameExpr* rosdiscover::name::NameSymbolizer::symbolize(clang::StringLiteral *literal) {
  return new rosdiscover::name::StringLiteral(literal->getString().str());
}

// TODO all of this will get replaced by symbolize methods
/** Attempts to return the string literal embedded in a given expression */
llvm::Optional<std::string> rosdiscover::name::NameSymbolizer::getLiteral(clang::Expr *expr) {
  using namespace clang;
  using namespace clang::ast_type_traits;

  // MaterializeTemporaryExpr 0x8c335e0 'const std::string':'const class std::__cxx11::basic_string<char>' lvalue
  // `-CXXBindTemporaryExpr 0x8c335c0 'const std::string':'const class std::__cxx11::basic_string<char>' (CXXTemporary 0x8c335c0)
  //   `-CXXConstructExpr 0x8c33580 'const std::string':'const class std::__cxx11::basic_string<char>' 'void (const char *, const class std::allocator<char> &)'
  //     |-ImplicitCastExpr 0x8c33548 'const char *' <ArrayToPointerDecay>
  //     | `-StringLiteral 0x8c32438 'const char [5]' lvalue "odom"
  //     `-CXXDefaultArgExpr 0x8c33560 'const class std::allocator<char>':'const class std::allocator<char>' lvalue

  static auto none = llvm::Optional<std::string>();

  auto const *materializeTempExpr = DynTypedNode::create(*expr).get<MaterializeTemporaryExpr>();
  if (materializeTempExpr == nullptr) {
    return none;
  }

  auto const *bindTempExpr = DynTypedNode::create(*(materializeTempExpr->getSubExpr())).get<CXXBindTemporaryExpr>();
  if (bindTempExpr == nullptr) {
    return none;
  }

  auto const *constructExpr = DynTypedNode::create(*(bindTempExpr->getSubExpr())).get<CXXConstructExpr>();
  if (constructExpr == nullptr) {
    return none;
  }

  // does this call the std::string constructor?
  // FIXME this is a bit hacky and may break when other libc++ versions are used
  if (constructExpr->getConstructor()->getParent()->getQualifiedNameAsString() != "std::__cxx11::basic_string") {
    return none;
  }

  auto const *castExpr = DynTypedNode::create(*(constructExpr->getArg(0))).get<ImplicitCastExpr>();
  if (castExpr == nullptr) {
    return none;
  }

  auto const *literal = DynTypedNode::create(*(castExpr->getSubExpr())).get<clang::StringLiteral>();
  if (literal == nullptr) {
    return none;
  }

  return llvm::Optional<std::string>(literal->getString().str());
}
