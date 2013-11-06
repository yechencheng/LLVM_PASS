link_libraries(cgraph)
link_libraries(log4cpp)
include_directories(../SLF)
include_directories(../LoopPass)
add_llvm_loadable_module(SGO 
	SGO.h SGO.cpp
	DotParser.h DotParser.cpp
	SITDotParser.h SITDotParser.cpp
	SGOUtil.h SGOUtil.cpp
)

#target_link_libraries(SGO LoopPass)
target_link_libraries(SGO SLF)
