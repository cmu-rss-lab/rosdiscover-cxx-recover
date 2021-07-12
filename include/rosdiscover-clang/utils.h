#pragma once

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

} // rosdiscover
