#include "SLF.h"
#include <iostream>

using namespace llvm;
using namespace std;

SLF::SLF() : FunctionPass(ID), AA(NULL), MDA(NULL), LI(NULL), InputValue(NULL), OutputValue(NULL) {}

bool SLF::IsWorkFunction(StringRef str)
{
    static Regex expr("_Z[0-9]+work_[^_]+_");
    return expr.match(str);
}

void SLF::getAnalysisUsage(AnalysisUsage& AU) const
{
    AU.addRequired<LoopInfo>();
    AU.addRequired<AliasAnalysis>();
    AU.addRequired<MemoryDependenceAnalysis>();
    AU.addRequired<ScalarEvolution>();
    AU.setPreservesAll();
}

bool SLF::runOnFunction(Function &F)
{
    
    
    //if(F.getName() != "_Z27work_CombineDFT__183_48__20i") return false;
    if(!IsWorkFunction(F.getName()))
        return false;
    cerr << (F.getName()).str() << endl;
    this->F = &F;
    LI = &getAnalysis<LoopInfo>();
    AA = &getAnalysis<AliasAnalysis>();
    MDA = &getAnalysis<MemoryDependenceAnalysis>();
    SE = &getAnalysis<ScalarEvolution>();
    
    PrintScalarEvolutionInfo();
    
    /*IOValueParser x(&F, SE, LI);
    x.FindInputValue();
    x.FindOutputValue();
    x.OutputInfo();
    */
    
    //F.viewCFG();
    return false;   //Not change code
}





char SLF::SLF::ID = 0;
static RegisterPass<SLF::SLF> X("SLF", "StreamIt Loop Fission", false, false);

