add_library(rosdiscover SHARED
  RosApiCallFinderPass.cpp
)

target_compile_definitions(rosdiscover PUBLIC ${LLVM_DEFINITIONS})
target_include_directories(rosdiscover PUBLIC ${LLVM_INCLUDE_DIRS})
target_link_directories(rosdiscover PUBLIC ${LLVM_LIBRARY_DIRS})

install(
  TARGETS rosdiscover
  DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)
