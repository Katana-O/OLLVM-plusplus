echo -e "\n\033[32m[*] Compiling passes... \033[0m"
mkdir -p Build
cd Build
cmake ../Transforms
make
cd ../Test
rm -rf Bin IR
mkdir -p Bin IR
echo -e "\n\033[32m[*] Compilation completed \033[0m"

runTest(){
    echo -e "\n\033[32m[*] Test $1 \033[0m"
    opt -load ../Build/LLVMObfuscator.so -$2 -split_num 3 -S IR/orig.ll -o IR/$2.ll
    opt -load ../Build/LLVMObfuscator.so -$2 -split_num 3 -S IR/AES.ll -o IR/AES_$2.ll
    llc -filetype=obj -mattr=+rdrnd IR/$2.ll -o Bin/$2.o
    llc -filetype=obj -mattr=+rdrnd IR/AES_$2.ll -o Bin/AES_$2.o
    clang++ Bin/$2.o Bin/AES_$2.o -o Bin/$2
    ./Bin/$2 flag{s1mpl3_11vm_d3m0}
}

# 编译未混淆的 LLVM IR 文件
clang++ -S -emit-llvm TestProgram-1.cpp -o IR/orig.ll
clang++ -S -emit-llvm AES.cpp -o IR/AES.ll
clang++ IR/orig.ll IR/AES.ll -o Bin/orig

# # 控制流平坦化
# runTest "Flattening" "fla"

# # 虚假控制流
# runTest "BogusControlFlow" "bcf"

# # 指令替换
# runTest "Substitution" "sub"

# # 随机控制流
# runTest "RandomControlFlow" "rcf"

# # 常量替换
# runTest "ConstantSubstitution" "csb"

# 字符串混淆
runTest "StringObfuscation" "sob"

# 全局变量混淆
runTest "GVObfuscation" "gvo"