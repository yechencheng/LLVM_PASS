#ifndef __SGO__SGO__
#define __SGO__SGO__

#include <llvm/IR/Module.h>
#include <llvm/Pass.h>

#include "SITDotParser.h"

using namespace llvm;

class SGO : public ModulePass{
private:
    SITGraph *GlobalDepenGraph;
public:
    static char ID;
    SGO();
    
    virtual bool runOnModule(Module &M);
    
};
#endif