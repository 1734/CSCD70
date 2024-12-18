#include "DFA.h"

using namespace llvm;

AnalysisKey IntegerRange::Key;

namespace {
dfa::UnconditionalInterval
transferUncondInterval(const Instruction &Inst,
                       const dfa::UnconditionalInterval *Itv1,
                       const dfa::UnconditionalInterval *Itv2) {
  if (Itv1->Low > Itv1->High) {
    return *Itv1;
  }
  if (Itv2->Low > Itv2->High) {
    return *Itv2;
  }
  switch (Inst.getOpcode()) {
  case Instruction::Add: {
    return dfa::UnconditionalInterval(Itv1->Low + Itv2->Low,
                                      Itv1->High + Itv2->High);
  }
  case Instruction::Sub: {
    return dfa::UnconditionalInterval(Itv1->Low - Itv2->High,
                                      Itv1->High - Itv2->Low);
  }
  case Instruction::Mul: {
    std::vector<int> Candidats;
    Candidats.push_back(Itv1->Low * Itv2->Low);
    Candidats.push_back(Itv1->Low * Itv2->High);
    Candidats.push_back(Itv1->High * Itv2->Low);
    Candidats.push_back(Itv1->High * Itv2->High);
    return dfa::UnconditionalInterval(
        *(std::min_element(Candidats.begin(), Candidats.end())),
        *(std::max_element(Candidats.begin(), Candidats.end())));
  }
  case Instruction::ICmp: {
    const ICmpInst *AnICmpInstPtr = dyn_cast<ICmpInst>(&Inst);
    InternalRuntimeChecker(bool(AnICmpInstPtr));
    if (AnICmpInstPtr->isGT(AnICmpInstPtr->getPredicate())) {
      if (Itv1->Low > Itv2->High) {
        return dfa::UnconditionalInterval(1, 1);
      }
      if (Itv1->High <= Itv2->Low) {
        return dfa::UnconditionalInterval(0, 0);
      }
      return dfa::UnconditionalInterval(0, 1);
    }
    if (AnICmpInstPtr->isLT(AnICmpInstPtr->getPredicate())) {
      if (Itv1->High < Itv2->Low) {
        return dfa::UnconditionalInterval(1, 1);
      }
      if (Itv1->Low >= Itv2->High) {
        return dfa::UnconditionalInterval(0, 0);
      }
      return dfa::UnconditionalInterval(0, 1);
    }
  }
  }
  InternalRuntimeChecker(0);
}

void transferIntervalWrapper(const Instruction &Inst,
                             const dfa::IntervalWrapper &Itv1,
                             const dfa::IntervalWrapper &Itv2,
                             dfa::IntervalWrapper &Result) {
  const dfa::UnconditionalInterval *UItv1 =
      Itv1.forceGetUnconditionalInterval();
  const dfa::UnconditionalInterval *UItv2 =
      Itv2.forceGetUnconditionalInterval();
  dfa::UnconditionalInterval UItvResult0 =
      transferUncondInterval(Inst, UItv1, UItv2);
  dfa::UnconditionalInterval *UItvResult = Result.tryGetUnconditionalInterval();
  if (UItvResult) {
    *UItvResult = UItvResult0;
  } else {
    dfa::ConditionalInterval *CItvResult = Result.tryGetConditionalInterval();
    InternalRuntimeChecker(bool(CItvResult));
    for (auto &[k, _] : CItvResult->ICmpInstToIntervalMap) {
      CItvResult->ICmpInstToIntervalMap[k] = UItvResult0;
    }
  }
  return;
}

} // namespace

