# OLLVM++
Obfuscator refactored and extended from OLLVM.
## Environment
- Ubuntu 18.04.5 LTS
- LLVM 12.0.1
- Clang 12.0.1
- CMake 3.21.1
## Usage
### Compile Obfuscation Passes
```
if [ ! -d "Build" ]; then
    mkdir Build
fi
cd ./Build
cmake ../Transforms
make
```
### Generate LLVM IR
```
clang -S -emit-llvm TestProgram.cpp -o IR/orig.ll
```
### Control Flow Flattening
```
opt -load ../Build/LLVMObfuscator.so -fla -split_num 3 -S IR/orig.ll -o IR/fla.ll
clang IR/fla.ll -o Bin/fla
```
### Bogus Control Flow
```
opt -load ../Build/LLVMObfuscator.so -bcf -bcf_loop 1 -split_num 3 -S IR/orig.ll -o IR/bcf.ll
clang IR/bcf.ll -o Bin/bcf
```
### Instruction Substitution
```
opt -load ../Build/LLVMObfuscator.so -sub -sub_loop 2 -S IR/orig.ll -o IR/sub.ll
clang IR/sub.ll -o Bin/sub
```
### Random Control Flow
```
opt -load ../Build/LLVMObfuscator.so -rcf -S IR/orig.ll -o IR/rcf.ll
llc -filetype=obj -mattr=+rdrnd IR/rcf.ll -o Bin/rcf.o
clang Bin/rcf.o -o Bin/rcf
```
### Constant Substitution
```
opt -load ../Build/LLVMObfuscator.so -csub -csub_loop 2 -S IR/orig.ll -o IR/csub.ll
clang IR/csub.ll -o Bin/csub
```
## Features
### Control Flow Flattening
<img src = "image/Control Flow Flatten.png">

### Bogus Control Flow
<img src = "image/Bogus Control Flow.png">

### Instruction Substitution
<img src = "image/Instruction Substitution.png">

### Random Control Flow
<img src = "image/Random Control Flow.png">

### Constant Substitution
<img src = "image/Constant Substitution.png">
