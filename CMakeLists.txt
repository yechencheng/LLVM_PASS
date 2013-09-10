cmake_minimum_required(VERSION 2.8)

find_package(LLVM)

if(${LLVM_FOUND})
	MESSAGE(STATUS "FOUND")
endif (${LLVM_FOUND})

# Define add_llvm_* macro's.
include(AddLLVM)

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

llvm_replace_compiler_option(CMAKE_CXX_FLAGS "-frtti" "-fno-rtti")
add_subdirectory(SLF)
add_subdirectory(TempPass)
