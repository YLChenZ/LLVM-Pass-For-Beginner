# LLVM Pass入门

参考文档：

https://www.cs.cornell.edu/~asampson/blog/llvm.html

## 预备LLVM

### **<!--编译任何项目之前，先把交换分区给弄到10个G（或以上），不然容易爆内存-->**
```bash
#查看交换分区大小
grep SwapTotal /proc/meminfo
#1、关闭交换空间
sudo swapoff -a
#2、扩充交换空间大小，10G 
sudo fallocate -l 10G /swapfile
#3、设置权限
sudo chmod 600 /swapfile
#4、指定交换空间对应的设备文件
sudo mkswap /swapfile
 #5、启用交换分区
sudo swapon /swapfile
```



### 编译LLVM

### 安装依赖：

```bash
sudo apt-get update
sudo apt-get install -y \
    cmake ninja-build git build-essential \
    python3 python3-dev zlib1g-dev libncurses5-dev \
    libxml2-dev libedit-dev swig
	
```

### 安装LLVM

```bash
git clone https://github.com/llvm/llvm-project.git
```

```bash
cd llvm-project

cmake -S llvm -B build -G Ninja -DLLVM_ENABLE_PROJECTS=clang -DCMAKE_BUILD_TYPE=Release

cmake --build build
```

测试安装是否成功

```bash
cmake --build build --target check-all
```

Install一下（在build目录下）

```bash
ninja install
```

## LLVM Pass之旅开启

假设我们有一个C语言编写的程序 **example.c** 如下：

```c
#include <stdio.h>
int main(int argc, const char** argv) {
    int num;
    scanf("%i", &num);
    printf("%i\n", num + 2);
    return 0;
}
```

代码逻辑很简单，就是读一个**num**，然后打印**num+2**.

可以利用clang直接生成可执行文件：

```bash
clang example.c
./a.out 
10
12
```

这样就会生成 **a.out** 可执行文件；就可以执行，这里输入一个数10，然后就会打印+2（12）。

我们还可以利用clang将其生成为可读的 **LLVM IR**：

```
clang -S -emit-llvm example.c -o example.ll
```

生成的IR长这样：

```llvm ir
; ModuleID = 'example.c'
source_filename = "example.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [3 x i8] c"%i\00", align 1
@.str.1 = private unnamed_addr constant [4 x i8] c"%i\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main(i32 noundef %0, ptr noundef %1) #0 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca ptr, align 8
  %6 = alloca i32, align 4
  store i32 0, ptr %3, align 4
  store i32 %0, ptr %4, align 4
  store ptr %1, ptr %5, align 8
  %7 = call i32 (ptr, ...) @__isoc99_scanf(ptr noundef @.str, ptr noundef %6)
  %8 = load i32, ptr %6, align 4
  %9 = add nsw i32 %8, 2
  %10 = call i32 (ptr, ...) @printf(ptr noundef @.str.1, i32 noundef %9)
  ret i32 0
}

declare i32 @__isoc99_scanf(ptr noundef, ...) #1

declare i32 @printf(ptr noundef, ...) #1

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"clang version 22.0.0git (git@github.com:llvm/llvm-project.git 94c751d6b5e202455cc67a432d4b69979b132051)"}
```

**LLVM IR** 的结构：

![image-20251123233503716](C:\Users\chen\AppData\Roaming\Typora\typora-user-images\image-20251123233503716.png)

这里的**Module**包含了所有的内容，这里的**Function**有三个：**main**， **__isoc99_scanf** 和 **printf**，这里只有**Function** **main**有一个**Basic Block**，里面包含了多条指令。

### 编译Pass Plugin

```bash
cd llvm-pass-skeleton
mkdir build
cd build
cmake ..  # Generate the Makefile.
make
```



### 第一个Pass

```cpp
PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM) {
    for (auto &F : M) {
        errs() << "I saw a function called " << F.getName() << "!\n";
    }
    return PreservedAnalyses::all();
};
```

这个Pass做的事情就是打印整个Module里面所有Function的name：

```bash
clang -fpass-plugin=../build/skeleton/SkeletonPass.so example.c
I saw a function called main!
I saw a function called __isoc99_scanf!
I saw a function called printf!
```

这个和IR里面看到的3个函数名字是一致的。

### 第二个Pass

```cpp
struct SkeletonPass : public PassInfoMixin<SkeletonPass> {
    PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM) {
        for (auto& F : M) {
            errs() << "Function:\n" << F << "\n";
            for (auto& B : F) {
                errs() << "Basic block:\n" << B << "\n";
                for (auto& I : B) {
                    errs() << "Instruction: " << I << "\n";
                }
            }
        }
        return PreservedAnalyses::all();
    };
};
```

这个Pass是用来打印所有的Function，以及该Function所有的BasicBlock，以及每个BasicBlock中的Instruction。

