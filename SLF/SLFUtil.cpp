//
//  MayBeUseful.cpp
//  SLF
//
//  Created by ycc on 13-9-3.
//  Copyright (c) 2013年 ycc. All rights reserved.
//

#include "SLFUtil.h"


#include <list>

using namespace llvm;




inline bool IsPopFunction(Function* F)
{
    StringRef x = F->getName();
    static Regex expr("_Z[0-9]+__pop__");
    return expr.match(x);
}

inline bool IsPushFunction(Function* F)
{
    StringRef x = F->getName();
    static Regex expr("_Z[0-9]+__push__");
    return expr.match(x);
}

inline bool IsPeekFunction(Function* F)
{
    StringRef x = F->getName();
    static Regex expr("_Z[0-9]+__peek__");
    return expr.match(x);
}

inline bool IsMemcpyFunction(Function* F)
{
    return (F->getName()).startswith("llvm.memcpy.");
}

inline bool IsCallFunc(Instruction* I, FUNC NAME)
{
    if(!isa<CallInst>(I)) return false;
    CallInst* callee = cast<CallInst>(I);
    Function* F = callee->getCalledFunction();
    switch (NAME){
        case PUSH : return IsPushFunction(F);
        case MEMCPY : return IsMemcpyFunction(F);
        case POP : return IsPopFunction(F);
        case PEEK: return IsPeekFunction(F);
    }
    return false;
}

// -1 = Multiple times which is unknow
//TODO : if the iteration for inner loop is clear, then the return value should be a specific value but -1.
//TODO : transive all path to define how many times push will be called in specific path, not just simply add all
int GetInstCallTime(Function &F, LoopInfo &LI)
{
    int cnt = 0;
    for(inst_iterator I = inst_begin(F), E = inst_end(F); I != E; I++)
    {
        if(!isa<CallInst>(*I)) continue;
        CallInst &c = cast<CallInst>(*I);
        if(IsPushFunction(c.getCalledFunction()))
        {
            BasicBlock *BB = c.getParent();
            
            if(LI.getLoopDepth(BB) >= 2)
                return -1;
            else
                cnt++;
        }
    }
    return cnt;
}

bool CanBeOptimize(Function &F, LoopInfo &LI)
{
    int PushTime = GetInstCallTime(F, LI);
    if(PushTime == 0 || PushTime == 1) return false;
    if(PushTime > 1) return false;
    Instruction *push, *peek = NULL, *pop = NULL;
    for(inst_iterator I = inst_begin(F), E = inst_end(F); I != E; I++)
    {
        if(IsCallFunc(&*I, PUSH)) push = &*I;
        else if(IsCallFunc(&*I, POP)) pop = &*I;
        else if(IsCallFunc(&*I, PEEK)) peek = &*I;
    }
    if(pop == NULL) return false;
    if(peek != NULL)
    {
        if(LI[peek->getParent()] == LI[push->getParent()])
            return false;
    }
    if(peek == NULL && LI[pop->getParent()] == LI[push->getParent()]) return false;
    return true;
}


std::list<Instruction*> FindPushInst(Function &F)
{
    std::list<Instruction*> rt;
    
    for(inst_iterator I = inst_begin(F), IE = inst_end(F); I != IE; I++)
    {
        if(IsCallFunc(&*I, PUSH))
            rt.push_back(&*I);
    }
    
    return rt;
}

//1.Find Root Value : **ASSUME**(See PPT, Challenge) that there is only one memcpy statement
void GetAllUse(Instruction* I, int depth = 0)
{
    //errs() << std::string(depth, '\t');
    //I->print(errs()); errs() << "\n";
    if(I->getNumUses() == 0) return ;
    
    
    for(Instruction::use_iterator UI = I->use_begin(), UE = I->use_end(); UI != UE; UI++)
        GetAllUse((Instruction*)(*UI), depth + 1);
}

Instruction* FindRootValue(Instruction* I)    //I is a push call, and find the global variable which is use for **PUSH**
{
    Value* NextI = I->getOperand(0);
    while(isa<Instruction>(*NextI))
    {
        I = (Instruction*)NextI;
        NextI = ((Instruction*)NextI)->getOperand(0);
    }
    
    NextI = I;
    
    return I;
}

Instruction* GetMemcpyInstruction(Instruction* I)   // Input Root Value, Output the memcpy inst used on this value
{
    if(I->getNumUses() == 0)
    {
        if(IsCallFunc(I, MEMCPY))
            return I;
        return NULL;
    }
    for(Instruction::use_iterator UI = I->use_begin(), UE = I->use_end(); UI != UE; UI++)
    {
        Instruction* rt = GetMemcpyInstruction((Instruction*)*UI);
        if(rt != NULL) return rt;
    }
    return NULL;
}

std::list<Instruction*> BuildIndexDepGraph(std::list<Instruction*> LI)
{
    SmallPtrSet<Instruction, 4096> SPS;
    std::list<Instruction*> rt;
    for(std::list<Instruction*>::const_iterator I = LI.begin(); I != LI.end(); ++I)
    {
        Instruction* MemI = GetMemcpyInstruction(FindRootValue(*I));
        MemI->getOperand(1);
    }
    return rt;
}

void GetRelateInst(Instruction *I, std::set<Instruction*> &rt, int dep = 0)
{
    I->print(errs() << std::string(dep, ' '));
    errs() << "\n";
    
    rt.insert(I);
    for(User::op_iterator opS = I->op_begin(), opE = I->op_end(); opS != opE; ++opS)
    {
        Value* v = *opS;
        if(!isa<Instruction>(v)) continue;
        if(rt.find((Instruction*)v) == rt.end())
            GetRelateInst((Instruction*)v, rt, dep + 1);
    }
    for(Value::use_iterator useS = I->use_begin(), useE = I->use_end(); useS != useE; ++useS)
    {
        Instruction* x = (Instruction*)*useS;
        if(rt.find(x) == rt.end())
            GetRelateInst(x, rt, dep + 1);
    }
}

std::set<Instruction*> GetRelateInst(Function& F)
{
    std::set<Instruction*> rt;
    for(inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I)
    {
        if(IsCallFunc(&*I, POP) || IsCallFunc(&*I, PUSH))
            GetRelateInst(&*I, rt);
    }
    return rt;
}
