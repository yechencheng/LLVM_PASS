#include <vector>

#include <llvm/Pass.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/GlobalVariable.h>
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
#include <llvm/Analysis/ScalarEvolutionExpressions.h>
#include <llvm/Analysis/LoopIterator.h>
#include <llvm/Analysis/DependenceAnalysis.h>
#include <llvm/Analysis/Dominators.h>
#include "llvm/Analysis/ValueTracking.h"



using namespace std;
using namespace llvm;



class MyDependence{
public:
    Dependence* DP;
    vector<const SCEV*> Dis;
    MyDependence();
    MyDependence(Dependence *_DP);
    void AddDependence(Dependence* _DP);
    void MergeWith(MyDependence *D, ScalarEvolution *SE);
    void AddWith(MyDependence *D, ScalarEvolution *SE);
    void print();
};

class OperatorDependence : public FunctionPass {
public:
    static char ID;
    AliasAnalysis *AA;
    MemoryDependenceAnalysis *MDA;
    LoopInfo *LI;
    ScalarEvolution *SE;
    Function *F;
    DependenceAnalysis *DA;
    
    OperatorDependence();
    virtual void getAnalysisUsage(AnalysisUsage& AU) const;
    virtual bool runOnFunction(Function &F);
    bool IsFullDepdence();
    bool IsFullDepdence(Loop *L);
    bool IsFullDepdence(BasicBlock* BB);
private:
    map<Loop*, MyDependence*> GlobalDependence;
    void test();
    MyDependence* GetDependence(Instruction *a, Instruction *b);
    MyDependence* GetDependence(Loop *a, Loop *b);
    MyDependence* GetGlobalDependence(BasicBlock *BB, Loop *predecessor, ScalarEvolution *SE);
    MyDependence* GetGlobalDependence(Loop *a, Loop *predecessor, ScalarEvolution *SE);
};