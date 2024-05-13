#include "LocalOpts.h"
#include <llvm/IR/Constants.h>

using namespace llvm;

PreservedAnalyses AlgebraicIdentityPass::run([[maybe_unused]] Function &F,
                                             FunctionAnalysisManager &) {

  /// @todo(CSCD70) Please complete this method.
  for (auto &bb : F) {
    for (auto &instr : bb) {
      if (instr.getNumOperands() != 2) {
        continue;
      }
      auto op1 = instr.getOperand(0);
      auto op2 = instr.getOperand(1);
      int64_t constVal1, constVal2;
      if (isa<ConstantInt>(op1)) {
        constVal1 = dyn_cast<ConstantInt>(op1)->getSExtValue();
      }
      if (isa<ConstantInt>(op2)) {
        constVal2 = dyn_cast<ConstantInt>(op2)->getSExtValue();
      }
      switch (instr.getOpcode()) {
      case Instruction::Add:
        if (isa<ConstantInt>(op1) && constVal1 == 0) {
          instr.replaceAllUsesWith(op2);
        } else if (isa<ConstantInt>(op2) && constVal2 == 0) {
          instr.replaceAllUsesWith(op1);
        }
        break;
      case Instruction::Sub:
        if (isa<ConstantInt>(op1) && constVal1 == 0) {
          instr.replaceAllUsesWith(op2);
        } else if (isa<ConstantInt>(op2) && constVal2 == 0) {
          instr.replaceAllUsesWith(op1);
        }
        break;
      case Instruction::Mul:
        if (isa<ConstantInt>(op1) && constVal1 == 1) {
          instr.replaceAllUsesWith(op2);
        } else if (isa<ConstantInt>(op2) && constVal2 == 1) {
          instr.replaceAllUsesWith(op1);
        }
        break;
      case Instruction::SDiv:
        if (isa<ConstantInt>(op2) && constVal1 == 2) {
          instr.replaceAllUsesWith(op1);
        } else if (op1 == op2) {
          instr.replaceAllUsesWith(ConstantInt::getSigned(instr.getType(), 1));
        }
        break;
      }
    }
  }
  return PreservedAnalyses::none();
}
