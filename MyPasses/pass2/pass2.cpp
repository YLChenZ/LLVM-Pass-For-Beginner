#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h"

using namespace llvm;

namespace {

struct MyPass2 : public PassInfoMixin<MyPass2> {
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

}

//clang
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return {
        .APIVersion = LLVM_PLUGIN_API_VERSION,
        .PluginName = "My pass2",
        .PluginVersion = "v0.1",
        .RegisterPassBuilderCallbacks = [](PassBuilder &PB) {
            PB.registerPipelineStartEPCallback(
                [](ModulePassManager &MPM, OptimizationLevel Level) {
                    MPM.addPass(MyPass2());
                });
        }
    };
}

//opt
// extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
// llvmGetPassPluginInfo() {
//     return {
//         .APIVersion = LLVM_PLUGIN_API_VERSION,
//         .PluginName = "My pass2",
//         .PluginVersion = "v0.1",
//         .RegisterPassBuilderCallbacks = [](PassBuilder &PB) {
//             PB.registerPipelineParsingCallback(
//                 [](llvm::StringRef Name, llvm::ModulePassManager &MPM,
//                    llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) {
//                     if (Name == "mypass2") {
//                     MPM.addPass(MyPass2()); // Module-level pass
//                     return true;
//                 }
//                 return false;
//             });
//         }
//     };
// }
