#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <iostream>

#include "SGO.h"
#include "SLF.h"

using namespace llvm;
using namespace std;

SGO::SGO() : ModulePass(ID)
{}

bool SGO::runOnModule(Module &M)
{
    for(Module::iterator itr = M.begin(); itr != M.end(); itr++)
        cerr << (itr->getName()).str() << endl;
    SLF x;
	return false;
}

char SGO::SGO::ID = 0;
static RegisterPass<SGO::SGO> X("SGO", "StreamIt Global Optimizer", false, false);
