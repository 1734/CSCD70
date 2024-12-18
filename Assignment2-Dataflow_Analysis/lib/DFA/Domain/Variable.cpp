#include <DFA/Domain/Variable.h>

using namespace llvm;
using dfa::Variable;

raw_ostream &operator<<(raw_ostream &Outs, const Variable &Var) {
  CHECK(Var.Var != nullptr);
  Var.Var->printAsOperand(Outs);
  return Outs;
}

void Variable::Initializer::visitInstruction(Instruction &I) {

  /// @todo(CSCD70) Please complete this method.
  if (I.getType()->isVoidTy()) {
    return;
  }
  DomainIdMap[&I] = DomainVector.size();
  DomainVector.push_back(&I);
  if (isa<ICmpInst>(&I)) {
    InternalRuntimeChecker(isa<ConstantInt>(I.getOperand(1)));
  }
}

void Variable::Initializer::visitFunction(llvm::Function &F) {
  for (const auto &Arg : F.args()) {
    DomainIdMap[&Arg] = DomainVector.size();
    DomainVector.push_back(&Arg);
  }
}
