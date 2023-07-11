#pragma once

#include <clang/AST/ASTContext.h>
#include <clang/AST/APValue.h>
#include <clang/AST/Expr.h>
#include <clang/Basic/LangOptions.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Lex/Lexer.h>

#include <llvm/Support/raw_ostream.h>
#include <llvm/ADT/APSInt.h>
#include <llvm/ADT/APFloat.h>

#include "../RosApiCall.h"

namespace rosdiscover {
  
std::string createName(const clang::DeclRefExpr* declRef) {
  std::string name = declRef->getNameInfo().getAsString();
  if (declRef->hasQualifier()) {
    name = "";
    llvm::raw_string_ostream os(name);
    static clang::LangOptions langOptions;
    static clang::PrintingPolicy printPolicy(langOptions);
    declRef->getQualifier()->print(os, printPolicy);
    name = os.str();
  }
  return name;
}

inline bool stmtContainsStmt(const clang::Stmt* parent, const clang::Stmt* child) {
  if (parent == nullptr || child == nullptr) {
    return false;
  }
  for (auto c: parent->children()) {
    if (c == child) {
      return true;
    } else if (stmtContainsStmt(c, child)) {
      return true;
    }
  }
  return false;
}

std::string prettyPrint(clang::SourceRange sourceRange, clang::ASTContext const &context) {
  auto range = clang::CharSourceRange::getTokenRange(sourceRange);
  llvm::StringRef ref = clang::Lexer::getSourceText(
    range, 
    context.getSourceManager(), 
    clang::LangOptions()
  );
  return ref.str();
}

std::string prettyPrint(const clang::Stmt* statement, clang::ASTContext const &context) {
  return prettyPrint(statement->getSourceRange(), context);
}

std::string prettyPrint(const clang::Expr* expr, clang::ASTContext const &context) {
  return prettyPrint(expr->getSourceRange(), context);
}

std::vector<const clang::Stmt*> getTransitiveChildenByType(const clang::Stmt* parent, 
  bool const includeDeclRefExpr,
  bool const includeCallExpr
) {
  std::vector<const clang::Stmt*> result;
  for (auto c: parent->children()) {
    if (includeDeclRefExpr) {
      if (auto *expr = clang::dyn_cast<clang::DeclRefExpr>(c)) {
        result.push_back(expr);
      } 
    }
    if (includeCallExpr) {
      if (auto *expr = clang::dyn_cast<clang::CallExpr>(c)) {
        result.push_back(expr);
      } 
    }
    auto childResult = getTransitiveChildenByType(c,  includeDeclRefExpr, includeCallExpr);
    result.insert(result.end(), childResult.begin(), childResult.end());
  }
  return result;
}
  
namespace api_call {

double apFloatToDouble(llvm::APFloat apfloat) {
  bool loseInfo = true;
  apfloat.convert(llvm::APFloatBase::IEEEdouble(), llvm::APFloatBase::rmNearestTiesToAway, &loseInfo) ;
  return apfloat.convertToDouble();
}

clang::APValue const * evaluateNumber(
  const std::string debugTag, 
  const clang::Expr *expr,
  const clang::ASTContext &Ctx,
  bool debugPrint=true
  ) {
  if (expr == nullptr) {
    return nullptr;
  }

  if (expr->isValueDependent()) {
    if (debugPrint) {
      llvm::outs() << "DEBUG [" << debugTag << "]: Is value-dependent and cannot be evaluated: "; 
      expr->dump();
      llvm::outs() << "\n";
    }

    return nullptr;
  }


  //Try evaluating the frequency as integer.
  clang::Expr::EvalResult resultInt;

  if (expr->EvaluateAsInt(resultInt, Ctx)) {
    llvm::outs() << "DEBUG [" << debugTag << "]: evaluated INT: (" << resultInt.Val.getInt().getSExtValue() << ")\n";
    return new clang::APValue(resultInt.Val);
  }

  //Try evaluating the frequency as float.
  llvm::APFloat resultFloat(0.0);
  if (expr->EvaluateAsFloat(resultFloat, Ctx)) {
    llvm::outs() << "DEBUG [" << debugTag << "]: evaluated Float: (" << apFloatToDouble(resultFloat) << ")\n";
    return new clang::APValue(resultFloat);
  }

  //Try evaluating the frequency as fixed point.
  clang::Expr::EvalResult resultFixed;
  if (expr->EvaluateAsFixedPoint(resultFixed, Ctx)) {
    llvm::outs() << "DEBUG [" << debugTag << "]: evaluated Fixed: (" << resultFixed.Val.getFixedPoint().toString() << ")\n";
    return new clang::APValue(resultFixed.Val.getFixedPoint());
  } 

  //All evaluation attempts have failed.
  if (debugPrint) {
    llvm::outs() << "DEBUG [" << debugTag << "]: Cannot be evaluated: "; 
    expr->dump();
    llvm::outs() << "\n";
  }

  return nullptr;
}

clang::Expr const * constrInitMemberExpr(const std::string debugTag, 
  const clang::MemberExpr *declRef) {
    const auto *valueDecl = declRef->getMemberDecl();
    if (valueDecl != nullptr) {
      const auto *fieldDecl = clang::dyn_cast<clang::FieldDecl>(valueDecl);
      if (fieldDecl != nullptr) {
        llvm::outs() << "Debug [" << debugTag << "] evaluateNumberDeclRef: ";
        fieldDecl->dump();
        llvm::outs() << "\n";
        if(fieldDecl->hasInClassInitializer()) {
          llvm::outs() << "Debug [" << debugTag << "] hasInClassInitializer: ";
          fieldDecl->getInClassInitializer()->dump();
          llvm::outs() << "\n";
          return fieldDecl->getInClassInitializer();
        }

        const auto *parent = fieldDecl->getParent();
        llvm::outs() << "Debug [" << debugTag << "] getParent\n";
        const auto *classDecl = clang::dyn_cast<clang::CXXRecordDecl>(parent);
        if (classDecl != nullptr) { 
          for (const auto *ctorDecl: classDecl->ctors()) {
            if (ctorDecl->getDefinition() == nullptr) {
              llvm::outs() << "Warning [" << debugTag << "] Constructor has no definition: ";
              ctorDecl->print(llvm::outs());
              llvm::outs() << "\n";
              continue;
            }
            const auto *constructorDef = clang::dyn_cast<clang::CXXConstructorDecl>(ctorDecl->getDefinition());
            if (constructorDef == nullptr) {
              llvm::outs() << "Warning [" << debugTag << "] Constructor definition is not CXXConstructorDecl: ";
              ctorDecl->print(llvm::outs());
              llvm::outs() << "\n";
              continue;
            }

            llvm::outs() << "Debug [" << debugTag << "] found constructor: ";
            constructorDef->print(llvm::outs());
            llvm::outs() << "\n";
            for (const auto *init : constructorDef->inits()) {
              if (init == nullptr || init->getMember() == nullptr) {
                llvm::outs() << "Warning [" << debugTag << "] init incomplete";
                continue;
              }
              llvm::outs() << "Debug [" << debugTag << "] found init: ";
              init->getMember()->dump();
              llvm::outs() << "\n";
              if (init->getInit() == nullptr) {
                llvm::outs() << "Init empty\n";
                continue; //empty init
              }
              init->getInit()->dump();
              llvm::outs() << "\n";
                
              if (init->getMember()->getID() == fieldDecl->getID()) {
                return init->getInit(); //TODO: Return all of them
              }
            }
          }
        }
      } 
    }

    return nullptr;
  }

clang::APValue const * evaluateNumberMemberExpr(const std::string debugTag, 
  const clang::MemberExpr *declRef,
  const clang::ASTContext &Ctx,
  bool debugPrint=true) {
    return evaluateNumber(debugTag, constrInitMemberExpr(debugTag, declRef), Ctx);
  }


const clang::ValueDecl *getCallerDecl(const std::string debugTag, const clang::CXXMemberCallExpr * memberCallExpr) {
  const auto *caller = memberCallExpr->getImplicitObjectArgument()->IgnoreImpCasts();

  const auto *member = clang::dyn_cast<clang::MemberExpr>(caller);
  if (member == nullptr)
  {
    const auto *declRef = clang::dyn_cast<clang::DeclRefExpr>(caller);
    if (declRef == nullptr || !declRef->getDecl()) {
      llvm::outs() << "ERROR [" << debugTag << "] Can't find declaration of CXXMemberCallExpr: ";
      memberCallExpr->dump();
      llvm::outs() << "\n";
      return nullptr;
    }
    return declRef->getDecl();
  }
  return member->getMemberDecl();
}
} // rosdiscover::api_call
} // rosdiscover
