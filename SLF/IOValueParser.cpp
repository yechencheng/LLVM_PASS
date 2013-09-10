//
//  IOValueParser.cpp
//  Project
//
//  Created by ycc on 13-9-4.
//
//

#include "IOValueParser.h"


using namespace llvm;

IOValueParser::IOValueParser(Function *_F, ScalarEvolution *_SE, LoopInfo *_LI) : LI(_LI), SE(_SE), F(_F)
{
    InputValue = NULL;
    OutputValue = NULL;
}


//Backward search data dependency, process content copy dependence, such as bitcast, memcpy, store.
//necessary and sufficient condition for SourceValue : 1.the value has name 2. followd by sequences of getelementptr 3.Only one SV
Value* IOValueParser::GetOSVBackwardStep(Value* V, std::set<Value*> visited, bool IsGetE)
{
    if(!isa<Instruction>(V)) return NULL;
    if(visited.find(V) != visited.end()) return NULL;
    visited.insert(V);
    
    Instruction *inst = cast<Instruction>(V);
    //if(IsGetE && inst->getOpcode() != Instruction::GetElementPtr && !(V->getName()).empty())
    //    return V;
    IsGetE = inst->getOpcode() == Instruction::GetElementPtr;
    if(IsGetE) return inst->getOperand(0);
    
    std::list<Value*> rt;
    for(Value::use_iterator U = V->use_begin(), UE = V->use_end(); U != UE; U++)
    {
        if(!isa<Instruction>(*U)) continue;
        Instruction *inst = cast<Instruction>(*U);
        if(inst->getOpcode() == Instruction::BitCast)
            rt.push_back(GetOSVBackwardStep(*U, visited, IsGetE));
        else if(inst->getOpcode() == Instruction::Store && inst->getOperand(1) == V)
            rt.push_back(GetOSVBackwardStep(inst->getOperand(0), visited, IsGetE));
        else if(IsCallFunc(inst, MEMCPY) && inst->getOperand(0) == V)
            rt.push_back(GetOSVBackwardStep(inst->getOperand(1), visited, IsGetE));
    }
    for(Instruction::op_iterator O = inst->op_begin(), OE = inst->op_end(); O != OE; O++)
        rt.push_back(GetOSVBackwardStep(*O, visited, IsGetE));
    
    for(std::list<Value*>::iterator it = rt.begin(); it != rt.end();)
        if(*it == NULL)
            it = rt.erase(it);
        else
            ++it;
    if(rt.size() != 1) return NULL;
    return rt.front();
}

//From %X = pop(), to memcpy(%A, %B); where B is a outter loop variable
Value* IOValueParser::GetISVForwardStep(Value* X, std::set<Value*> &visited)
{
    if(visited.find(X) != visited.end()) return NULL;
    visited.insert(X);
    
    Value* rt = NULL;
    
    Instruction *inst = cast<Instruction>(X);
    if(inst->getOpcode() == Instruction::Store)
        rt = GetOSVBackwardStep(inst->getOperand(1), visited);
    else
        for(Value::use_iterator U = X->use_begin(), UE = X->use_end(); U != UE; U++)
        {
            Value* tmp = GetISVForwardStep(*U, visited);
            if(rt == NULL) rt = tmp;
            else if(tmp != NULL && rt != tmp) return NULL;
        }
    return rt;
}

Value* IOValueParser::GetInputSourceValue(Value* V)
{
    std::set<Value*> visited;
    return GetISVForwardStep(V, visited);
}

bool IOValueParser::FindInputValue()
{
    //TODO : COMMENT FOR APPROCH TO FIND SOURCE
    for(inst_iterator I = inst_begin(F), IE = inst_end(F); I != IE; I++)
    {
        if(IsCallFunc(&*I, POP))
        {
            //find where does it to be stored
            Value* X = GetInputSourceValue(&*I);
            if(X == NULL) continue;
            if(InputValue == NULL) InputValue = X;
            else if(InputValue != X) return false;
        }
    }
    return false;
}

Value* IOValueParser::GetOutputSourceValue(Value* V)
{
    std::set<Value*> visited;
    return GetOSVBackwardStep(V, visited);
}

bool IOValueParser::FindOutputValue()
{
    for(inst_iterator I = inst_begin(F), IE = inst_end(F); I != IE; I++)
    {
        if(IsCallFunc(&*I, PUSH))
        {
            Value* X = GetOutputSourceValue((*I).getOperand(0));
            if(X == NULL) continue;
            if(OutputValue == NULL) OutputValue = X;
            else if(OutputValue != X) return false;
        }
    }
    return true;
}

void IOValueParser::OutputInfo()
{
    errs() << F->getName() << "\n";
    if(InputValue != NULL) errs() << "\t InputValue : " << *InputValue << "\n";
    if(OutputValue != NULL) errs() << "\t OutputValue : " << *OutputValue << "\n";
    
    for(inst_iterator I = inst_begin(F), IE = inst_end(F); I != IE; I++)
        if(I->getValueID() == 69) // 57 : %7 = sext i32 %6 to i64
        {
            const SCEV* sexp = SE->getSCEV(&*I);
            errs() << *I <<  "\n\t" << *sexp << "\t" << SE->hasComputableLoopEvolution(sexp, LI->getLoopFor(I->getParent())) << "\n";
            //break;
        }
}
