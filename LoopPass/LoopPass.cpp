#include "LoopPass.h"


#include "log4cpp/Category.hh"
#include "log4cpp/Appender.hh"
#include "log4cpp/FileAppender.hh"
#include "log4cpp/OstreamAppender.hh"
#include "log4cpp/Layout.hh"
#include "log4cpp/BasicLayout.hh"
#include "log4cpp/Priority.hh"
#include <llvm/Target/TargetLibraryInfo.h>



#include <list>
#include <map>
#include <vector>
#include <assert.h>

using namespace std;
using namespace llvm;

#define DEBUG_FLAG 2
#include "Debug.h"

//Log4cpp
log4cpp::Category& logger = log4cpp::Category::getRoot();
bool _log4cppInit = false;
void _InitLogger()
{
    if(_log4cppInit == true) return;
    _log4cppInit = true;
    log4cpp::Appender *ap = new log4cpp::OstreamAppender("console", &std::cout);
    ap->setLayout(new log4cpp::BasicLayout());
    
    logger.setPriority(log4cpp::Priority::INFO);
    logger.addAppender(ap);
    logger.error("log4cpp Init Over");
}


LoopPass::LoopPass() : FunctionPass(ID) { NeedShift = NULL; }

/*
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
 */

void LoopPass::getAnalysisUsage(AnalysisUsage& AU) const
{
    AU.addRequiredTransitive<AliasAnalysis>();
    AU.addRequired<LoopInfo>();
    //AU.addRequired<AliasAnalysis>();
    AU.addRequired<MemoryDependenceAnalysis>();
    AU.addRequired<ScalarEvolution>();
    AU.addRequired<DependenceAnalysis>();
    AU.setPreservesAll();
}

bool LoopPass::runOnFunction(Function &_F) {
    _InitLogger();
    LI = &getAnalysis<LoopInfo>();
    AA = &getAnalysis<AliasAnalysis>();
    MDA = &getAnalysis<MemoryDependenceAnalysis>();
    SE = &getAnalysis<ScalarEvolution>();
    DA = &getAnalysis<DependenceAnalysis>();
    this->F = &_F;
    go();
    //NewGo();
    return false;
}

Value *getPointerOperand(Instruction *I) {
    if (LoadInst *LI = dyn_cast<LoadInst>(I))
        return LI->getPointerOperand();
    if (StoreInst *SI = dyn_cast<StoreInst>(I))
        return SI->getPointerOperand();
    llvm_unreachable("Value is not load or store instruction");
    return 0;
}

const Value* MyGetUnderlyingObject(const Value* V);

const Value* GetUnderlyingObject(const PHINode *P)
{
    vector<const Value*> rt(0);
    for(int i = 0; i < P->getNumIncomingValues(); i++)
    {
        const Value* base = MyGetUnderlyingObject(P->getIncomingValue(i));
        if(isa<PHINode>(base)) continue;
        rt.push_back(base);
    }
    for(int i = 1; i < rt.size(); i++)
        assert(rt[i] == rt[0]);
    return rt[0];
}

const Value* MyGetUnderlyingObject(const Value* V)
{
    if(isa<PHINode>(V))
    {
        const PHINode* P = cast<const PHINode>(V);
        return GetUnderlyingObject(P);
    }
    else
        return GetUnderlyingObject(V);
}



static
AliasAnalysis::AliasResult underlyingObjectsAlias(AliasAnalysis *AA,
                                                  const Value *A,
                                                  const Value *B) {
    const Value *AObj = MyGetUnderlyingObject(A);
    const Value *BObj = MyGetUnderlyingObject(B);
    errs() << *AObj << "\n" << *BObj << "\n";
    return AA->alias(AObj, AA->getTypeStoreSize(AObj->getType()),
                     BObj, AA->getTypeStoreSize(BObj->getType()));
}

