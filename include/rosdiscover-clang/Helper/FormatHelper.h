#pragma once

#include <string>

namespace rosdiscover {

std::string typeNameToFormatName(std::string typeName) {
  auto separatorPos = typeName.find("::");
  if (separatorPos == std::string::npos) {
    return typeName;
  }

  return typeName.replace(separatorPos, 2, "/");
}

} // rosdiscover