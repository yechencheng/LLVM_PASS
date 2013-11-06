link_libraries(cgraph)
link_libraries(log4cpp)

add_llvm_loadable_module(OperatorDependence
    OperatorDependence.h OperatorDependence.cpp 
    ODUtil.h ODUtil.cpp 
)