void LoopPass::NewGo()
{
    errs() << F->getName() << "\n";
    if((F->getName()).find("_Z2f1") == StringRef::npos) return ;
    for(inst_iterator I0 = inst_begin(F), IE = inst_end(F); I0 != IE; I0++)
    {
        for(inst_iterator I1 = inst_begin(F); I1 != IE; I1++)
        {
            if(LI->getLoopFor(I0->getParent()) == NULL || LI->getLoopFor(I1->getParent()) == NULL)
                continue;
            if(LI->getLoopFor(I0->getParent()) == LI->getLoopFor(I1->getParent()))
                continue;
            if(I0->getOpcode() == Instruction::Store && I1->getOpcode() == Instruction::Load)
            {
                errs() << "\n" << *I0 << "\n" << *I1 << "\n";
                /*
                Value *SrcPtr = getPointerOperand(&*I0);
                Value *DstPtr = getPointerOperand(&*I1);
                errs() << *SrcPtr << "\n" << *DstPtr << "\n";
                underlyingObjectsAlias(AA, SrcPtr, DstPtr);
                */
                Dependence* d = DA->depends(&*I0, &*I1, true);
                if(d == NULL) {
                    errs() << "NO DEP\n";
                    continue;
                };
                d->dump(errs());
                if(d->isConfused()) continue;
                for(int i = 1; i < 7; i++)
                {
                    const SCEV* x = d->getDistance(i);
                    if(x != NULL)
                        errs() << *x << "\n";
                }
                return;
                
            }
            //if(d->isConsistent()) continue;
            //errs() << "DEP : " << d->getLevels() << "\n";
            //errs() << *I0 << "\n" << *I1 << "\n";
        }
    }
    //F->viewCFG();
}

bool LoopPass::IsBBHasNoDepend(BasicBlock* BB, Loop* L)
{
    for(BasicBlock::iterator I = BB->begin(), IE = BB->end(); I != IE; I++)
    {
        if(I->getOpcode() == Instruction::Alloca) continue;
        for(int i = 0; i < I->getNumOperands(); i++)
        {
            Value* x = I->getOperand(i);
            if(isa<Constant>(x)) continue;
            if(isa<GlobalValue>(x)) continue;
            if(isa<BasicBlock>(x)) continue;
            if(!isa<Instruction>(x)) continue;
            Instruction* ix = cast<Instruction>(x);
            if((LI->getLoopFor(ix->getParent())) == NULL)
                continue;
            //DEBUG(1, errs() << *BB);
            //DEBUG(1, errs() << "Local Instruction !!! " << *I << "\n\t" << *x << "\n");
            //F->viewCFG();
            return false;
        }
    }
    return true;
}

bool LoopPass::LoopAdjacent(Loop* L1, Loop* L2)
{
    //Direct Adjacent
    if(L2->contains(L1->getLoopPredecessor()) || L1->contains(L2->getLoopPredecessor()))
        return true;
    
    if(L1->getLoopPredecessor() == NULL) return false;
    //Preheader between them , Let L2 be successor
    BasicBlock *BB = (L1->getLoopPredecessor());
    if(BB->getNumUses() == 0) return false;
    BB = BB->getPrevNode();
    if(BB != NULL && L2->contains(BB))
        swap(L1, L2);
    else
    {
        BB = (L2->getLoopPredecessor());
        if(BB->getNumUses() == 0) return false;
        BB = BB->getPrevNode();
        if(BB != NULL && L1->contains(BB));
        else return false;
    }
    DEBUG(0, errs() << *L1 << "\n" << *L2 << "\n");
    return IsBBHasNoDepend(L2->getLoopPredecessor(), L1);
}

bool LoopPass::IsAllLoopAdjacent()
{
    int cnt = 0;
    for(LoopInfo::iterator L = LI->begin(), LE = LI->end(); L != LE; L++)
        for(LoopInfo::iterator LN = L + 1; LN != LE; LN++)
        {
            if(!LoopAdjacent(*L, *LN))
                continue;
            DEBUG(0, errs() << **L << "\t" << **LN << "\n");
            cnt++;
        }
    return cnt == LI->end() - LI->begin() - 1;
}

