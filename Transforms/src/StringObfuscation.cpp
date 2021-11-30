#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Module.h"

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
    for(GlobalVariable &GV : M.getGlobalList()){
        // errs() << GV << "\n";
        // @.str.4 = private unnamed_addr constant [18 x i8] c"Sorry try again.\0A\00", align 1
        // errs() << *GV.getInitializer() << "\n";
        // [18 x i8] c"Sorry try again.\0A\00"
        
    }
}

char StringObfuscation::ID = 0;
static RegisterPass<StringObfuscation> X("sob", "todo.");