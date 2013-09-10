//
//  MayBeUseful.h
//  SLF
//
//  Created by ycc on 13-9-3.
//  Copyright (c) 2013年 ycc. All rights reserved.
//

#ifndef __SLF__MayBeUseful__
#define __SLF__MayBeUseful__

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


using namespace llvm;

enum FUNC {
    PUSH = 1,
    MEMCPY = 2,
    POP = 3,
    PEEK = 4
};



inline bool IsCallFunc(llvm::Instruction* I, FUNC NAME);

template<class T>
bool ExistOrCreate(T* &p);

#endif /* defined(__SLF__MayBeUseful__) */