Instruction* LoopPass::FindExitInst(BasicBlock* BB)
{
    TerminatorInst *TI = BB->getTerminator();
    Value* CmpInst = TI->getOperand(0);
    if(isa<Instruction>(CmpInst))
    {
        Instruction* I = cast<Instruction>(CmpInst);
        if(I->getOpcode() == Instruction::ICmp)
            return I;
        return NULL;
    }
    return NULL;
}

//Find All Inducation Variable
list<PHINode*> LoopPass::GetAllIndvar(Loop* L)
{
    list<PHINode*> rt(0);
    BasicBlock *H = L->getHeader();
    BasicBlock *Incoming = 0, *Backedge = 0;
    pred_iterator PI = pred_begin(H);
    Backedge = *PI++;
    if (PI == pred_end(H)) return rt;
    Incoming = *PI++;
    if (PI != pred_end(H)) return rt;
    
    if (L->contains(Incoming)) {
        if (L->contains(Backedge))
            return rt;
        swap(Incoming, Backedge);
    }
    else if(!L->contains(Backedge))
        return rt;
    for(BasicBlock::iterator I = (L->getHeader())->begin(); isa<PHINode>(I); I++)
    {
        PHINode *PN = cast<PHINode>(I);
        if (ConstantInt *CI = dyn_cast<ConstantInt>(PN->getIncomingValueForBlock(Incoming)))
            if (CI->isNullValue())
                if(Instruction* Inc = dyn_cast<Instruction>(PN->getIncomingValueForBlock(Backedge)))
                    if(Inc->getOpcode() == Instruction::Add && Inc->getOperand(0) == PN)
                        if(dyn_cast<ConstantInt>(Inc->getOperand(1)))
                            rt.push_back(PN);
    }
    return rt;
}

//Tile Induction Variable and Keep Data Related Induction Value
void LoopPass::ReplaceIndvar(list<PHINode*> IndvarList, Instruction *sv)
{
    for(list<PHINode*>::iterator itr = IndvarList.begin(), itrE = IndvarList.end(); itr != itrE; itr++)
    {
        //COPY NODE
        PHINode* PN = *itr;
        IRBuilder<> Builder(*itr);
        PHINode* CPN = Builder.CreatePHI(PN->getType(), 2);
        Instruction* AddInst = cast<Instruction>(PN->getIncomingValue(0));
        
        Builder.SetInsertPoint(AddInst);
        Value* IncomInst = Builder.CreateAdd(CPN, AddInst->getOperand(1));
        
        CPN->addIncoming(IncomInst, AddInst->getParent());
        if(sv->getType() != IncomInst->getType())
        {
            BasicBlock::iterator pos = (sv->getParent())->begin();
            while(&*(pos++) != sv)
                ;
            Builder.SetInsertPoint(pos);
            
            Value* x = Builder.CreateSExtOrTrunc(sv, IncomInst->getType());
            CPN->addIncoming(x, PN->getIncomingBlock(1));
        }
        else
            CPN->addIncoming(sv, sv->getParent());
        
        DEBUG(0, errs() << *(PN->getParent()));
        
        
        
        //Find Instruction Must Be Replace
        for(Value::use_iterator U = PN->use_begin(), UE = PN->use_end(); U != UE; U++)
        {
            if(*U == PN->getIncomingValue(0)) continue;
            if(!isa<Instruction>(*U))
                continue;
            Instruction* I = cast<Instruction>(*U);
            if(I->getOpcode() == Instruction::GetElementPtr)
            {
                Value* x = I->getOperand(0);
                if(!isa<GlobalValue>(x))
                    continue;
                I->setOperand(2, CPN);
            }
            else
            {
                if(I->getOpcode() == Instruction::Trunc) //Wrong Solution: Need to propagate from branch inst
                    continue;
                I->replaceUsesOfWith(PN, CPN);
            }
        }
    }
}

