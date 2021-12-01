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
#include "GVObfuscation.h"

using namespace std;
using namespace llvm;

namespace{
    struct EncryptedGV{
        GlobalVariable *GV;
        uint64_t key;
        uint32_t len;
    };

    class GVObfuscation : public ModulePass{
        public:
            static char ID;
            bool onlystr;

            GVObfuscation() : ModulePass(ID){
                GVObfuscation::onlystr = false;
            }

            GVObfuscation(bool onlystr) : ModulePass(ID){
                GVObfuscation::onlystr = onlystr;
            }

            bool runOnModule(Module &M) override;

            void insertModifyArrayFunctions(Module &M, EncryptedGV encGV);

            void insertModifyIntegerFunctions(Module &M, EncryptedGV encGV);

    };
}

bool GVObfuscation::runOnModule(Module &M){
    INIT_CONTEXT(M);
    vector<GlobalVariable*> GVs;
    for(GlobalVariable &GV : M.getGlobalList()){
        GVs.push_back(&GV);
    }
    for(GlobalVariable *GV : GVs){
        
        if(GV->hasInitializer() && (GV->getName().contains(".str") || !onlystr )
            && !GV->getSection().equals("llvm.metadata")){
            Constant *initializer = GV->getInitializer();
            ConstantInt *idata = dyn_cast<ConstantInt>(initializer);
            ConstantDataSequential *cdata = dyn_cast<ConstantDataSequential>(initializer);
            if(cdata && GV->getValueType()->isArrayTy()){
                char *data = const_cast<char*>(cdata->getRawDataValues().data());
                uint32_t size = cdata->getElementByteSize();
                if(size > 8){
                    errs() << "Strange element byte size...\n";
                    continue;
                }
                uint32_t len = cdata->getNumElements();
                uint64_t key = cryptoutils->get_uint64_t();
                for(int i = 0;i < len * size;i ++){
                    data[i] ^= ((char*)&key)[i % size];
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
                insertModifyArrayFunctions(M, {dynGV, key, len});
            }else if(idata && GV->getValueType()->isIntegerTy()){
                uint64_t key = cryptoutils->get_uint64_t();
                ConstantInt *enc = CONST(idata->getType(), key ^ idata->getZExtValue());
                // Duplicate global variable
                GlobalVariable *dynGV = new GlobalVariable(M,
                    GV->getType()->getElementType(),
                    false, GV->getLinkage(),
                    enc, GV->getName(),
                    nullptr,
                    GV->getThreadLocalMode(),
                    GV->getType()->getAddressSpace());
                GV->replaceAllUsesWith(dynGV);
                GV->eraseFromParent();
                insertModifyIntegerFunctions(M, {dynGV, key, 1LL});
            }
        }
    }
}

void GVObfuscation::insertModifyIntegerFunctions(Module &M, EncryptedGV encGV){
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

    BasicBlock *entry = BasicBlock::Create(*CONTEXT, "entry", func);
    IRBuilder<> builder(*CONTEXT);
    builder.SetInsertPoint(entry);
    LoadInst *val = builder.CreateLoad(encGV.GV);
    Value *xorVal = builder.CreateXor(val, CONST(encGV.GV->getValueType(), encGV.key));
    builder.CreateStore(xorVal, encGV.GV);
    builder.CreateRetVoid();
    appendToGlobalCtors(M, func, 0);
}

void GVObfuscation::insertModifyArrayFunctions(Module &M, EncryptedGV encGV){
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
    ArrayType *arrTy = cast<ArrayType>(encGV.GV->getValueType());
    Type *eleTy = arrTy->getElementType();
    Value *encEle = builder.CreateXor(builder.CreateLoad(ele), CONST(eleTy, encGV.key));
    builder.CreateStore(encEle, ele);
    builder.CreateBr(forInc);
    builder.SetInsertPoint(forInc);
    builder.CreateStore(builder.CreateAdd(index, CONST_I32(1)), indexPtr);
    builder.CreateBr(forCond);

    builder.SetInsertPoint(forEnd);
    builder.CreateRetVoid();
    appendToGlobalCtors(M, func, 0);
}

ModulePass* llvm::createGVObfuscationPass(){
    return new GVObfuscation(false);
}

ModulePass* llvm::createGVObfuscationPass(bool onlystr){
    return new GVObfuscation(onlystr);
}

char GVObfuscation::ID = 0;
static RegisterPass<GVObfuscation> X("gvo", "Obfuscate all GVs and insert functions that decrypt GVs at runtime.");