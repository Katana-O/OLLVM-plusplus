#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

class StringObfuscation : ModulePass{
    public:
        static char ID;

        StringObfuscation() : ModulePass(ID){

        }

        bool runOnModule(Module &M) override;
};

bool StringObfuscation::runOnModule(Module &M){
    
}