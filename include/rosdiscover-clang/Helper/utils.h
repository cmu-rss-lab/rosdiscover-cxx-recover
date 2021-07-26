#pragma once

#include <unordered_map>
#include <unordered_set>

#include <clang/AST/ASTContext.h>
#include <clang/AST/ASTTypeTraits.h>
#include <clang/AST/Decl.h>
#include <clang/AST/Stmt.h>
#include <clang/Analysis/CallGraph.h>

namespace rosdiscover {

// https://stackoverflow.com/questions/20446201/how-to-check-if-string-ends-with-txt
// TODO: replace with c++20 std::string::ends_with
bool ends_with(const std::string &str, const std::string &suffix) {
  return str.size() >= suffix.size() &&
    str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

bool starts_with(const std::string &str, const std::string &prefix) {
  return str.rfind(prefix, 0) == 0;
}

clang::FunctionDecl const * getParentFunctionDecl(clang::ASTContext &context, clang::DynTypedNode const &node) {
  clang::FunctionDecl const *functionDecl = node.get<clang::FunctionDecl>();
  if (functionDecl != nullptr) {
    return functionDecl;
  }

  for (clang::DynTypedNode const parent : context.getParents(node)) {
    functionDecl = getParentFunctionDecl(context, parent);
    if (functionDecl != nullptr) {
      return functionDecl;
    }
  }

  return nullptr;
}

clang::FunctionDecl const * getParentFunctionDecl(clang::ASTContext &context, clang::Stmt const *stmt) {
  return getParentFunctionDecl(context, clang::DynTypedNode::create(*stmt));
}

/** Uses a given call graph to produce a mapping from functions to their callers */
std::unordered_map<clang::FunctionDecl const *, std::unordered_set<clang::FunctionDecl const *>> findCallers(
  clang::CallGraph &callGraph
) {
  std::unordered_map<clang::FunctionDecl const *, std::unordered_set<clang::FunctionDecl const *>> functionToCallers;

  for (auto const &callGraphEntry : callGraph) {
    if (callGraphEntry.first == nullptr || !clang::isa<clang::FunctionDecl>(callGraphEntry.first)) {
      continue;
    }

    clang::FunctionDecl const *caller = clang::dyn_cast<clang::FunctionDecl>(callGraphEntry.first)->getCanonicalDecl();
    clang::CallGraphNode const &callerNode = *callGraphEntry.second.get();
    for (clang::CallGraphNode::CallRecord const &callRecord : callerNode) {
      auto const *callee = clang::dyn_cast<clang::FunctionDecl>(callRecord.Callee->getDecl())->getCanonicalDecl();

      if (functionToCallers.find(callee) != functionToCallers.end()) {
        functionToCallers[callee].insert(caller);
      } else {
        functionToCallers.emplace(callee, std::unordered_set<clang::FunctionDecl const *>());
        functionToCallers[callee].insert(caller);
      }
    }
  }

  return functionToCallers;
}

} // rosdiscover
