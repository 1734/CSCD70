#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/PassPlugin.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;

namespace {

class FunctionInfoPass final : public PassInfoMixin<FunctionInfoPass> {
public:
  PreservedAnalyses run([[maybe_unused]] Module &M, ModuleAnalysisManager &) {
    outs() << "CSCD70 Function Information Pass"
           << "\n";

    /// @todo(CSCD70) Please complete this method.
    for (const auto &fun : M.getFunctionList()) {
      outs() << "Function Name: " << fun.getName() << "\n";
      outs() << "Number of Arguments: " << fun.getFunction().arg_size()
             << (fun.isVarArg() ? "*" : "") << "\n";
      int callsNum = 0;
      for (auto user : fun.users()) {
        if (auto ci = dyn_cast<CallInst>(user)) {
          if (ci->getCalledFunction() == &fun) {
            ++callsNum;
          }
        }
      }
      outs() << "Number of Calls: " << callsNum << "\n";
      outs() << "Number OF BBs: " << fun.size() << "\n";
      int instrNum = 0;
      for (auto it = fun.begin(); it != fun.end(); ++it) {
        instrNum += it->size();
      }
      outs() << "Number of Instructions: " << instrNum << "\n";
    }
    return PreservedAnalyses::all();
  }
}; // class FunctionInfoPass

} // anonymous namespace

extern "C" PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {
      .APIVersion = LLVM_PLUGIN_API_VERSION,
      .PluginName = "FunctionInfo",
      .PluginVersion = LLVM_VERSION_STRING,
      .RegisterPassBuilderCallbacks =
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) -> bool {
                  if (Name == "function-info") {
                    MPM.addPass(FunctionInfoPass());
                    return true;
                  }
                  return false;
                });
          } // RegisterPassBuilderCallbacks
  };        // struct PassPluginLibraryInfo
}
