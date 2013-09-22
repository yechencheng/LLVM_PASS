#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <iostream>

#include "SGO.h"
#include "SLF.h"

using namespace llvm;
using namespace std;

SGO::SGO() : ModulePass(ID)
{}

void SGO::getAnalysisUsage(AnalysisUsage &AU) const
{
    AU.addRequired<SLF>();
    AU.addRequired<LoopInfo>();
    AU.setPreservesAll();
}

bool SGO::runOnModule(Module &M)
{
    for(Module::iterator itr = M.begin(); itr != M.end(); itr++)
    {
        if(itr->isDeclaration()) continue;
        LoopInfo &LI = getAnalysis<LoopInfo>(*itr);
        SLF &x = getAnalysis<SLF>(*itr);
        //x.runOnFunction(*itr);
        
	}
    return false;
}

char SGO::SGO::ID = 0;
static RegisterPass<SGO::SGO> X("SGO", "StreamIt Global Optimizer", false, false);