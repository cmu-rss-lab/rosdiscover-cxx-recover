add_executable(rosdiscover
  RosDiscover.cpp
)

# TODO compile these into a static executable with a static libc++
llvm_map_components_to_libnames(ROSDISCOVER_LLVM_LIBS
  Demangle
)

set_target_properties(rosdiscover
  PROPERTIES
    CXX_STANDARD 14
    CMAKE_CXX_STANDARD_REQUIRED ON
)
target_link_libraries(rosdiscover
  ${ROSDISCOVER_LLVM_LIBS}
  ${CLANG_LIBS}
  nlohmann_json::nlohmann_json
  fmt::fmt-header-only
)
target_compile_options(rosdiscover PRIVATE
  -Wall -Werror -fno-rtti
)
target_compile_definitions(rosdiscover
  PUBLIC ${CLANG_DEFINITIONS} ${LLVM_DEFINITIONS}
)
target_include_directories(rosdiscover
  PUBLIC
    ${CLANG_INCLUDE_DIRS}
    ${LLVM_INCLUDE_DIRS}
    nlohmann_json::nlohmann_json
    fmt::fmt
    ../../include
)

install(
  TARGETS rosdiscover
  DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

# TODO we need to install clang's headers!
