#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <iostream>

#include "SGO.h"
//#include "SLF.h"
#include "LoopPass.h"
#include <llvm/Analysis/DependenceAnalysis.h>
#include <llvm/Analysis/ScalarEvolution.h>
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/Analysis/AliasAnalysis.h>
#include <llvm/Analysis/Dominators.h>
#include <llvm/Analysis/MemoryDependenceAnalysis.h>

using namespace llvm;
using namespace std;

SGO::SGO() : ModulePass(ID)
{}

void SGO::getAnalysisUsage(AnalysisUsage &AU) const
{
    //AU.addRequired<SLF>();
    AU.addRequired<LoopPass>();
    //AU.addRequired<AliasAnalysis>();
    AU.addRequiredTransitive<AliasAnalysis>();
    //AU.setPreservesAll();
}

bool SGO::runOnModule(Module &M)
{
    
    return false;
    LoopPass *LP0, *LP1;
    for(Module::iterator itr = M.begin(); itr != M.end(); itr++)
    {
        if(itr->isDeclaration()) continue;
        //LoopInfo &LI = getAnalysis<LoopInfo>(*itr);
        //SLF &x = getAnalysis<SLF>(*itr);
        
        LoopPass &LP = getAnalysis<LoopPass>(*itr);
        if((itr->getName()) == "_Z2f0i")
            LP0 = &LP;
        else if(itr->getName() == "_Z2f1x")
            LP1 = &LP;
        else continue;
        if(LP.NeedShift == NULL) continue;
        errs() << "VALUE : " << *(LP.NeedShift) << "\n";
        //LP.SE->print(errs());
        
    }
    
    return false;
}

char SGO::SGO::ID = 0;
static RegisterPass<SGO::SGO> X("SGO", "StreamIt Global Optimizer", false, false);