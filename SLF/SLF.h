//
//  SLF.h
//  Project
//
//  Created by ycc on 13-9-8.
//
//

#ifndef Project_SLF_h
#define Project_SLF_h

#include <iostream>
#include <list>
#include <string>
#include <set>

#include <llvm/Pass.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
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

#include "IOValueParser.h"
#include "Util.h"


class SLF : public FunctionPass {
public:
    static char ID;
    AliasAnalysis *AA;
    MemoryDependenceAnalysis *MDA;
    LoopInfo *LI;
    ScalarEvolution *SE;
    Value *InputValue, *OutputValue;
    Function *F;
    
    SLF();
    bool IsWorkFunction(StringRef str);
    virtual void getAnalysisUsage(AnalysisUsage& AU) const;
    virtual bool runOnFunction(Function &F);
    
private:
    //Methods below are used for test only!!!
    void PrintScalarEvolutionInfo();
};
#endif
