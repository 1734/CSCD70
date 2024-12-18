#include <DFA/Domain/VariableForInterval.h>

using namespace llvm;
using dfa::VariableForInterval;

raw_ostream &operator<<(raw_ostream &Outs, const VariableForInterval &Var) {
  CHECK(Var.Var != nullptr);
  Var.Var->printAsOperand(Outs);
  return Outs;
}

void VariableForInterval::Initializer::visitFunction(llvm::Function &F) {
  for (const auto &Arg : F.args()) {
    DomainIdMap[&Arg] = DomainVector.size();
    DomainVector.push_back(&Arg);
  }
}

void VariableForInterval::Initializer::visitInstruction(Instruction &I) {

  /// @todo(CSCD70) Please complete this method.
  if (I.getType()->isVoidTy()) {
    return;
  }
  DomainIdMap[&I] = DomainVector.size();
  DomainVector.push_back(&I);
  if (isa<ICmpInst>(&I)) {
    InternalRuntimeChecker(isa<ConstantInt>(I.getOperand(1)));
    ConditionToBindVariableMap[dyn_cast<ICmpInst>(&I)] = I.getOperand(0);
  }
}
