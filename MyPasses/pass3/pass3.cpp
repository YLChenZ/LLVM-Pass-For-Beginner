#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"


using namespace llvm;

namespace {

bool runHelper(Function& F)
{
    bool modified = false;
    SmallVector<BinaryOperator*, 16> BinOps;

    for (Instruction &I : instructions(&F))
    {
        if (auto *binOp = dyn_cast<BinaryOperator>(&I))
        {
            BinOps.push_back(binOp);
        }
    }

    for (auto *binOp : BinOps)
    {
        IRBuilder<> builder(binOp);

        // Make a multiply with the same operands as `binOp`.
        Value* lhs = binOp->getOperand(0);
        Value* rhs = binOp->getOperand(1);
        Value* mul = builder.CreateMul(lhs, rhs);

        if (auto *binOpInst = dyn_cast<Instruction>(binOp))
        {
            binOpInst->replaceAllUsesWith(mul);
            binOpInst->eraseFromParent();
            modified |= true;
        }
    }
    return modified;
}

struct MyPass3 : public PassInfoMixin<MyPass3> {
    PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
        if (!runHelper(F))
        {
            return PreservedAnalyses::none();
        }
        return PreservedAnalyses::all();
    };
};

}

//clang
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {
    LLVM_PLUGIN_API_VERSION,
    "My pass3",
    "v0.1",
    [](PassBuilder &PB) {
      PB.registerPipelineParsingCallback(
        [](StringRef Name, FunctionPassManager &FPM,
           ArrayRef<PassBuilder::PipelineElement>) {
          if (Name == "mypass3") {
            FPM.addPass(MyPass3()); // MyPass3 应是函数级 pass
            return true;
          }
          return false;
        }
      );
    }
  };
}

//opt
// extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
// llvmGetPassPluginInfo() {
//   return {
//     LLVM_PLUGIN_API_VERSION,          // APIVersion
//     "My pass3",                       // PluginName
//     "v0.1",                           // PluginVersion
//     // RegisterPassBuilderCallbacks
//     [](llvm::PassBuilder &PB) {
//       PB.registerPipelineParsingCallback(
//         [](llvm::StringRef Name,
//            llvm::FunctionPassManager &FPM,
//            llvm::ArrayRef<llvm::PassBuilder::PipelineElement> Elements) {
//           if (Name == "mypass3") {
//             FPM.addPass(MyPass3()); // MyPass3 必须是函数级 pass
//             return true;
//           }
//           return false;
//         }
//       );
//     }
//   };
// }
