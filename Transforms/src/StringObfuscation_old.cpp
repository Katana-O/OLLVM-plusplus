#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "CryptoUtils.h"
#include <vector>
#include "Utils.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
#include <sstream>
#include "StringObfuscation.h"

using namespace std;
using namespace llvm;

namespace{
    struct EncryptedGV{
        GlobalVariable *GV;
        uint8_t key;
        uint32_t len;
    };

    class StringObfuscation : public ModulePass{
        public:
            static char ID;

            StringObfuscation() : ModulePass(ID){

            }

            bool runOnModule(Module &M) override;

            void insertModifyFunctions(Module &M, EncryptedGV encGV);

    };
}

bool StringObfuscation::runOnModule(Module &M){
    INIT_CONTEXT(M);
    vector<GlobalVariable*> GVs;
    for(GlobalVariable &GV : M.getGlobalList()){
        GVs.push_back(&GV);
    }
    for(GlobalVariable *GV : GVs){
        
        if(GV->hasInitializer() && GV->getName().contains(".str") 
            && !GV->getSection().equals("llvm.metadata")){
            errs() << *GV->getInitializer();
            Constant *initializer = GV->getInitializer();
            ConstantDataSequential *cdata = dyn_cast<ConstantDataSequential>(initializer);
            if(cdata){
                char *data = const_cast<char*>(cdata->getRawDataValues().data());
                uint32_t len = cdata->getNumElements() * cdata->getElementByteSize();
                uint8_t key = cryptoutils->get_uint8_t();
                for(int i = 0;i < len;i ++){
                    data[i] ^= key;
                }
                // Duplicate global variable
                GlobalVariable *dynGV = new GlobalVariable(M,
                    GV->getType()->getElementType(),
                    false, GV->getLinkage(),
                    initializer, GV->getName(),
                    nullptr,
                    GV->getThreadLocalMode(),
                    GV->getType()->getAddressSpace());
                GV->replaceAllUsesWith(dynGV);
                GV->eraseFromParent();
                insertModifyFunctions(M, {dynGV, key, len});
            }
        }
    }
}

void StringObfuscation::insertModifyFunctions(Module &M, EncryptedGV encGV){
    vector<Type*>args;
    FunctionType* funcType = FunctionType::get(
        /*Result=*/Type::getVoidTy(M.getContext()),  // returning void
        /*Params=*/args,  // taking no args
        /*isVarArg=*/false);
    // generate random function suffix
    stringstream stream;
    string suffix;
    stream << hex << cryptoutils->get_uint64_t();
    stream >> suffix;
    Twine funcName = encGV.GV->getName() + suffix;
    FunctionCallee callee = M.getOrInsertFunction(funcName.str(), funcType);
    Function *func = cast<Function>(callee.getCallee());
    // set the calling convention of this function to C
    // func->setCallingConv(CallingConv::C);
    BasicBlock *entry = BasicBlock::Create(*CONTEXT, "entry", func);
    BasicBlock *forCond = BasicBlock::Create(*CONTEXT, "for.cond", func);
    BasicBlock *forBody = BasicBlock::Create(*CONTEXT, "for.body", func);
    BasicBlock *forInc = BasicBlock::Create(*CONTEXT, "for.inc", func);
    BasicBlock *forEnd = BasicBlock::Create(*CONTEXT, "for.inc", func);

    IRBuilder<> builder(*CONTEXT);
    builder.SetInsertPoint(entry);
    AllocaInst *indexPtr = builder.CreateAlloca(TYPE_I32, CONST_I32(1), "i");
    builder.CreateStore(CONST_I32(0), indexPtr);
    builder.CreateBr(forCond);
    builder.SetInsertPoint(forCond);
    LoadInst *index = builder.CreateLoad(TYPE_I32, indexPtr);
    ICmpInst *cond = cast<ICmpInst>(builder.CreateICmpSLT(index, CONST_I32(encGV.len)));
    builder.CreateCondBr(cond, forBody, forEnd);

    builder.SetInsertPoint(forBody);
    Value *indexList[2] = {CONST_I32(0), index};
    Value *ele = builder.CreateGEP(encGV.GV, ArrayRef<Value*>(indexList, 2));
    Value *encEle = builder.CreateXor(builder.CreateLoad(ele), CONST_I8(encGV.key));
    builder.CreateStore(encEle, ele);
    builder.CreateBr(forInc);

    builder.SetInsertPoint(forInc);
    builder.CreateStore(builder.CreateAdd(index, CONST_I32(1)), indexPtr);
    builder.CreateBr(forCond);

    builder.SetInsertPoint(forEnd);
    builder.CreateRetVoid();
    appendToGlobalCtors(M, func, 0);
}

ModulePass* llvm::createStringObfuscationPass(){
    return new StringObfuscation();
}

char StringObfuscation::ID = 0;
static RegisterPass<StringObfuscation> X("sob", "Obfuscate all strings and insert functions that decrypt strings at runtime.");