add_library(rosdiscover-llvm SHARED
  RosApiCallFinderPass.cpp
)

llvm_map_components_to_libnames(ROSDISCOVER_LLVM_LINK_LIBS
  Demangle
)

target_compile_definitions(rosdiscover-llvm PUBLIC ${LLVM_DEFINITIONS})
target_include_directories(rosdiscover-llvm
  PUBLIC ${LLVM_INCLUDE_DIRS} ../../include
)
target_link_directories(rosdiscover-llvm PUBLIC ${LLVM_LIBRARY_DIRS})
target_link_libraries(rosdiscover-llvm PUBLIC ${ROSDISCOVER_LLVM_LINK_LIBS})

install(
  TARGETS rosdiscover-llvm
  DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)