bool IntegerRange::transferFunc(const Instruction &Inst, const DomainVal_t &IDV,
                                DomainVal_t &ODV) {

  /// @todo(CSCD70) Please complete this method.
  DomainVal_t OldODV = ODV;
  ODV = IDV;
  if (Inst.getOpcode() != Instruction::Add &&
      Inst.getOpcode() != Instruction::Sub &&
      Inst.getOpcode() != Instruction::Mul &&
      Inst.getOpcode() != Instruction::ICmp &&
      Inst.getOpcode() != Instruction::PHI) {
    return false;
  }
  InternalRuntimeChecker(DomainIdMap.find(&Inst) != DomainIdMap.end());
  auto Index = DomainIdMap[&Inst];
  if (Inst.getOpcode() != Instruction::PHI) {
    auto *Op1 = Inst.getOperand(0);
    auto *Op2 = Inst.getOperand(1);
    const dfa::IntervalWrapper *Range1, *Range2;
    if (isa<ConstantInt>(Op1)) {
      int ConstantIntValue = (int)dyn_cast<ConstantInt>(Op1)->getSExtValue();
      Range1 = new dfa::IntervalWrapper(
          dfa::UnconditionalInterval(ConstantIntValue, ConstantIntValue));
    } else {
      InternalRuntimeChecker(DomainIdMap.find(Op1) != DomainIdMap.end());
      Range1 = &IDV[DomainIdMap[Op1]];
    }
    if (isa<ConstantInt>(Op2)) {
      int ConstantIntValue = (int)dyn_cast<ConstantInt>(Op2)->getSExtValue();
      Range2 = new dfa::IntervalWrapper(
          dfa::UnconditionalInterval(ConstantIntValue, ConstantIntValue));
    } else {
      InternalRuntimeChecker(DomainIdMap.find(Op2) != DomainIdMap.end());
      Range2 = &IDV[DomainIdMap[Op2]];
    }

    transferIntervalWrapper(Inst, *Range1, *Range2, ODV[Index]);

    if (Inst.getOpcode() == Instruction::ICmp) {
      InternalRuntimeChecker(DomainIdMap.find(&Inst) != DomainIdMap.end());
      InternalRuntimeChecker(DomainIdMap.find(Op1) != DomainIdMap.end());
      InternalRuntimeChecker(isa<ConstantInt>(Op2));
      int ConstantIntValue = (int)dyn_cast<ConstantInt>(Op2)->getSExtValue();
      dfa::ConditionalInterval *CForkedVariable =
          ODV[DomainIdMap.at(Op1)].tryGetConditionalInterval();
      InternalRuntimeChecker(bool(CForkedVariable));
      const ICmpInst *AnICmpInstPtr = dyn_cast<ICmpInst>(&Inst);
      InternalRuntimeChecker(bool(AnICmpInstPtr));
      dfa::UnconditionalInterval *UItvTrueBranch = nullptr;
      dfa::UnconditionalInterval *UItvFalseBranch = nullptr;
      if (AnICmpInstPtr->isGT(AnICmpInstPtr->getPredicate())) {
        UItvTrueBranch = new dfa::UnconditionalInterval(ConstantIntValue + 1,
                                                        dfa::Interval::ItvMax);
        UItvFalseBranch = new dfa::UnconditionalInterval(dfa::Interval::ItvMin,
                                                         ConstantIntValue);
      } else if (AnICmpInstPtr->isLT(AnICmpInstPtr->getPredicate())) {
        UItvTrueBranch = new dfa::UnconditionalInterval(dfa::Interval::ItvMin,
                                                        ConstantIntValue - 1);
        UItvFalseBranch = new dfa::UnconditionalInterval(ConstantIntValue,
                                                         dfa::Interval::ItvMax);
      } else {
        InternalRuntimeChecker(0);
      }
      dfa::ConditionalInterval::CustomKey KeyTrueBranch(
          AnICmpInstPtr, dfa::ConditionalInterval::KindBranch::TrueBranch);
      dfa::ConditionalInterval::CustomKey KeyFalseBranch(
          AnICmpInstPtr, dfa::ConditionalInterval::KindBranch::FalseBranch);
      InternalRuntimeChecker(
          CForkedVariable->ICmpInstToIntervalMap.find(KeyTrueBranch) !=
          CForkedVariable->ICmpInstToIntervalMap.end());
      InternalRuntimeChecker(
          CForkedVariable->ICmpInstToIntervalMap.find(KeyFalseBranch) !=
          CForkedVariable->ICmpInstToIntervalMap.end());
      CForkedVariable->ICmpInstToIntervalMap[KeyTrueBranch] =
          CForkedVariable->ICmpInstToIntervalMap[KeyTrueBranch] &
          *UItvTrueBranch;
      CForkedVariable->ICmpInstToIntervalMap[KeyFalseBranch] =
          CForkedVariable->ICmpInstToIntervalMap[KeyFalseBranch] &
          *UItvFalseBranch;
    }
  } else if (Inst.getOpcode() == Instruction::PHI) {
    InternalRuntimeChecker(isa<llvm::PHINode>(&Inst));
    const llvm::PHINode *AnPHINode = dyn_cast<llvm::PHINode>(&Inst);
    auto Func = [this,
                 &IDV](dfa::UnconditionalInterval &Acc,
                       const llvm::Use &AnUse) -> dfa::UnconditionalInterval {
      if (DomainIdMap.find(AnUse.get()) != DomainIdMap.end()) {
        return Acc | *(IDV.at(DomainIdMap.at(AnUse.get()))
                           .forceGetUnconditionalInterval());
      }
      InternalRuntimeChecker(isa<ConstantInt>(AnUse.get()));
      return Acc | dfa::UnconditionalInterval(
                       dyn_cast<ConstantInt>(AnUse.get())->getSExtValue());
    };
    ODV[Index].forceSetUnconditionalInterval(
        std::accumulate(AnPHINode->incoming_values().begin(),
                        AnPHINode->incoming_values().end(),
                        dfa::UnconditionalInterval::top(), Func));
  } else {
    InternalRuntimeChecker(0);
  }

  syncCondition(ODV);
  if (!(ODV == OldODV)) {
    return true;
  }
  return false;
}

