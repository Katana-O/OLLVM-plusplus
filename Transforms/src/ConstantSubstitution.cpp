#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/CommandLine.h"
#include <vector>
#include <cstdlib>
#include <cstdint>
#include <ctime>
#include "Utils.h"
using namespace llvm;
using std::vector;

#define NUMBER_CONST_SUBST 2

// 混淆次数，混淆次数越多混淆结果越复杂
static cl::opt<int> loops("csb_loops", cl::init(1), cl::desc("Obfuscate each function ${csb_loops} time(s)."));

namespace{

    class ConstantSubstitution : public FunctionPass {
        public:
            static char ID;

            ConstantSubstitution() : FunctionPass(ID) {
                srand(time(NULL));
            }

            bool runOnFunction(Function &F);

            // 对单个指令 BI 进行替换
            void substitute(Instruction *I);

            // 线性替换：val -> ax + by + c
            // 其中 val 为原常量 a, b 为随机常量 x, y 为随机全局变量 c = val - (ax + by)
            void linearSubstitute(Instruction *I, int i);

            // 按位运算替换：val -> (x << left | y >> right) ^ c
            // 其中 val 为原常量x, y 为随机全局变量 c = val ^ (x << left | y >> right)
            void bitwiseSubstitute(Instruction *I, int i);
    };
}

bool ConstantSubstitution::runOnFunction(Function &F){
    INIT_CONTEXT(F);
    for(int i = 0;i < loops;i ++){
        for(BasicBlock &BB : F){
            vector<Instruction*> origInst;
            for(Instruction &I : BB){
                origInst.push_back(&I);
            }
            for(Instruction *I : origInst){
                if(isa<StoreInst>(I) || isa<CmpInst>(I) || isa<BinaryOperator>(I)){
                    substitute(I);
                }
            }
        }
    }
}

void ConstantSubstitution::linearSubstitute(Instruction *I, int i){
    Module &M = *I->getModule();
    ConstantInt *val = cast<ConstantInt>(I->getOperand(i));
    IntegerType *type = val->getType();
    // 随机生成 x, y, a, b
    uint64_t randX = rand(), randY = rand();
    uint64_t randA = rand(), randB = rand();
    // 计算 c = val - (ax + by)
    APInt c = val->getValue() - (randA * randX + randB * randY);
    ConstantInt *constX = CONST(type, randX);
    ConstantInt *constY = CONST(type, randY);
    ConstantInt *constA = CONST(type, randA);
    ConstantInt *constB = CONST(type, randB);
    ConstantInt *constC = (ConstantInt*)CONST(type, c);
    // 创建全局变量 x, y
    GlobalVariable *x = new GlobalVariable(M, type, false, GlobalValue::PrivateLinkage, constX, "x");
    GlobalVariable *y = new GlobalVariable(M, type, false, GlobalValue::PrivateLinkage, constY, "y");
    LoadInst *opX = new LoadInst(type, x, "", I);
    LoadInst *opY = new LoadInst(type, y, "", I);
    // 构造 op = ax + by + c 表达式
    BinaryOperator *op1 = BinaryOperator::CreateMul(opX, constA, "", I);
    BinaryOperator *op2 = BinaryOperator::CreateMul(opY, constB, "", I);
    BinaryOperator *op3 = BinaryOperator::CreateAdd(op1, op2, "", I);
    BinaryOperator *op4 = BinaryOperator::CreateAdd(op3, constC, "", I);
    // 用表达式 ax + by + c 替换原常量操作数
    I->setOperand(i, op4);
}

void ConstantSubstitution::bitwiseSubstitute(Instruction *I, int i){
    Module &M = *I->getModule();
    ConstantInt *val = cast<ConstantInt>(I->getOperand(i));
    IntegerType *type = val->getType();
    uint32_t width = type->getIntegerBitWidth();
    // 不对位数小于8的整数进行替代
    if(width < 8){
        return;
    }
    uint32_t left = rand() % (width - 1) + 1;
    uint32_t right = width - left;
    // 随机生成 x, y
    uint64_t randX = rand(), randY = rand();
    // 计算 c = val ^ (x << left | y >> right)
    APInt c = val->getValue() ^ (randX << left | randY >> right);
    ConstantInt *constX = CONST(type, randX);
    ConstantInt *constY = CONST(type, randY);
    ConstantInt *constC = (ConstantInt*)CONST(type, c);
    // 创建全局变量 x, y
    GlobalVariable *x = new GlobalVariable(M, type, false, GlobalValue::PrivateLinkage, constX, "x");
    GlobalVariable *y = new GlobalVariable(M, type, false, GlobalValue::PrivateLinkage, constY, "y");
    LoadInst *opX = new LoadInst(type, x, "", I);
    LoadInst *opY = new LoadInst(type, y, "", I);
    // 构造 op = (x << left | y >> right) ^ c 表达式
    BinaryOperator *op1 = BinaryOperator::CreateShl(opX, CONST(type, left), "", I);
    BinaryOperator *op2 = BinaryOperator::CreateLShr(opY, CONST(type, right), "", I);
    BinaryOperator *op3 = BinaryOperator::CreateOr(op1, op2, "", I);
    BinaryOperator *op4 = BinaryOperator::CreateXor(op3, constC, "", I);
    // 用表达式 (x << left | y >> right) ^ c 替换原常量操作数
    I->setOperand(i, op4);
}

void ConstantSubstitution::substitute(Instruction *I){
    int operandNum = I->getNumOperands();
    for(int i = 0;i < operandNum;i ++){
        if(isa<ConstantInt>(I->getOperand(i))){
            int choice = rand() % NUMBER_CONST_SUBST;
            switch (choice) {
                case 0:
                    linearSubstitute(I, i);
                    break;
                case 1:
                    bitwiseSubstitute(I, i);
                    break;
            }
        }
    }
}

char ConstantSubstitution::ID = 0;
static RegisterPass<ConstantSubstitution> X("csb", "Replace a constant value with equivalent instructions.");