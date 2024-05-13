#include "LocalOpts.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/InstrTypes.h>

using namespace llvm;

static int getShift(int64_t x) {
  if (x <= 0 || (x & (~x + 1)) != x) {
    return -1;
  }
  int i = 0;
  while (x != 1) {
    ++i;
    x = x >> 1;
  }
  return i;
}

PreservedAnalyses StrengthReductionPass::run([[maybe_unused]] Function &F,
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
      int shift1 = -1, shift2 = -1;
      if (isa<ConstantInt>(op1)) {
        constVal1 = dyn_cast<ConstantInt>(op1)->getSExtValue();
        shift1 = getShift(constVal1);
      }
      if (isa<ConstantInt>(op2)) {
        constVal2 = dyn_cast<ConstantInt>(op2)->getSExtValue();
        shift2 = getShift(constVal2);
      }
      switch (instr.getOpcode()) {
      case Instruction::Mul: {
        if (shift1 != -1) {
          auto v = ConstantInt::getSigned(instr.getType(), shift1);
          instr.replaceAllUsesWith(
              BinaryOperator::Create(Instruction::Shl, op2, v, "xxx", &instr));
        } else if (shift2 != -1) {
          auto v = ConstantInt::getSigned(instr.getType(), shift2);
          instr.replaceAllUsesWith(
              BinaryOperator::Create(Instruction::Shl, op1, v, "xxx", &instr));
        }
        break;
      }
      case Instruction::SDiv: {
        if (shift2 != -1) {
          auto v = ConstantInt::getSigned(instr.getType(), shift2);
          instr.replaceAllUsesWith(
              BinaryOperator::Create(Instruction::AShr, op1, v, "xxx", &instr));
        }
        break;
      }
      default:
        break;
      }
    }
  }
  return PreservedAnalyses::none();
}
