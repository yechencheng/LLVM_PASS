#include <iostream>
#include <list>
#include <string>
#include <set>

#include <llvm/Pass.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm-c/Core.h>

#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/SmallPtrSet.h>

#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/Regex.h>
#include <llvm/Support/InstIterator.h>
#include <llvm/Support/Casting.h>

#include <llvm/Analysis/LoopInfo.h>
#include <llvm/Analysis/ScalarEvolution.h>
#include <llvm/Analysis/AliasAnalysis.h>
#include <llvm/Analysis/MemoryDependenceAnalysis.h>

#include <llvm/IR/Constants.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/GlobalVariable.h>

#include <llvm/IR/IRBuilder.h>

using namespace std;
using namespace llvm;

class LoopPass : public FunctionPass {
public:
    static char ID;
    AliasAnalysis *AA;
    MemoryDependenceAnalysis *MDA;
    LoopInfo *LI;
    ScalarEvolution *SE;
    Function *F;
    
    LoopPass();
    virtual void getAnalysisUsage(AnalysisUsage& AU) const;
    virtual bool runOnFunction(Function &F);
    void go();
    bool IsAllLoopAdjacent();
    
    
    bool LoopAdjacent(Loop* L1, Loop* L2);
    bool LoopTile(Loop* L);
    bool LoopShift(Loop* L);
    void DistanceVector(Loop* L1, Loop* L2);
    bool IsBBHasNoDepend(BasicBlock* BB, Loop* L);
private:
};