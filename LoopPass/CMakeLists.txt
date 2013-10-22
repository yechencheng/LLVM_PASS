link_libraries(cgraph)

add_llvm_loadable_module(LoopPass 
    LoopPass.h LoopPass.cpp
)
