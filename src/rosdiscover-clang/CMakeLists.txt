add_executable(rosdiscover
  RosDiscover.cpp
)

# TODO compile these into a static executable with a static libc++
target_link_libraries(rosdiscover
  ${LLVM_LIBS}
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
