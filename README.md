# OLLVM++
Obfuscator refactored and extended from OLLVM.
## Environment
- Ubuntu 18.04.5 LTS
- LLVM 12.0.1
- Clang 12.0.1
- CMake 3.21.1
## Usage
### Control Flow Flattening
```
opt -lowerswitch -S IR/TestProgram_orig.ll -o IR/TestProgram_lowerswitch.ll
opt -load ../Build/LLVMObfuscator.so -fla -split_num 3 -S IR/TestProgram_lowerswitch.ll -o IR/TestProgram_fla.ll
clang IR/TestProgram_fla.ll -o Bin/TestProgram_fla
```
### Bogus Control Flow
```
opt -load ../Build/LLVMObfuscator.so -bcf -bcf_loop 1 -split_num 3 -S IR/TestProgram_orig.ll -o IR/TestProgram_bcf.ll
clang IR/TestProgram_bcf.ll -o Bin/TestProgram_bcf
```
### Instruction Substitution
```
opt -load ../Build/LLVMObfuscator.so -sub -sub_loop 2 -S IR/TestProgram_orig.ll -o IR/TestProgram_sub.ll
clang IR/TestProgram_sub.ll -o Bin/TestProgram_sub
```
### Random Control Flow
```
opt -load ../Build/LLVMObfuscator.so -rcf -S IR/TestProgram_orig.ll -o IR/TestProgram_rcf.ll
llc -filetype=obj -mattr=+rdrnd IR/TestProgram_rcf.ll -o Bin/TestProgram_rcf.o
clang Bin/TestProgram_rcf.o -o Bin/TestProgram_rcf
```
### Constant Substitution
```
opt -load ../Build/LLVMObfuscator.so -csub -csub_loop 2 -S IR/TestProgram_orig.ll -o IR/TestProgram_csub.ll
clang IR/TestProgram_csub.ll -o Bin/TestProgram_csub
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
