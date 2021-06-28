add_executable(rosdiscover
  RosDiscover.cpp
)

# TODO compile these into a static executable with a static libc++
llvm_map_components_to_libnames(ROSDISCOVER_LLVM_LIBS
  Demangle
)

target_link_libraries(rosdiscover
  ${ROSDISCOVER_LLVM_LIBS}
  ${CLANG_LIBS}
)
target_compile_definitions(rosdiscover
  PUBLIC ${CLANG_DEFINITIONS} ${LLVM_DEFINITIONS}
)
target_include_directories(rosdiscover
  PUBLIC ${CLANG_INCLUDE_DIRS} ${LLVM_INCLUDE_DIRS} ../../include
)

install(
  TARGETS rosdiscover
  DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

# we need to install clang's headers!
