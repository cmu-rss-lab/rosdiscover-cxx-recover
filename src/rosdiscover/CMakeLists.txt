add_library(rosdiscover SHARED
  RosApiCallFinderPass.cpp
)

llvm_map_components_to_libnames(ROSDISCOVER_LLVM_LINK_LIBS
  Demangle
)

target_compile_definitions(rosdiscover PUBLIC ${LLVM_DEFINITIONS})
target_include_directories(rosdiscover
  PUBLIC ${LLVM_INCLUDE_DIRS} ../include
)
target_link_directories(rosdiscover PUBLIC ${LLVM_LIBRARY_DIRS})
target_link_libraries(rosdiscover PUBLIC ${ROSDISCOVER_LLVM_LINK_LIBS})

install(
  TARGETS rosdiscover
  DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)
