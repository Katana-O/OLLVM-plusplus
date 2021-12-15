#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "GVObfuscation.h"
#include "ConstantSubstitution.h"
#include "RandomControlFlow.h"

using namespace llvm;

namespace {

    class FullProtect : public ModulePass{
        public:
            static char ID;
            FullProtect() : ModulePass(ID) {}

            bool runOnModule(Module &M);
    };
    
}

bool FullProtect::runOnModule(Module &M){
    ModulePass *gvoPass = createGVObfuscationPass();
    FunctionPass *csbPass = createConstantSubstitutionPass();
	FunctionPass *rcfPass = createRandomControlFlowPass();
    gvoPass->runOnModule(M);
    for(Function &F : M){
        csbPass->runOnFunction(F);
        rcfPass->runOnFunction(F);
    }
    delete gvoPass;
    delete csbPass;
    delete rcfPass;
}

char FullProtect::ID = 0;
static RegisterPass<FullProtect> X("fpt", "Full protect.");