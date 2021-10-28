#pragma once

#include <string>

namespace rosdiscover {

std::string typeNameToFormatName(std::string typeName) {
  // remove the trailing underscore, if there is any
  if (typeName.back() == '_') {
    typeName = typeName.substr(0, typeName.size() - 1);
  }

  auto separatorPos = typeName.find("::");
  if (separatorPos == std::string::npos) {
    llvm::outs() << "WARNING: unable to find :: separator in C++ type name for ROS format\n";
    return typeName;
  }

  return typeName.replace(separatorPos, 2, "/");
}

} // rosdiscover