cmake_minimum_required(VERSION 2.8)

#set(CMAKE_C_COMPILER clang-3.4)
#set(CMAKE_CXX_COMPILER clang-3.4)
find_package(LLVM)

if(${LLVM_FOUND})
	MESSAGE(STATUS "FOUND")
endif (${LLVM_FOUND})

# Define add_llvm_* macro's.
include(AddLLVM)

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(./)
link_directories(${LLVM_LIBRARY_DIRS})

link_libraries(log4cpp)

llvm_replace_compiler_option(CMAKE_CXX_FLAGS "-frtti" "-fno-rtti")
add_subdirectory(SLF)
add_subdirectory(TempPass)
add_subdirectory(SGO)
add_subdirectory(LoopPass)
add_subdirectory(OperatorDependence)
