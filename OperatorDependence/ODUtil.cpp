#include "OperatorDependence.h"
#include "ODUtil.h"

#include "log4cpp/Category.hh"
#include "log4cpp/Appender.hh"
#include "log4cpp/FileAppender.hh"
#include "log4cpp/OstreamAppender.hh"
#include "log4cpp/Layout.hh"
#include "log4cpp/BasicLayout.hh"
#include "log4cpp/Priority.hh"
#include <llvm/Target/TargetLibraryInfo.h>
#include <llvm/Support/Debug.h>


#include <list>
#include <map>
#include <vector>
#include <assert.h>

//Fixed Method

MyDependence::MyDependence() : DP(NULL) { Dis.clear(); }
void MyDependence::AddDependence(Dependence* _DP)
{
    DP = _DP;
    for(int i = 1; i <= DP->getLevels(); i++)
    {
        if(DP->getDistance(i) != NULL)
            Dis.push_back(DP->getDistance(i));
    }
}
MyDependence::MyDependence(Dependence* _DP)
{
    MyDependence();
    AddDependence(_DP);
}

OperatorDependence::OperatorDependence() : FunctionPass(ID) { }

char OperatorDependence::ID = 1;
static RegisterPass<OperatorDependence> X("OperatorDependence", "OperatorDependence", false, false);

bool _log4cppInit = false;
void _InitLogger(log4cpp::Category& logger)
{
    if(_log4cppInit == true) return;
    _log4cppInit = true;
    log4cpp::Appender *ap = new log4cpp::OstreamAppender("console", &std::cout);
    ap->setLayout(new log4cpp::BasicLayout());
    
    logger.setPriority(log4cpp::Priority::INFO);
    logger.addAppender(ap);
    logger.error("log4cpp Init Over");
}

void OperatorDependence::getAnalysisUsage(AnalysisUsage& AU) const
{
    AU.addRequired<LoopInfo>();
    AU.addRequired<AliasAnalysis>();
    AU.addRequired<MemoryDependenceAnalysis>();
    AU.addRequired<ScalarEvolution>();
    AU.addRequired<DependenceAnalysis>();
    AU.setPreservesAll();
}

void MyDependence::MergeWith(MyDependence *D, ScalarEvolution *SE)
{
    if(D == NULL || D->Dis.size() == 0) return;
    if(Dis.size() == 0)
    {
        Dis = D->Dis;
        return ;
    }
    
    if(Dis.size() != D->Dis.size())
        llvm_unreachable_internal("TODO : MERGE DIFFERENT DIS SIZE");
    for(int i = 0; i < Dis.size(); i++)
    {
        if(SE->isKnownPredicate(ICmpInst::ICMP_SGE, D->Dis[i], Dis[i]))
            Dis[i] = D->Dis[i];
        else if(SE->isKnownPredicate(ICmpInst::ICMP_SGE, Dis[i], D->Dis[i]))
            continue;
        else
            llvm_unreachable_internal("TODO : Can not compare!");
    }
}

void MyDependence::AddWith(MyDependence *D, ScalarEvolution *SE)
{
    if(D == NULL || D->Dis.size() == 0) return;
    if(Dis.size() == 0)
    {
        Dis = D->Dis;
        return ;
    }
    
    if(Dis.size() != D->Dis.size())
        llvm_unreachable_internal("TODO : ADD DIFFERENT DIS SIZE");
    for(int i = 0; i < Dis.size(); i++)
        Dis[i] = SE->getAddExpr(Dis[i], D->Dis[i]);
}

void MyDependence::print()
{
    for(int i = 0; i < Dis.size(); i++)
        errs() << *Dis[i] << "\t";
    errs() << "\n";
}

/***************************************************************************************/
//Util Method

bool IsLoadOrStore(Instruction *I)
{
    if(isa<LoadInst>(I)) return true;
    else if(isa<StoreInst>(I)) return true;
    else return false;
}

Value *getPointerOperand(Instruction *I) {
    if (LoadInst *LI = dyn_cast<LoadInst>(I))
        return LI->getPointerOperand();
    if (StoreInst *SI = dyn_cast<StoreInst>(I))
        return SI->getPointerOperand();
    llvm_unreachable("Value is not load or store instruction");
    return 0;
}

vector<const SCEV*> getSubscrit(Instruction* I, ScalarEvolution *SE)
{
    vector<const SCEV*> rt(0);
    if(!IsLoadOrStore(I)) return rt;
    
    Value* Ptr = getPointerOperand(I);
    DEBUG(dbgs() << " Subscrit : " << *I << "\t" << *(SE->getSCEV(Ptr)) << "\n");
    GEPOperator *GEP = dyn_cast<GEPOperator>(Ptr);
    if(GEP == NULL)
    {
        rt.push_back(SE->getSCEV((Ptr)));
        return rt;
    }
    else{
        for(GEPOperator::const_op_iterator idx = GEP->idx_begin(), idxE = GEP->idx_end(); idx != idxE; idx++)
        {
            rt.push_back(SE->getSCEV(*idx));
        }
    }
    return rt;
}

/*********************************************/
//TEST Method
bool ToPass(Function *F)
{
    StringRef s = F->getName();
    if(s.find("push") != StringRef::npos)
        return false;
    if(s.find("pop") != StringRef::npos)
        return false;
    if(s.find("peek") != StringRef::npos)
        return false;
    return true;
}