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



void _InitLogger();
Value *getPointerOperand(Instruction *I);
vector<const SCEV*> getSubscrit(Instruction* I, ScalarEvolution *SE);
bool IsLoadOrStore(Instruction *I);



//TEST METHOD
bool ToPass(Function *F);