```
Function:
; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main(i32 noundef %0, ptr noundef %1) #0 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca ptr, align 8
  %6 = alloca i32, align 4
  store i32 0, ptr %3, align 4
  store i32 %0, ptr %4, align 4
  store ptr %1, ptr %5, align 8
  %7 = call i32 (ptr, ...) @__isoc99_scanf(ptr noundef @.str, ptr noundef %6)
  %8 = load i32, ptr %6, align 4
  %9 = add nsw i32 %8, 2
  %10 = call i32 (ptr, ...) @printf(ptr noundef @.str.1, i32 noundef %9)
  ret i32 0
}

Basic block:

  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca ptr, align 8
  %6 = alloca i32, align 4
  store i32 0, ptr %3, align 4
  store i32 %0, ptr %4, align 4
  store ptr %1, ptr %5, align 8
  %7 = call i32 (ptr, ...) @__isoc99_scanf(ptr noundef @.str, ptr noundef %6)
  %8 = load i32, ptr %6, align 4
  %9 = add nsw i32 %8, 2
  %10 = call i32 (ptr, ...) @printf(ptr noundef @.str.1, i32 noundef %9)
  ret i32 0

Instruction:   %3 = alloca i32, align 4
Instruction:   %4 = alloca i32, align 4
Instruction:   %5 = alloca ptr, align 8
Instruction:   %6 = alloca i32, align 4
Instruction:   store i32 0, ptr %3, align 4
Instruction:   store i32 %0, ptr %4, align 4
Instruction:   store ptr %1, ptr %5, align 8
Instruction:   %7 = call i32 (ptr, ...) @__isoc99_scanf(ptr noundef @.str, ptr noundef %6)
Instruction:   %8 = load i32, ptr %6, align 4
Instruction:   %9 = add nsw i32 %8, 2
Instruction:   %10 = call i32 (ptr, ...) @printf(ptr noundef @.str.1, i32 noundef %9)
Instruction:   ret i32 0
Function:
declare i32 @__isoc99_scanf(ptr noundef, ...) #1

Function:
declare i32 @printf(ptr noundef, ...) #1
```

### 第三个Pass

```cpp
bool runHelper(Module &M, ModuleAnalysisManager &AM) {
    bool modified = false;
    SmallVector<BinaryOperator*, 16> BinOps;
    for (Function &F : M) {
        for (BasicBlock &B : F) {
            for (auto &I : B) {
                if (auto *binOp = dyn_cast<BinaryOperator>(&I)) {
                        BinOps.push_back(binOp);
                }
            }
        }
    }

    for (auto *binOp : BinOps) {
        IRBuilder<> builder(binOp);

        // Make a multiply with the same operands as `binOp`.
        Value* lhs = binOp->getOperand(0);
        Value* rhs = binOp->getOperand(1);
        Value* mul = builder.CreateMul(lhs, rhs);

        if (auto *binOpInst = dyn_cast<Instruction>(binOp)) {
            binOpInst->replaceAllUsesWith(mul);
            binOpInst->eraseFromParent();
            modified != true;
        }
    }
    return modified;
}

struct SkeletonPass : public PassInfoMixin<SkeletonPass> {
    PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM) {
        runHelper(M, AM);
        return PreservedAnalyses::all();
    };
};
```

```bash
clang -fpass-plugin=../build/skeleton/SkeletonPass.so example.c
./a.out 
10
20
```

之前是12，现在变成20了。

### 使用opt

上面演示的都是clang加载pass的插件，如果想要使用opt来加载pass的插件需要修改源代码：

```cpp
//clang
// extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
// llvmGetPassPluginInfo() {
//     return {
//         .APIVersion = LLVM_PLUGIN_API_VERSION,
//         .PluginName = "Skeleton pass",
//         .PluginVersion = "v0.1",
//         .RegisterPassBuilderCallbacks = [](PassBuilder &PB) {
//             PB.registerPipelineStartEPCallback(
//                 [](ModulePassManager &MPM, OptimizationLevel Level) {
//                     MPM.addPass(SkeletonPass());
//                 });
//         }
//     };
// }

// opt
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return {
        .APIVersion = LLVM_PLUGIN_API_VERSION,
        .PluginName = "Skeleton pass",
        .PluginVersion = "v0.1",
        .RegisterPassBuilderCallbacks = [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef Name, llvm::ModulePassManager &MPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) {
                    if (Name == "skeleton") {
                    	MPM.addPass(SkeletonPass()); // Module-level pass
                    	return true;
                }
                return false;
            });
        }
    };
}
```

使用opt：

```
opt -S --load-pass-plugin=../build/skeleton/SkeletonPass.so -passes=skeleton example.ll -o example_opt.ll
```

