#include "OperatorDependence.h"
#include "ODUtil.h"

#include "log4cpp/Category.hh"
#include "log4cpp/Appender.hh"
#include "log4cpp/FileAppender.hh"
#include "log4cpp/OstreamAppender.hh"
#include "log4cpp/Layout.hh"
#include "log4cpp/BasicLayout.hh"
#include "log4cpp/Priority.hh"
#include <llvm/Support/Debug.h>

#include <list>
#include <map>
#include <vector>
#include <assert.h>

using namespace std;
using namespace llvm;

log4cpp::Category& logger = log4cpp::Category::getRoot();

bool OperatorDependence::runOnFunction(Function &_F) {
    //_InitLogger();
    LI = &getAnalysis<LoopInfo>();
    AA = &getAnalysis<AliasAnalysis>();
    MDA = &getAnalysis<MemoryDependenceAnalysis>();
    SE = &getAnalysis<ScalarEvolution>();
    DA = &getAnalysis<DependenceAnalysis>();
    this->F = &_F;
    
    if(!ToPass(this->F))
        return false;
    test();
    return false;
}

void OperatorDependence::test()
{
    errs() << F->getName() << "\n";
    MyDependence *d = GetGlobalDependence(&F->getEntryBlock(), NULL, SE);
    if(d == NULL) return ;
    d->print();
    //F->viewCFG();
    return ;
    
    for(LoopInfo::iterator L0 = LI->begin(), LE = LI->end(); L0 != LE; L0++)
        for(LoopInfo::iterator L1 = LI->begin(); L1 != LE; L1++)
        {
            MyDependence* rt = GetDependence(*L0, *L1);
            if(rt == NULL) continue;
            if(rt->Dis.size() == 0) continue;
            errs() << **L0 << **L1;
            for(int i = 0; i < rt->Dis.size(); i++)
                errs() << "\t" << *(rt->Dis[i]) << "\n";
            errs() << "\n";
        }
        return ;
    
    for(inst_iterator I0 = inst_begin(F), IE = inst_end(F); I0 != IE; I0++)
    {
        for (inst_iterator I1 = inst_begin(F) ; I1 != IE; I1++)
        {
            MyDependence* D = GetDependence(&*I0, &*I1);
            if(D == NULL) continue;
            errs() << *I0 << "\n" << *I1 << "\n";
            for(int i = 0; i < D->Dis.size(); i++)
                errs() << "\t" << *(D->Dis[i]) << "\n";
        }
    }
}

//Dependence for Instructions
MyDependence* OperatorDependence::GetDependence(Instruction *a, Instruction *b)
{
    if(!IsLoadOrStore(a) || !IsLoadOrStore(b)) return NULL;
    Dependence* d = DA->depends(a, b, true);
    if(d == NULL) return NULL;
    if(!d->isOrdered()) return NULL;
    Loop* La = LI->getLoopFor(a->getParent());
    Loop* Lb = LI->getLoopFor(b->getParent());
    if(La == Lb) return new MyDependence(d);
    
    //OK, Now, there is no common loop
    vector<const SCEV*> Suba = getSubscrit(a, SE);
    vector<const SCEV*> Subb = getSubscrit(b, SE);
    
    if(Suba.size() != Subb.size())
        llvm_unreachable_internal("Should not happen");
    int IsNestedA = LI->getLoopDepth(a->getParent()) <= 1;
    int IsNestedB = LI->getLoopDepth(b->getParent()) <= 1;
    if(Suba.size() - IsNestedA != LI->getLoopDepth(a->getParent()) || Subb.size() - IsNestedB != LI->getLoopDepth(b->getParent()))
    {
        errs() << *a << "\n" << *b << "\n";
        errs() << LI->getLoopDepth(a->getParent()) << "\t" << LI->getLoopDepth(b->getParent()) << "\n";
        errs() << Suba.size() << " " << Subb.size() << "\n";
        F->viewCFG();
        llvm_unreachable_internal("TODO:Loop Depth and Subscrit different!");
    }
    MyDependence *rt = new MyDependence(d);
    for(int i = 0; i < Suba.size(); i++)
    {
        const SCEVAddRecExpr *Ra = dyn_cast<SCEVAddRecExpr>(Suba[i]);
        const SCEVAddRecExpr *Rb = dyn_cast<SCEVAddRecExpr>(Subb[i]);
        if(Ra == NULL || Rb == NULL)
        {
            //errs() << "TODO : Subscrit is loop independent\n";
            continue;
        }
        if(!Ra->isAffine() && !Rb->isAffine())
            llvm_unreachable_internal("TODO : Subscrit is not not affine");
        rt->Dis.push_back(SE->getMinusSCEV(Rb->getOperand(0), Ra->getOperand(0)));
    }
    return rt;
}

