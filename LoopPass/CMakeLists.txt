link_libraries(cgraph)
link_libraries(log4cpp)

add_llvm_loadable_module(LoopPass 
    LoopPass.h LoopPass.cpp
)
