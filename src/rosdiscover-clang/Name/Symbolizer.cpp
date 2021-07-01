#include <rosdiscover-clang/Name/Symbolizer.h>

rosdiscover::name::NameExpr* rosdiscover::name::NameSymbolizer::symbolize(clang::Expr const *nameExpr) const {
  using namespace rosdiscover::name;
  return new Unknown();
}