bool LoopPass::LoopTile(Loop* L)
{
    DEBUG(0, errs() << *L);
    const SCEV* TripCount = SE->getBackedgeTakenCount(L);
    DEBUG(0, errs() << "Trip Count : " << *TripCount << "\n");
    
    //Modify Bound Variable
    BasicBlock* BB = L->getExitingBlock();
    Instruction* ExitInst = FindExitInst(BB);
    Value* UpperBound = ExitInst->getOperand(1);
    Type* BoundType = UpperBound->getType();
    DEBUG(0, errs() << *BB);
    DEBUG(0, errs() << "Before Modify : " << *ExitInst << "\n");
    
    
    Value* newBound = ConstantInt::get(BoundType, 64);
    ExitInst->setOperand(1, newBound);
    DEBUG(0, errs() << "After Modify : " << *ExitInst << "\n");
    
    //Generate Statful Code
    GlobalVariable* SValue =  new GlobalVariable(*(F->getParent()), BoundType, false, GlobalValue::InternalLinkage, ConstantInt::get(BoundType, -64));
    BasicBlock* PreBB = &(F->getEntryBlock());
    
    IRBuilder<> Builder(PreBB->getFirstNonPHI());
    LoadInst *l0 = Builder.CreateLoad(SValue);
    Value* x = Builder.CreateAdd(l0, newBound);
    DEBUG(0, PreBB->print(errs()));
    
    //Replace Index Code with Stateful
    list<PHINode*> IndvarList = GetAllIndvar(L);
    ReplaceIndvar(IndvarList, cast<Instruction>(x));
    
    //Change Loop Scalar
    PHINode* Indvar = L->getCanonicalInductionVariable();
    DEBUG(0, errs() << *Indvar << "\n");
    return false;
}

Value* LoopPass::GetLastCmpInst(TerminatorInst* TI)
{
    return TI->getOperand(0);
}

/*
void VerifyBBClone(vector<BasicBlock*> BBList)
{
    map<BasicBlock*, BasicBlock*> B2B;
    
    for(int i = 0; i < BBList.size(); i++)
    {
        ValueToValueMap V2V(0);
        BasicBlock* NewBB = CloneBasicBlock(BBList[i], V2V, ".NewBB");
        B2B[BBList[i]] = NewBB;
    }
    
    for(int i = 0; i < BBList.size(); i++)
    {
        BasicBlock* NewBB = B2B[BBList[i]];
        TerminatorInst *TI = NewBB->getTerminator();
        
        for(int i = 0; i < TI->getNumOperands(); i++)
        {
            Value* x = TI->getOperand(i);
            if(!isa<BasicBlock>(x))
                continue;
            BasicBlock* TargetBB = cast<BasicBlock>(x);
            if(B2B.find(TargetBB) == B2B.end())
                continue;
            BasicBlock* NewTBB = B2B[TargetBB];
            TI->setOperand(i, NewTBB);
        }
    }
    
    for(int i = 0; i < BBList.size(); i++)
    {
        BasicBlock* NewBB = B2B[BBList[i]];
        
        for(BasicBlock::iterator I = NewBB->begin(); I != NewBB->end(); I++)
        {
            PHINode *p = dyn_cast<PHINode>(I);
            if(p == NULL) continue;
            for(int i = 0; i < p->getNumIncomingValues(); i++)
            {
                BasicBlock *IB = p->getIncomingBlock(i);
                if(B2B.find(IB) == B2B.end()) continue;
                p->setIncomingBlock(i, B2B[IB]);
            }
        }
    }
}
*/
/*
 Split Loop into 2 parts, for example:
 for i, 0, n
 ...
 TO
 for i, 0, prework
 ...
 for i, prework, n
 ...
 */
