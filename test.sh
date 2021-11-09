if [ ! -d "Build" ]; then
    mkdir Build
fi
cd ./Build
cmake ../Transforms
make
cd ../Test

# 未混淆的ELF文件
clang -S -emit-llvm TestProgram.cpp -o IR/orig.ll
clang IR/orig.ll -o Bin/orig
echo -e "\n\033[32m> Test case on original binary <\033[0m"
./Bin/orig flag{s1mpl3_11vm_d3m0}

# 基本块分割
opt -load ../Build/LLVMObfuscator.so -split -split_num 3 -S IR/orig.ll -o IR/split.ll
clang IR/split.ll -o Bin/split

# 控制流平坦化
opt -lowerswitch -S IR/orig.ll -o IR/lowerswitch.ll
opt -load ../Build/LLVMObfuscator.so -fla -split_num 3 -S IR/lowerswitch.ll -o IR/fla.ll
clang IR/fla.ll -o Bin/fla
echo -e "\n\033[32m> Test case on Flattening <\033[0m"
./Bin/fla flag{s1mpl3_11vm_d3m0}

# 虚假控制流
opt -load ../Build/LLVMObfuscator.so -bcf -bcf_loop 1 -split_num 3 -S IR/orig.ll -o IR/bcf.ll
clang IR/bcf.ll -o Bin/bcf
echo -e "\n\033[32m> Test case on BogusControlFlow <\033[0m"
./Bin/bcf flag{s1mpl3_11vm_d3m0}

# 指令替换
opt -load ../Build/LLVMObfuscator.so -sub -sub_loop 2 -S IR/orig.ll -o IR/sub.ll
clang IR/sub.ll -o Bin/sub
echo -e "\n\033[32m> Test case on Substitution <\033[0m"
./Bin/sub flag{s1mpl3_11vm_d3m0}

# 随机控制流
opt -load ../Build/LLVMObfuscator.so -rcf -S IR/orig.ll -o IR/rcf.ll
llc -filetype=obj -mattr=+rdrnd IR/rcf.ll -o Bin/rcf.o
clang Bin/rcf.o -o Bin/rcf
echo -e "\n\033[32m> Test case on RandomControlFlow <\033[0m"
./Bin/rcf flag{s1mpl3_11vm_d3m0}

# 常量替换
opt -load ../Build/LLVMObfuscator.so -csub -csub_loop 2 -S IR/orig.ll -o IR/csub.ll
clang IR/csub.ll -o Bin/csub
echo -e "\n\033[32m> Test case on ConstantSubstitution <\033[0m"
./Bin/csub flag{s1mpl3_11vm_d3m0}