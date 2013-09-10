//
//  IOValueParser.h
//  Project
//
//  Created by ycc on 13-9-4.
//
//

#ifndef __Project__IOValueParser__
#define __Project__IOValueParser__

#include <iostream>

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

#include "Util.h"

using namespace llvm;

class IOValueParser
{
private:
    LoopInfo *LI;
    ScalarEvolution *SE;
    Function *F;
    
    Value* GetOSVBackwardStep(Value* V, std::set<Value*> visited, bool IsGetE =false);
    Value* GetISVForwardStep(Value* X, std::set<Value*> &visited);
    Value* GetInputSourceValue(Value* V);
    Value* GetOutputSourceValue(Value* V);
    
public:
    Value *InputValue, *OutputValue;
    IOValueParser(Function *_F, ScalarEvolution *_SE, LoopInfo *_LI);
    
    bool FindInputValue();
    bool FindOutputValue();
    void OutputInfo();
    
};

#endif /* defined(__Project__IOValueParser__) */
