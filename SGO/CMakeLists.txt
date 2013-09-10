link_libraries(cgraph)
include_directories(../SLF)
add_llvm_loadable_module(SGO 
	SGO.h SGO.cpp
	DotParser.h DotParser.cpp
	SITDotParser.h SITDotParser.cpp
	SGOUtil.h SGOUtil.cpp
)

target_link_libraries(SGO SLF)
