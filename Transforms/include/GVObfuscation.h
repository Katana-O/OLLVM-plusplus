#include "llvm/IR/Function.h"
#include "llvm/Pass.h"

namespace llvm{
    ModulePass* createGVObfuscationPass();
    ModulePass* createGVObfuscationPass(bool onlystr);
}