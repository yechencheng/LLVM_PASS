#include <iostream>
#include <list>
#include <string>
#include <set>

#include <llvm/Pass.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
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

using namespace std;
using namespace llvm;

struct Hello : public FunctionPass {
    static char ID;
    AliasAnalysis *AA;
    MemoryDependenceAnalysis *MDA;
    LoopInfo *LI;
    ScalarEvolution *SE;
    Function *F;
    
    Hello() : FunctionPass(ID) {}
    
    void addInstruction(CallInst& I)
    {
        Value* x = I.getArgOperand(0);
        LoadInst& lx = cast<LoadInst>(*x);
        StoreInst *y = new StoreInst(&I, lx.getPointerOperand());
        y->insertAfter(&I);
    }
    
    void VerifyInst(Function &F)
    {
        for(inst_iterator I = inst_begin(F), IE = inst_end(F); I != IE; I++)
        {
            if(!isa<CallInst>(*I)) continue;
            CallInst& x = cast<CallInst>(*I);
            //x.setOnlyReadsMemory();
            x.setDoesNotAccessMemory();
            
            errs() << x << "\n";
            if(x.mayHaveSideEffects())
                errs() << "\tSide Effect!\n";
            if(x.doesNotReturn())
                errs() << "\tDoesNotReturn!\n";
            if(x.doesNotAccessMemory())
                errs() << "\tDoesNotAccessMemory!\n";
            if(x.getCalledFunction()->getName().find("push") != StringRef::npos)
                addInstruction(x);
        }
    }
    
    virtual void getAnalysisUsage(AnalysisUsage& AU) const
    {
        AU.addRequired<LoopInfo>();
        AU.addRequired<AliasAnalysis>();
        AU.addRequired<MemoryDependenceAnalysis>();
        AU.addRequired<ScalarEvolution>();
        AU.setPreservesAll();
    }
    
    virtual bool runOnFunction(Function &F) {
        if(F.getName() == "_Z2f1i" || F.getName() == "_Z2f0i")
            ;
        else return false;
        LI = &getAnalysis<LoopInfo>();
        AA = &getAnalysis<AliasAnalysis>();
        MDA = &getAnalysis<MemoryDependenceAnalysis>();
        SE = &getAnalysis<ScalarEvolution>();
        this->F = &F;
        go();
        return false;
    }
    
    void go()
    {
        
        for(LoopInfo::iterator L = LI->begin(), LE = LI->end(); L != LE; L++)
        {
            (*L)->print(errs());
            //errs() << *((*L)->getLoopPreheader()) << "\n";
        }
        SE->print(errs());
        //F->viewCFG();
    }
};

char Hello::ID = 0;
static RegisterPass<Hello> X("hello", "MyPass", false, false);


