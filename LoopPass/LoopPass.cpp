#include "LoopPass.h"

#include "log4cpp/Category.hh"
#include "log4cpp/Appender.hh"
#include "log4cpp/FileAppender.hh"
#include "log4cpp/OstreamAppender.hh"
#include "log4cpp/Layout.hh"
#include "log4cpp/BasicLayout.hh"
#include "log4cpp/Priority.hh"

#include <list>

using namespace std;
using namespace llvm;

#define DEBUG_FLAG 1
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
    
    logger.setPriority(log4cpp::Priority::WARN);
    logger.addAppender(ap);
    logger.error("log4cpp Init Over");
}

LoopPass::LoopPass() : FunctionPass(ID) {}

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
    AU.addRequired<LoopInfo>();
    AU.addRequired<AliasAnalysis>();
    AU.addRequired<MemoryDependenceAnalysis>();
    AU.addRequired<ScalarEvolution>();
    //AU.setPreservesAll();
}

bool LoopPass::runOnFunction(Function &_F) {
    _InitLogger();
    LI = &getAnalysis<LoopInfo>();
    AA = &getAnalysis<AliasAnalysis>();
    MDA = &getAnalysis<MemoryDependenceAnalysis>();
    SE = &getAnalysis<ScalarEvolution>();
    this->F = &_F;
    
    
    if(F->getName() == "_Z2f1i" || F->getName() == "_Z2f0i")
        ;
    else return false;
    go();
    return true;
}

bool LoopPass::IsBBHasNoDepend(BasicBlock* BB, Loop* L)
{
    for(BasicBlock::iterator I = BB->begin(), IE = BB->end(); I != IE; I++)
    {
        for(int i = 0; i < I->getNumOperands(); i++)
        {
            Value* x = I->getOperand(i);
            if(isa<Constant>(x)) continue;
            if(isa<GlobalValue>(x)) continue;
            if(isa<BasicBlock>(x)) continue;
            if(!isa<Instruction>(x)) continue;
            DEBUG(1, errs() << *x << "\n");
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
    //Preheader between them
    BasicBlock *BB = (L1->getLoopPredecessor())->getPrevNode();
    if(BB != NULL && L2->contains(BB))
        swap(L1, L2);
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
            cnt++;
        }
    return cnt == LI->end() - LI->begin() - 1;
}

Instruction* FindExitInst(BasicBlock* BB)
{
    for(BasicBlock::iterator IE = BB->end(), IB = BB->begin(); --IE != IB; )
    {
        if((*IE).getOpcode() == Instruction::ICmp)
            return IE;
    }
    return NULL;
}

//Find All Inducation Variable
list<PHINode*> GetAllIndvar(Loop* L)
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
void ReplaceIndvar(list<PHINode*> IndvarList, Instruction *sv)
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

void LoopPass::go()
{
    logger.warn("Fucntion : " + (F->getName()).str());
    if(IsAllLoopAdjacent())
    {
        logger.warn("All Loop Adjacent");
        for (LoopInfo::iterator L = LI->begin(), LE = LI->end(); L != LE; L++) {
            LoopTile(*L);
        }
        F->viewCFG();
    }
    else
    {
        logger.warn("Some Loop Not Adjacent\n");
    }
    //F->viewCFG();
}

char LoopPass::ID = 0;
static RegisterPass<LoopPass> X("LoopPass", "My Loop Pass", false, false);


