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

bool HasArrayOperation(Function &F, bool ToPrint)
{
    if(ToPrint)
        errs() << (F.getName()).str() << "\n";
    
    bool flag = false;
    bool HasGetPtr = false;
    bool HasPeek = false;
    for(inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I)
    {
        if(I->getOpcode() == Instruction::GetElementPtr)
        {
            if(ToPrint)
                errs() << "\t" << *I << "\n";
            flag = true;
            HasGetPtr = true;
        }
        if(I->getOpcode() == Instruction::Call)
        {
            CallInst &CI = cast<CallInst>(*I);
            Function *X = CI.getCalledFunction();
            StringRef sr = X->getName();
            if(sr.find("push") == StringRef::npos && sr.find("pop") == StringRef::npos && sr.find("peek") == StringRef::npos && sr.find("llvm"))
                flag |= HasArrayOperation(*X, ToPrint);
            if(sr.find("peek") != StringRef::npos)
                HasPeek = true;
        }
    }
    return HasPeek && !HasGetPtr;
    //return flag;
}

bool SLF::runOnFunction(Function &F)
{
    
    
    //if(F.getName() != "_Z27work_CombineDFT__183_48__20i") return false;
    if(!IsWorkFunction(F.getName()))
        return false;
    //cerr << (F.getName()).str() << endl;
    this->F = &F;
    LI = &getAnalysis<LoopInfo>();
    AA = &getAnalysis<AliasAnalysis>();
    MDA = &getAnalysis<MemoryDependenceAnalysis>();
    SE = &getAnalysis<ScalarEvolution>();
    
    if(HasArrayOperation(F, false))
    {
        HasArrayOperation(F, true);
    }
    //PrintScalarEvolutionInfo();
    
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

