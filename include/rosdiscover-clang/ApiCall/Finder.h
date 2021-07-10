#pragma once

#include <vector>

#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Tooling/Tooling.h>

#include "Calls.h"
#include "RosApiCall.h"

namespace rosdiscover {
namespace api_call {


class RosApiCallFinder {
public:

  static std::vector<RosApiCall*> find(clang::tooling::ClangTool &tool) {
    return RosApiCallFinder(tool).run();
  }

private:
  RosApiCallFinder(clang::tooling::ClangTool &tool) :
    tool(tool),
    callFinders(),
    matchFinder(),
    calls()
  {
    build();
  }

  ~RosApiCallFinder() {
    for (auto finder : callFinders) {
      delete finder;
    }
  }

  void build() {
    addFinder(new AdvertiseServiceCall::Finder(calls));
    addFinder(new AdvertiseTopicCall::Finder(calls));
    addFinder(new BareDeleteParamCall::Finder(calls));
    addFinder(new BareGetParamCachedCall::Finder(calls));
    addFinder(new BareGetParamCall::Finder(calls));
    addFinder(new BareGetParamWithDefaultCall::Finder(calls));
    addFinder(new BareHasParamCall::Finder(calls));
    addFinder(new BareServiceCall::Finder(calls));
    addFinder(new BareSetParamCall::Finder(calls));
    addFinder(new DeleteParamCall::Finder(calls));
    addFinder(new GetParamCachedCall::Finder(calls));
    addFinder(new GetParamCall::Finder(calls));
    addFinder(new GetParamWithDefaultCall::Finder(calls));
    addFinder(new HasParamCall::Finder(calls));
    addFinder(new RosInitCall::Finder(calls));
    addFinder(new ServiceClientCall::Finder(calls));
    addFinder(new SetParamCall::Finder(calls));
    addFinder(new SubscribeTopicCall::Finder(calls));
  }

  void addFinder(RosApiCall::Finder *finder) {
    callFinders.push_back(finder);
    matchFinder.addMatcher(finder->getPattern(), finder);
  }

  /** Finds all API calls within a program described by a clang tool. */
  std::vector<RosApiCall*> run() {
    // TODO remove any existingcalls
    calls.clear();
    int result = tool.run(clang::tooling::newFrontendActionFactory(&matchFinder).get());
    if (result != 0) {
      llvm::errs() << "ERROR: ROS API call finder failed!\n";
      abort();
    }
    return calls;
  }

  std::vector<RosApiCall*> run(clang::ASTContext &context) {
    calls.clear();
    matchFinder.matchAST(context);
    return calls;
  }

private:
  clang::tooling::ClangTool &tool;
  std::vector<RosApiCall::Finder*> callFinders;
  clang::ast_matchers::MatchFinder matchFinder;
  // TODO use shared_ptr?
  std::vector<RosApiCall*> calls;
};


} // rosdiscover::api_call
} // rosdiscover
