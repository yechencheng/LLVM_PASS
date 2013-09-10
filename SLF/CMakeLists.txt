link_libraries(cgraph)

add_llvm_loadable_module(SLF 
    SLF.h SLF.cpp
    Util.h Util.cpp
    IOValueParser.cpp IOValueParser.h
    TempTest.h TempTest.cpp
    DotParser.h DotParser.cpp)
