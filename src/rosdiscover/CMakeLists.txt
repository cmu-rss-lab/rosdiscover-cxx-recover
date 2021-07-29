add_executable(rosdiscover
  RosDiscover.cpp
)

llvm_map_components_to_libnames(ROSDISCOVER_LLVM_LIBS
  Demangle
)

set_target_properties(rosdiscover
  PROPERTIES
    CXX_STANDARD 14
    CMAKE_CXX_STANDARD_REQUIRED ON
    LINK_SEARCH_START_STATIC ON
    LINK_SEARCH_END_STATIC ON
)
target_link_libraries(rosdiscover PRIVATE
  ${ROSDISCOVER_LLVM_LIBS}
  ${CLANG_LIBS}
  nlohmann_json::nlohmann_json
  fmt::fmt-header-only
)
target_compile_options(rosdiscover PRIVATE
  -Wall -Werror -fno-rtti
)
target_link_options(rosdiscover PRIVATE
  -static-libgcc -static-libstdc++ -static
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
