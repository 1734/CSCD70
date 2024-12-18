#pragma once // NOLINT(llvm-header-guard)

#include "4-LCM/LCM.h"
#include "DFA/Domain/Variable.h"
#include "DFA/Domain/VariableForInterval.h"

#include <DFA/Domain/Expression.h>
#include <DFA/Flow/ForwardAnalysis.h>
#include <DFA/MeetOp.h>

#include <llvm/IR/PassManager.h>

class AvailExprs final : public dfa::ForwardAnalysis<dfa::Expression, dfa::Bool,
                                                     dfa::Intersect<dfa::Bool>>,
                         public llvm::AnalysisInfoMixin<AvailExprs> {
private:
  using ForwardAnalysis_t = dfa::ForwardAnalysis<dfa::Expression, dfa::Bool,
                                                 dfa::Intersect<dfa::Bool>>;

  friend llvm::AnalysisInfoMixin<AvailExprs>;
  static llvm::AnalysisKey Key;

  std::string getName() const final { return "AvailExprs"; }
  bool transferFunc(const llvm::Instruction &, const DomainVal_t &,
                    DomainVal_t &) final;

public:
  using Result = typename ForwardAnalysis_t::AnalysisResult_t;
  using ForwardAnalysis_t::run;
};

class AvailExprsWrapperPass
    : public llvm::PassInfoMixin<AvailExprsWrapperPass> {
public:
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &FAM) {
    FAM.getResult<AvailExprs>(F);
    return llvm::PreservedAnalyses::all();
  }
};

/// @todo(CSCD70) Please complete the main body of the following passes, similar
///               to the Available Expressions pass above.

class LivenessWrapperPass : public llvm::PassInfoMixin<LivenessWrapperPass> {
public:
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &FAM) {

    /// @todo(CSCD70) Get the result from the main body.

    return llvm::PreservedAnalyses::all();
  }
};

class SCCPWrapperPass : public llvm::PassInfoMixin<SCCPWrapperPass> {
public:
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &FAM) {

    /// @todo(CSCD70) Get the result from the main body.

    return llvm::PreservedAnalyses::all();
  }
};