void LoopPass::LoopSplit(Loop* L, SCEV* ShiftValue)
{
    BasicBlock *BB = L->getLoopPredecessor();
    TerminatorInst *TI = BB->getTerminator();
    
    //Get Shift Value in Constant -- TODO: Process Variable Value
    ConstantRange range = SE->getSignedRange(ShiftValue);
    //errs() << range.getSignedMin() << " " << range.getSignedMax() << "\n";
    assert(range.getSignedMax() == range.getSignedMax());
    uint64_t v = (range.getSignedMax()).getLimitedValue();
    
    //Loop Clone
    /*
     LoopBlocksDFS LoopTraver(L);
     LoopTraver.perform(LI);
     ValueToValueMapTy VMap;
     Loop* NewLoop = new Loop();
     
     vector<BasicBlock*> NewBBList;
     for(LoopBlocksDFS::POIterator LBI = LoopTraver.beginPostorder(); LBI != LoopTraver.endPostorder(); LBI++)
     {
     BasicBlock *NewBB = CloneBasicBlock(*LBI, VMap, ".CLONE");
     NewBBList.push_back(NewBB);
     NewLoop->addBasicBlockToLoop(NewBB, LI->getBase());
     }
     for(int i = 0; i < NewBBList.size(); i++)
     for(BasicBlock::iterator I = NewBBList[i]->begin(), IE = NewBBList[i]->end(); I != IE; I++)
     RemapInstruction(&*I, VMap);
     */
    
    
}



bool LoopPass::GetNextLoop(BasicBlock* BB, vector<Loop*>& result)
{
    Loop* rt = LI->getLoopFor(BB);
    if(rt != NULL)
    {
        result.push_back(rt);
        return true;
    }
    
    bool found = false;
    TerminatorInst *TI = BB->getTerminator();
    for(int i = 0; i < TI->getNumSuccessors(); i++)
        found |= GetNextLoop(TI->getSuccessor(i), result);
    return found;
}

void LoopPass::GetNextLoop(Loop *L, vector<Loop*>& result)
{
    BasicBlock* TB =  L->getExitingBlock();
    assert(TB != NULL);
    TerminatorInst *TI = TB->getTerminator();
    for(int i = 0; i < TI->getNumSuccessors(); i++)
    {
        if(L->contains(TI->getSuccessor(i))) continue;
        GetNextLoop(TI->getSuccessor(i), result);
    }
}


const SCEV* LoopPass::GetShiftCount(Loop* L) //TODO: Should not count adjacent loops only!!!
{
    if(ShiftCount.find(L) != ShiftCount.end())
    {
        return ShiftCount[L];
    }
    vector<Loop*> nextLoop(0);
    GetNextLoop(L, nextLoop);
    const SCEV* result = NULL;
    //DEBUG(2, errs() << *L;);
    for(int i = 0; i < nextLoop.size(); i++)
    {
        DEBUG(0, errs() << "\t" << *nextLoop[i];);
        const SCEV* temp = GetShiftCount(nextLoop[i]);
        DisList* dl = LoopDis[make_pair(L, nextLoop[i])];
        const SCEV* maxValue = NULL;
        for(int i = 0; i < dl->size(); i++)
        {
            maxValue = (maxValue == NULL) ? dl->at(i) : SE->getSMinExpr(maxValue, dl->at(i));
        }
        if(maxValue != NULL)
            temp = SE->getAddExpr(temp, maxValue);
        result = result == NULL ? temp : SE->getSMinExpr(result, temp);
    }
    if(result == NULL)
        result = SE->getConstant(APInt(64, 0));
    return ShiftCount[L] = result;
}

