#include "LocalOpts.h"
#include <iostream>
#include <llvm/IR/Constants.h>
#include <llvm/Support/raw_ostream.h>
#include <unordered_map>
using namespace llvm;

class Expr {
public:
  Value *op1;
  Value *op2;
  int operand;
  Expr() : op1(0), op2(0), operand(-1) {}
  Expr(Value *_op1, Value *_op2, int _operand)
      : op1(_op1), op2(_op2), operand(_operand) {}
  bool operator==(const Expr &Other) const {
    return op1 == Other.op1 && op2 == Other.op2 && operand == Other.operand;
  }
};

struct MyHash {
  size_t operator()(const Expr &e) const {
    return (unsigned long)e.op1 + (unsigned long)e.op2 + e.operand;
  }
};

PreservedAnalyses MultiInstOptPass::run([[maybe_unused]] Function &F,
                                        FunctionAnalysisManager &) {

  /// @todo(CSCD70) Please complete this method.
  for (auto &bb : F) {
    std::unordered_map<Expr, Value *, MyHash> Expr2val;
    std::list<Instruction *> toDelete;
    for (auto &instr : bb) {
      if (instr.getNumOperands() != 2) {
        continue;
      }
      if (instr.getOpcode() != Instruction::Add &&
          instr.getOpcode() != Instruction::Sub &&
          instr.getOpcode() != Instruction::Mul &&
          instr.getOpcode() != Instruction::SDiv) {
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
      if (isa<ConstantInt>(op1) && isa<ConstantInt>(op2)) {
        switch (instr.getOpcode()) {
        case Instruction::Add:
          instr.replaceAllUsesWith(
              ConstantInt::getSigned(instr.getType(), constVal1 + constVal2));
          break;
        case Instruction::Sub:
          instr.replaceAllUsesWith(
              ConstantInt::getSigned(instr.getType(), constVal1 - constVal2));
          break;
        case Instruction::Mul:
          instr.replaceAllUsesWith(
              ConstantInt::getSigned(instr.getType(), constVal1 * constVal2));
          break;
        case Instruction::SDiv:
          instr.replaceAllUsesWith(
              ConstantInt::getSigned(instr.getType(), constVal1 / constVal2));
          break;
        }
        toDelete.push_back(&instr);
      } else {
        Expr expr1(op1, op2, instr.getOpcode());
        if (auto It = Expr2val.find(expr1); It != Expr2val.end()) {
          instr.replaceAllUsesWith(It->second);
          toDelete.push_back(&instr);
        } else {
          auto *PVal = static_cast<Value *>(&instr);
          Expr2val[expr1] = PVal;
          Expr2val[Expr(PVal, PVal, Instruction::Sub)] =
              ConstantInt::getSigned(instr.getType(), 0);
          Expr2val[Expr(PVal, PVal, Instruction::SDiv)] =
              ConstantInt::getSigned(instr.getType(), 1);
          switch (instr.getOpcode()) {
          case Instruction::Add:
            Expr2val[Expr(op2, op1, Instruction::Add)] = PVal;
            Expr2val[Expr(PVal, op2, Instruction::Sub)] = op1;
            Expr2val[Expr(PVal, op1, Instruction::Sub)] = op2;
            break;
          case Instruction::Sub:
            Expr2val[Expr(PVal, op2, Instruction::Add)] = op1;
            Expr2val[Expr(op2, PVal, Instruction::Add)] = op1;
            Expr2val[Expr(op1, PVal, Instruction::Sub)] = op2;
            break;
            // case Instruction::Mul:
            //   expr2val[Expr(op2, op1, Instruction::Mul)] = PVal;
            //   expr2val[Expr(op2, op1, Instruction::Mul)] = PVal;
            //   break;
            // case Instruction::SDiv:
            //   break;
          }
        }
      }
    }
    for (auto it : toDelete) {
      it->eraseFromParent();
    }
  }
  return PreservedAnalyses::none();
}