class IntegerRange final
    : public dfa::ForwardAnalysis<dfa::VariableForInterval,
                                  dfa::IntervalWrapper,
                                  dfa::Union<dfa::IntervalWrapper>>,
      public llvm::AnalysisInfoMixin<IntegerRange> {
private:
  using ForwardAnalysis_t =
      dfa::ForwardAnalysis<dfa::VariableForInterval, dfa::IntervalWrapper,
                           dfa::Union<dfa::IntervalWrapper>>;
  std::unordered_map<llvm::ICmpInst *, llvm::Value *>
      ConditionToBindVariableMap;
  std::unordered_map<llvm::ICmpInst *, size_t> ConditionBitIndex;
  std::unordered_map<llvm::Value *, std::vector<llvm::ICmpInst *>>
      BindedVariableToConditionsMap;
  friend llvm::AnalysisInfoMixin<IntegerRange>;
  static llvm::AnalysisKey Key;

  std::string getName() const final { return "IntegerRange"; }
  bool transferFunc(const llvm::Instruction &, const DomainVal_t &,
                    DomainVal_t &) final;
  void syncCondition(DomainVal_t &DV);

  /// @brief Get the list of domain values to which the meet operator will be
  ///        applied.
  /// @param BB
  /// @return
  /// @sa @c getMeetBBConstRange
  MeetOperands_t getMeetOperands(const llvm::BasicBlock &BB) const {
    MeetOperands_t Operands;

    /// @todo(CSCD70) Please complete this method.
    for (const llvm::BasicBlock *PrevBB : getMeetBBConstRange(BB)) {
      InternalRuntimeChecker(PrevBB->getTerminator());
      auto ItVal = InstDomainValMap.find(PrevBB->getTerminator());
      InternalRuntimeChecker(ItVal != InstDomainValMap.end());
      DomainVal_t Operand = ItVal->second;
      if (auto *AnBranchInst =
              llvm::dyn_cast<llvm::BranchInst>(PrevBB->getTerminator())) {
        if (AnBranchInst->isConditional()) {
          llvm::Value *Condition = AnBranchInst->getCondition();
          InternalRuntimeChecker(DomainIdMap.find(Condition) !=
                                 DomainIdMap.end());
          dfa::UnconditionalInterval *UItv =
              Operand[DomainIdMap.at(Condition)].tryGetUnconditionalInterval();
          InternalRuntimeChecker(bool(UItv));
          if (AnBranchInst->getSuccessor(0) == &BB) {
            *UItv = dfa::UnconditionalInterval(1, 1);
          } else if (AnBranchInst->getSuccessor(1) == &BB) {
            *UItv = dfa::UnconditionalInterval(0, 0);
          } else {
            InternalRuntimeChecker(0);
          }
        }
      }

      Operands.push_back(Operand);
    }
    return Operands;
  }

  std::vector<size_t> getIndexesMatchingBits(const llvm::Value *AnValue,
                                             size_t OneBits, size_t ZeroBits) {
    std::vector<size_t> Indexes;
    for (size_t I = 0; I < (size_t)(1 << BindedVariableToConditionsMap
                                             .at((llvm::Value *)AnValue)
                                             .size());
         ++I) {
      if ((I & OneBits) != 0 || (I & ZeroBits) != ZeroBits) {
        Indexes.push_back(I);
      }
    }
    return Indexes;
  }

public:
  using Result = typename ForwardAnalysis_t::AnalysisResult_t;
  using ForwardAnalysis_t::run;
  /// @brief Traverse through the CFG of the function.
  /// @param F
  /// @return True if either BasicBlock-DomainValue mapping or
  ///         Instruction-DomainValue mapping has been modified, false
  ///         otherwise.
  bool traverseCFG(const llvm::Function &F) {
    bool Changed = false;

    /// @todo(CSCD70) Please complete this method.
    for (const auto &BB : getBBConstRange(F)) {
      if (!BB.isEntryBlock()) {
        BVs[&BB] = getBoundaryVal(BB);
        syncCondition(BVs[&BB]);
      }
      const DomainVal_t *PrevInstrOut = &BVs[&BB];
      for (const auto &Inst : getInstConstRange(BB)) {
        if (transferFunc(Inst, *PrevInstrOut, InstDomainValMap[&Inst])) {
          Changed = true;
        }
        PrevInstrOut = &InstDomainValMap[&Inst];
        printInstDomainValMap(Inst);
      }
    }
    return Changed;
  }

  virtual AnalysisResult_t run(llvm::Function &F,
                               llvm::FunctionAnalysisManager &FAM) {

    /// @todo(CSCD70) Please complete this method.
    Initializer(DomainIdMap, DomainVector, ConditionToBindVariableMap).visit(F);

    for (const auto &[condition, variable] : ConditionToBindVariableMap) {
      InternalRuntimeChecker(DomainIdMap.find(variable) != DomainIdMap.end());
      if (BindedVariableToConditionsMap.find(variable) ==
          BindedVariableToConditionsMap.end()) {
        BindedVariableToConditionsMap[variable] =
            std::vector<llvm::ICmpInst *>();
      }
      ConditionBitIndex[condition] =
          BindedVariableToConditionsMap.at(variable).size();
      BindedVariableToConditionsMap[variable].push_back(condition);
    }

    for (const auto &[BindedVariable, Conditions] :
         BindedVariableToConditionsMap) {
      llvm::errs() << *BindedVariable << ": [";
      for (const llvm::ICmpInst *AnICmpInst : Conditions) {
        llvm::errs() << *AnICmpInst << ", ";
      }
      llvm::errs() << "]\n";
    }

    TopDomainVal = dfa::Union<dfa::IntervalWrapper>().top(DomainVector.size());
    BottomDomainVal =
        dfa::Union<dfa::IntervalWrapper>().bottom(DomainVector.size());
    for (const auto &[BindedVariable, Conditions] :
         BindedVariableToConditionsMap) {
      size_t UItvsSize = 1 << Conditions.size();
      size_t VariableIndex = DomainIdMap.at(BindedVariable);

      if (TopDomainVal[VariableIndex].tryGetUnconditionalInterval()) {
        TopDomainVal[VariableIndex] = dfa::ConditionalInterval();
      }
      InternalRuntimeChecker(
          TopDomainVal[VariableIndex].tryGetConditionalInterval());

      if (BottomDomainVal[VariableIndex].tryGetUnconditionalInterval()) {
        BottomDomainVal[VariableIndex] = dfa::ConditionalInterval();
      }
      InternalRuntimeChecker(
          BottomDomainVal[VariableIndex].tryGetConditionalInterval());

      for (size_t UItvIndex = 0; UItvIndex < UItvsSize; ++UItvIndex) {
        TopDomainVal[VariableIndex]
            .tryGetConditionalInterval()
            ->UItvs.push_back(dfa::UnconditionalInterval::top());
        BottomDomainVal[VariableIndex]
            .tryGetConditionalInterval()
            ->UItvs.push_back(dfa::UnconditionalInterval::bottom());
      }
    }

    LOG_ANALYSIS_INFO << stringifyDomainWithMask(TopDomainVal) << "\n";
    LOG_ANALYSIS_INFO << stringifyDomainWithMask(BottomDomainVal) << "\n";
    for (const llvm::BasicBlock &BB : getBBConstRange(F)) {
      if (BB.isEntryBlock()) {
        BVs[&BB] = TopDomainVal;
      }
      for (const llvm::Instruction &Inst : getInstConstRange(BB)) {
        InstDomainValMap[&Inst] = TopDomainVal;
      }
    }
    while (traverseCFG(F)) {
      // Framework_t::printInstDomainValMap(F);
      LOG_ANALYSIS_INFO
          << "==================================================="
             "===================================================\n";
    }
    return std::make_tuple(DomainIdMap, DomainVector, BVs, InstDomainValMap);
  }
};

class IntegerRangeWrapperPass
    : public llvm::PassInfoMixin<IntegerRangeWrapperPass> {
public:
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &FAM) {
    FAM.getResult<IntegerRange>(F);
    return llvm::PreservedAnalyses::all();
  }
};