void IntegerRange::syncCondition(DomainVal_t &DV) {
  for (const auto [condition, variable] : ConditionToBindVariableMap) {
    InternalRuntimeChecker(DomainIdMap.find(condition) != DomainIdMap.end());
    InternalRuntimeChecker(DomainIdMap.find(variable) != DomainIdMap.end());
    size_t ConditionIndex = DomainIdMap[condition];
    size_t VariableIndex = DomainIdMap[variable];
    InternalRuntimeChecker(DV[ConditionIndex].tryGetUnconditionalInterval());
    InternalRuntimeChecker(DV[VariableIndex].tryGetConditionalInterval());
    auto &ICmpInstToIntervalMap =
        DV[VariableIndex].tryGetConditionalInterval()->ICmpInstToIntervalMap;
    if (*DV[ConditionIndex].tryGetUnconditionalInterval() ==
        dfa::UnconditionalInterval(1, 1)) {
      dfa::ConditionalInterval::CustomKey Key(
          condition, dfa::ConditionalInterval::KindBranch::FalseBranch);
      InternalRuntimeChecker(ICmpInstToIntervalMap.find(Key) !=
                             ICmpInstToIntervalMap.end());
      ICmpInstToIntervalMap[Key] = dfa::UnconditionalInterval::top();
    } else if (*DV[ConditionIndex].tryGetUnconditionalInterval() ==
               dfa::UnconditionalInterval(0, 0)) {
      dfa::ConditionalInterval::CustomKey Key(
          condition, dfa::ConditionalInterval::KindBranch::TrueBranch);
      InternalRuntimeChecker(ICmpInstToIntervalMap.find(Key) !=
                             ICmpInstToIntervalMap.end());
      ICmpInstToIntervalMap[Key] = dfa::UnconditionalInterval::top();
    }
  }
}