void LoopPass::go()
{
    if(LI->end() - LI->begin() == 0) return ;
    logger.info("Fucntion : " + (F->getName()).str());
    //DEBUG(2, SE->print(errs()));
    //DEBUG(2, F->viewCFG());
    if(IsAllLoopAdjacent())
    {
        
        logger.info("All Loop Adjacent");
        for (LoopInfo::iterator L0 = LI->begin(), LE = LI->end(); L0 != LE; L0++) {
            for (LoopInfo::iterator L1 = LI->begin(); L1 != LE; L1++)
            {
                errs() << **L0 << "\t" << **L1 << "\n";
                DisList* dis = GetDepDistance(*L0, *L1);
                LoopDis[make_pair(*L0, *L1)] = dis;
                DEBUG(0,
                      for(vector<const SCEV*>::iterator itr = dis->begin(), itrE = dis->end(); itr != itrE; itr++)
                      errs() << **itr << "\t" << **L0 << "\t" << **L1 << "\n";);
            }
        }
        for(LoopInfo::iterator L = LI->begin(), LE = LI->end(); L != LE; L++)
        {
            const SCEV* rt = GetShiftCount(*L);
            DEBUG(2, errs() << *rt << "\n");
            if(NeedShift == NULL) NeedShift = rt;
            else{
                NeedShift = SE->getSMinExpr(rt, NeedShift);
            }
        }
        //LoopTile(*L);
    }
    else
    {
        logger.info("Some Loop Not Adjacent\n");
    }
    //F->viewCFG();
}

Value* LoopPass::GetBaseValue(const SCEV *S) {
    if (const SCEVAddRecExpr *AR = dyn_cast<SCEVAddRecExpr>(S)) {
        return GetBaseValue(AR->getStart());
    } else if (const SCEVAddExpr *A = dyn_cast<SCEVAddExpr>(S)) {
        const SCEV *Last = A->getOperand(A->getNumOperands()-1);
        if (Last->getType()->isPointerTy())
            return GetBaseValue(Last);
    } else if (const SCEVUnknown *U = dyn_cast<SCEVUnknown>(S)) {
        // This is a leaf node.
        return U->getValue();
    }
    // No Identified object found.
    return 0;
}

DisList* LoopPass::GetDepDistance(Loop* L)
{
    DisList* rt = new DisList(0);
    LoopBlocksDFS LoopTraver(L);
    LoopTraver.perform(LI);
    
    //Check PHI Node -- Variable Dependence
    //TODO : replace with dependence analysis in llvm
    for(LoopBlocksDFS::POIterator B0 = LoopTraver.beginPostorder(), BE0 = LoopTraver.endPostorder(); B0 != BE0; B0++)
        for(BasicBlock::iterator I0 = (*B0)->begin(), IE0 = (*B0)->end(); I0 != IE0; I0++)
        {
            if(I0->getOpcode() != Instruction::PHI)
                continue;
            for(Instruction::use_iterator U = I0->use_begin(), UE = I0->use_end(); U != UE; U++)
            {
                if(!isa<Instruction>(*U)) continue;
                Instruction* iu = cast<Instruction>(*U);
                if(!L->contains(iu))
                    rt->push_back(SE->getConstant(APInt(64, 4)));   // 4 = sizeof(int), TODO: Fix
            }
        }
    return rt;
}