MyDependence* OperatorDependence::GetDependence(Loop *a, Loop *b)
{
    if(LI->getLoopDepth(a->getHeader()) != LI->getLoopDepth(b->getHeader()))
        llvm_unreachable_internal("TODO : Loop with different depth");
    
    vector<MyDependence*> deps(0);
    
    for(Loop::block_iterator B0 = a->block_begin(), BE0 = a->block_end(); B0 != BE0; B0++)
        for(Loop::block_iterator B1 = b->block_begin(), BE1 = b->block_end(); B1 != BE1; B1++)
            for(BasicBlock::iterator I0 = (*B0)->begin(), IE0 = (*B0)->end(); I0 != IE0; I0++)
                for(BasicBlock::iterator I1 = (*B1)->begin(), IE1 = (*B1)->end(); I1 != IE1; I1++)
                {
                    MyDependence* d = GetDependence(&*I0, &*I1);
                    if(d == NULL) continue;
                    deps.push_back(d);
                }
    
    MyDependence *rt = new MyDependence();
    for(int i = 0; i < deps.size(); i++)
        for(int j = 0; j < deps[i]->Dis.size(); j++)
        {
            bool issmall = false;
            for(int k = 0; k < rt->Dis.size(); k++)
            {
                if(SE->isKnownPredicate(ICmpInst::ICMP_SGT, rt->Dis[k], deps[i]->Dis[j]))
                    issmall = true;
                else if(SE->isKnownPredicate(ICmpInst::ICMP_SGT, deps[i]->Dis[j], rt->Dis[k]))
                    rt->Dis[k] = deps[i]->Dis[j];
            }
            if(issmall) continue;
            rt->Dis.push_back(deps[i]->Dis[j]);
        }
    sort(rt->Dis.begin(), rt->Dis.end());
    rt->Dis.resize(unique(rt->Dis.begin(), rt->Dis.end()) - rt->Dis.begin());
    return rt;
}

//a should be the outtest loop
MyDependence* OperatorDependence::GetGlobalDependence(Loop *a, Loop* predcessor, ScalarEvolution *SE)
{
    //GET OWN DEP
    GlobalDependence[a] = new MyDependence();
    SmallVector<BasicBlock*, 128> ExitBB;
    a->getExitBlocks(ExitBB);
    
    for(int i = 0; i < ExitBB.size(); i++)
    {
        //find maximal adjacent edge
        GetGlobalDependence(ExitBB[i], a, SE);
        if(predcessor != NULL)
            GetGlobalDependence(ExitBB[i], predcessor, SE);
    }
    
    MyDependence* d = GetDependence(a, a);
    d->AddWith(GlobalDependence[a], SE);
    GlobalDependence[a] = d;
    
    if(predcessor != NULL)
    {
        MyDependence* td = GetDependence(predcessor, a);
        td->AddWith(d, SE);
        GlobalDependence[predcessor]->MergeWith(td, SE);
    }
    return d;
}

MyDependence* OperatorDependence::GetGlobalDependence(BasicBlock *BB, Loop* predecessor, ScalarEvolution *SE)
{
    Loop* L = LI->getLoopFor(BB);
    if(L != NULL) return GetGlobalDependence(L, predecessor, SE);
    
    MyDependence* rt = NULL;
    TerminatorInst *TI = BB->getTerminator();
    if(TI == NULL) return NULL;
    
    for(int i = 0; i < TI->getNumSuccessors(); i++)
    {
        BasicBlock *B = TI->getSuccessor(i);
        MyDependence* x = GetGlobalDependence(B, predecessor, SE);
        if(rt == NULL) rt = x;
        else rt->MergeWith(x, SE);
    }
    
    if(rt != NULL && predecessor != NULL)
        (GlobalDependence[predecessor])->MergeWith(rt, SE);
    return rt;
}

bool OperatorDependence::IsFullDepdence()
{
    set<Loop*> processed;
    for(Function::iterator BB = F->begin(), BBE = F->end(); BB != BBE; BB++)
    {
        Loop* L = LI->getLoopFor(BB);
        
        bool flag = true;
        if(L != NULL)
        {
            if(processed.find(L) != processed.end()) flag = true;
            else{
                flag = IsFullDepdence(L);
                processed.insert(L);
            }
        }
        else
            flag = IsFullDepdence(BB);
        if(flag == false) return false;
    }
    return true;
}

bool IsLoopInvariant(const SCEV* s, ScalarEvolution* SE, Loop *L)
{
    if(L == NULL) return true;
    return SE->isLoopInvariant(s, L) && IsLoopInvariant(s, SE, L->getParentLoop());
}

bool OperatorDependence::IsFullDepdence(BasicBlock *BB)
{
    for(BasicBlock::iterator I = BB->begin(), IE = BB->end(); I != IE; I++)
        for(Instruction::use_iterator U = I->use_begin(), UE = I->use_end(); U != UE; U++)
        {
            if(!isa<Instruction>(*U)) llvm_unreachable_internal("TODO : NOT INSTRUCTION!");
            Instruction *x = cast<Instruction>(*U);
            BasicBlock *TBB = x->getParent();
            if(TBB == BB) continue;
            Loop* L = LI->getLoopFor(TBB);
            if(L == NULL) continue;
            //if(L->isLoopInvariant(x)) continue;
            if(IsLoopInvariant(SE->getSCEV(x), SE, L)) continue;
            if(L->getCanonicalInductionVariable() == x) continue;
            
            errs() << "TODO : IS FULL DEPENDENCE : " << *I << "\n" << "\t" << (*x) << "\n";
            return false;
        }
    
    return true;
}

bool OperatorDependence::IsFullDepdence(Loop *L)
{
    for(Loop::block_iterator BB = L->block_begin(), BBE = L->block_end(); BB != BBE; BB++)
    {
        Loop *TL = LI->getLoopFor(*BB);
        bool flag = true;
        if(TL == L)
            flag = IsFullDepdence(*BB);
        else if(TL->getParentLoop() == L)
            flag = IsFullDepdence(TL);
        else continue;
        if(!TL) return false;
    }
    return true;
}