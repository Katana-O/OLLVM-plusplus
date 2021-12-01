# OLLVM++
Obfuscator refactored and extended from OLLVM.
> 购买了看雪课程《LLVM与代码混淆技术》的小伙伴请注意，最新版本的OLLVM++与课程中的版本有所差别，要浏览课程版本一致的OLLVM++源码[请点这里](https://github.com/bluesadi/OLLVM-plusplus/tree/525680ab2ddf4f5c3744f1f6f6a3ba1581394ff4)。
## Environment
- Ubuntu 18.04.5 LTS
- LLVM 12.0.1
- Clang 12.0.1
- CMake 3.21.1
## Usage
See [test.sh](test.sh)
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
