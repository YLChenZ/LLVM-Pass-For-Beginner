#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h"

using namespace llvm;

namespace {

struct MyPass1 : public PassInfoMixin<MyPass1> {
    PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM) {
        for (auto &F : M) {
            errs() << "I saw a function called " << F.getName() << "!\n";
        }
        return PreservedAnalyses::all();
    };
};

}

//clang
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return {
        .APIVersion = LLVM_PLUGIN_API_VERSION,
        .PluginName = "My pass1",
        .PluginVersion = "v0.1",
        .RegisterPassBuilderCallbacks = [](PassBuilder &PB) {
            PB.registerPipelineStartEPCallback(
                [](ModulePassManager &MPM, OptimizationLevel Level) {
                    MPM.addPass(MyPass1());
                });
        }
    };
}

//opt
// extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
// llvmGetPassPluginInfo() {
//     return {
//         .APIVersion = LLVM_PLUGIN_API_VERSION,
//         .PluginName = "My pass1",
//         .PluginVersion = "v0.1",
//         .RegisterPassBuilderCallbacks = [](PassBuilder &PB) {
//             PB.registerPipelineParsingCallback(
//                 [](llvm::StringRef Name, llvm::ModulePassManager &MPM,
//                    llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) {
//                     if (Name == "mypass1") {
//                     MPM.addPass(MyPass1()); // Module-level pass
//                     return true;
//                 }
//                 return false;
//             });
//         }
//     };
// }
