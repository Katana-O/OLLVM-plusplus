#include "llvm/Pass.h"
#include "StringObfuscation.h"
#include "GVObfuscation.h"

using namespace llvm;

namespace{

    class StringObfuscation : public ModulePass{
        public:
            static char ID;

            StringObfuscation() : ModulePass(ID){

            }

            bool runOnModule(Module &M) override;

    };
}

bool StringObfuscation::runOnModule(Module &M){
    ModulePass *pass = createGVObfuscationPass(true);
    pass->runOnModule(M);
}

ModulePass* llvm::createStringObfuscationPass(){
    return new StringObfuscation();
}

char StringObfuscation::ID = 0;
static RegisterPass<StringObfuscation> X("sob", "Obfuscate all strings and insert functions that decrypt strings at runtime.");