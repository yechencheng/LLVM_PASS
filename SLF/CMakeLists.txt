link_libraries(cgraph)

add_llvm_loadable_module(SLF 
    SLF.h SLF.cpp
    SLFUtil.h SLFUtil.cpp
    IOValueParser.cpp IOValueParser.h
    TempTest.h TempTest.cpp
)
