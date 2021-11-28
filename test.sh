cd ./Build
cmake ../Transforms
make
cd ../Test

runTest(){
    echo -e "\n\033[32m[*] Test $1 \033[0m"
    opt -load ../Build/LLVMObfuscator.so -$2 -split_num 3 -S IR/orig.ll -o IR/$2.ll
    if test $2 = "rcf"
    then
        llc -filetype=obj -mattr=+rdrnd IR/rcf.ll -o Bin/rcf.o
        clang Bin/rcf.o -o Bin/rcf
    else
        clang IR/$2.ll -o Bin/$2
    fi
    ./Bin/$2 flag{s1mpl3_11vm_d3m0}
}

# 编译未混淆的 LLVM IR 文件
clang -S -emit-llvm TestProgram.cpp -o IR/orig.ll

# 控制流平坦化
runTest "Flattening" "fla"

# 虚假控制流
runTest "BogusControlFlow" "bcf"

# 指令替换
runTest "Substitution" "sub"

# 随机控制流
runTest "RandomControlFlow" "rcf"

# 常量替换
runTest "ConstantSubstitution" "csub"