const SCEV* LoopPass::GetDistance(const SCEVAddRecExpr *S0, const SCEVAddRecExpr *S1)
{
    Value* base0 = GetBaseValue(S0);
    Value* base1 = GetBaseValue(S1);
    if(AA->alias(base0, base1) == AliasAnalysis::NoAlias) return NULL;
    //TODO : Fix here
    if(base0 != base1) return NULL;
    
    //TODO : Loop With Different Depth
    //SS1 is nested loop and is load
    
    if(S0->getLoop()->getLoopDepth() != S1->getLoop()->getLoopDepth())
    {
        const SCEVAddRecExpr* SS0 = S0;
        const SCEVAddRecExpr* SS1 = S1;
        if(SS1->getLoop()->getLoopDepth() > SS0->getLoop()->getLoopDepth())
        {
            int dif = SS1->getLoop()->getLoopDepth() - SS0->getLoop()->getLoopDepth();
            
            const SCEV* rtSCEV = SE->getMulExpr(SE->getBackedgeTakenCount(SS1->getLoop()), SS1->getOperand(SS1->getNumOperands() - 1));
            for(int i = SS1->getNumOperands() - 2; i >= SS1->getNumOperands() - dif; i--)
            {
                const SCEV* x = SS1->getOperand(i);
                const SCEVAddRecExpr* y = cast<SCEVAddRecExpr>(x);
                rtSCEV = (SE->getAddExpr(SE->getMulExpr(SE->getBackedgeTakenCount(y->getLoop()), y->getOperand(i)), rtSCEV));
            }
            return SE->getNegativeSCEV(rtSCEV);
        }
        else
        {
            errs() << "More Store Than Load\n";
            return NULL;
        }
        //TODO:SS0 is nested and is store
    }
    
    //TS0 = (a, +, x), TS1 = (b, +, y)  TODO : Implement Mod for scalar evolution
    // y % x != 0, NO Dependence
    if(S0->getNumOperands() == 2)
    {
        const SCEVConstant* CS0 = dyn_cast<SCEVConstant>(S0->getOperand(1));
        const SCEVConstant* CS1 = dyn_cast<SCEVConstant>(S1->getOperand(1));
        if(CS0 != NULL && CS1 != NULL)
        {
            const ConstantRange &CR1 = (SE->getSignedRange(CS0));
            const ConstantRange &CR2 = (SE->getSignedRange(CS1));
            int a = (CR1.getLower()).getLimitedValue();
            int b = (CR2.getLower()).getLimitedValue();
            if(b % a != 0) return NULL;
        }
    }

    //S0 and S1 belong different loop
    //We will employ Loop Simplify and Induction Variable Strength at first
    const SCEV* TS0 = SE->getAddRecExpr(S0->getStart(), S0->getStepRecurrence(*SE), S0->getLoop(), S0->getNoWrapFlags());
    const SCEV* TS1 = SE->getAddRecExpr(S1->getStart(), S0->getStepRecurrence(*SE), S0->getLoop(), S1->getNoWrapFlags());
    
    
    const SCEV* dis = SE->getMinusSCEV(TS0, TS1);
    if(dis->isZero()) return NULL;
    DEBUG(0, errs() << "\t" << *dis << "\n";);
    
    return dis;
}

DisList* LoopPass::GetDepDistance(Loop* L1, Loop* L2)
{
    DisList* rt = new DisList(0);
    LoopBlocksDFS LoopTraver1(L1);
    LoopBlocksDFS LoopTraver2(L2);
    LoopTraver1.perform(LI);
    LoopTraver2.perform(LI);
    
    for(LoopBlocksDFS::POIterator B0 = LoopTraver1.beginPostorder(), BE0 = LoopTraver1.endPostorder(); B0 != BE0; B0++)
    {
        for(LoopBlocksDFS::POIterator B1 = LoopTraver2.beginPostorder(), BE1 = LoopTraver2.endPostorder(); B1 != BE1; B1++)
        {
            for(BasicBlock::iterator I0 = (*B0)->begin(), IE0 = (*B0)->end(); I0 != IE0; I0++) if(I0->getOpcode() == Instruction::Store)
                for(BasicBlock::iterator I1 = (*B1)->begin(), IE1 = (*B1)->end(); I1 != IE1; I1++) if(I1->getOpcode() == Instruction::Load || I1->getOpcode() == Instruction::Store)
                {
                    
                    const SCEVAddRecExpr* S0 = dyn_cast<SCEVAddRecExpr>(SE->getSCEV(I0->getOperand(1)));
                    const SCEVAddRecExpr* S1 = dyn_cast<SCEVAddRecExpr>(SE->getSCEV(I1->getOperand(0)));
                    if(S0 == NULL || S1 == NULL) continue;
                    
                    const SCEV* dis = GetDistance(S0, S1);
                    if(dis != NULL)
                        rt->push_back(dis);
                }
        }
    }
    if(L1 == L2)
    {
        DisList* temp = GetDepDistance(L1);
        rt->insert(rt->end(), temp->begin(), temp->end());
    }
    return rt;
}

char LoopPass::ID = 1;
static RegisterPass<LoopPass> X("LoopPass", "My Loop Pass", false, false);


