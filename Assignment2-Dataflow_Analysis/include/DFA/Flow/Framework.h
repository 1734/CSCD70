#pragma once // NOLINT(llvm-header-guard)

#include "Utility.h"
#include <llvm/IR/Function.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Support/raw_ostream.h>

#include <numeric>
#include <ostream>
#include <tuple>
namespace dfa {

template <typename TValue> struct ValuePrinter {
  static std::string print(const TValue &V) { return ""; }
};

template <typename TDomainElem, typename TValue, typename TMeetOp,
          typename TMeetBBConstRange, typename TBBConstRange,
          typename TInstConstRange>
class Framework {
protected:
  using DomainIdMap_t = typename TDomainElem::DomainIdMap_t;
  using DomainVector_t = typename TDomainElem::DomainVector_t;
  using DomainVal_t = typename TMeetOp::DomainVal_t;
  using Initializer = typename TDomainElem::Initializer;
  using MeetOperands_t = std::vector<DomainVal_t>;
  using MeetBBConstRange_t = TMeetBBConstRange;
  using BBConstRange_t = TBBConstRange;
  using InstConstRange_t = TInstConstRange;
  using AnalysisResult_t =
      std::tuple<DomainIdMap_t, DomainVector_t,
                 std::unordered_map<const llvm::BasicBlock *, DomainVal_t>,
                 std::unordered_map<const llvm::Instruction *, DomainVal_t>>;

  DomainIdMap_t DomainIdMap;
  DomainVector_t DomainVector;
  DomainVal_t TopDomainVal;
  DomainVal_t BottomDomainVal;
  std::unordered_map<const llvm::BasicBlock *, DomainVal_t> BVs;
  std::unordered_map<const llvm::Instruction *, DomainVal_t> InstDomainValMap;

  /// @name Print utility functions
  /// @{

  std::string stringifyDomainWithMask(const DomainVal_t &Mask) const {
    std::string StringBuf;
    llvm::raw_string_ostream Strout(StringBuf);
    Strout << "{";
    CHECK(Mask.size() == DomainIdMap.size() &&
          Mask.size() == DomainVector.size())
        << "The size of mask must be equal to the size of domain, but got "
        << Mask.size() << " vs. " << DomainIdMap.size() << " vs. "
        << DomainVector.size() << " instead";
    for (size_t DomainId = 0; DomainId < DomainIdMap.size(); ++DomainId) {
      // if (!static_cast<bool>(Mask[DomainId])) {
      //   continue;
      // }
      Strout << DomainVector.at(DomainId)
             << ValuePrinter<TValue>::print(Mask[DomainId]) << ", ";
    } // for (MaskIdx : [0, Mask.size()))
    Strout << "}";
    return StringBuf;
  }
  virtual void printInstDomainValMap(const llvm::Instruction &Inst) const = 0;
  void printInstDomainValMap(const llvm::Function &F) const {
    for (const llvm::Instruction &Inst : llvm::instructions(&F)) {
      printInstDomainValMap(Inst);
    }
  }
  virtual std::string getName() const = 0;

  /// @}
  /// @name Boundary values
  /// @{

  DomainVal_t getBoundaryVal(const llvm::BasicBlock &BB) const {
    MeetOperands_t MeetOperands = getMeetOperands(BB);

    /// @todo(CSCD70) Please complete this method.

    return meet(MeetOperands);
  }
  /// @brief Get the list of basic blocks to which the meet operator will be
  ///        applied.
  /// @param BB
  /// @return
  virtual MeetBBConstRange_t
  getMeetBBConstRange(const llvm::BasicBlock &BB) const = 0;
  /// @brief Get the list of domain values to which the meet operator will be
  ///        applied.
  /// @param BB
  /// @return
  /// @sa @c getMeetBBConstRange
  virtual MeetOperands_t getMeetOperands(const llvm::BasicBlock &BB) const {
    MeetOperands_t Operands;

    /// @todo(CSCD70) Please complete this method.
    for (auto MeetBB : getMeetBBConstRange(BB)) {
      InternalRuntimeChecker(bool(MeetBB->getTerminator()));
      auto ItVal = InstDomainValMap.find(MeetBB->getTerminator());
      InternalRuntimeChecker(ItVal != InstDomainValMap.end());
      printInstDomainValMap(*MeetBB->getTerminator());
      Operands.push_back(ItVal->second);
    }
    return Operands;
  }
  DomainVal_t bc() const { return DomainVal_t(DomainIdMap.size()); }
  DomainVal_t meet(const MeetOperands_t &MeetOperands) const {

    /// @todo(CSCD70) Please complete this method.
    return std::accumulate(MeetOperands.begin(), MeetOperands.end(),
                           TopDomainVal,
                           [](DomainVal_t Acc, DomainVal_t Current) {
                             return TMeetOp().operator()(Acc, Current);
                           });
  }

  /// @}
  /// @name CFG traversal
  /// @{

  /// @brief Get the list of basic blocks from the function.
  /// @param F
  /// @return
  virtual BBConstRange_t getBBConstRange(const llvm::Function &F) const = 0;
  /// @brief Get the list of instructions from the basic block.
  /// @param BB
  /// @return
  virtual InstConstRange_t
  getInstConstRange(const llvm::BasicBlock &BB) const = 0;
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
      }
      const DomainVal_t *PrevInstrOut = &BVs[&BB];
      for (const auto &Inst : getInstConstRange(BB)) {
        if (transferFunc(Inst, *PrevInstrOut, InstDomainValMap[&Inst])) {
          Changed = true;
        }
        PrevInstrOut = &InstDomainValMap[&Inst];
      }
    }
    return Changed;
  }

  /// @}

  virtual ~Framework() {}

  /// @brief Apply the transfer function to the input domain value at
  ///        instruction @p inst .
  /// @param Inst
  /// @param IDV
  /// @param ODV
  /// @return Whether the output domain value is to be changed.
  virtual bool transferFunc(const llvm::Instruction &Inst,
                            const DomainVal_t &IDV, DomainVal_t &ODV) = 0;

  virtual AnalysisResult_t run(llvm::Function &F,
                               llvm::FunctionAnalysisManager &FAM) {

    /// @todo(CSCD70) Please complete this method.
    // Initializer(DomainIdMap, DomainVector).visit(F);
    // for (const auto &BB : getBBConstRange(F)) {
    //   if (BB.isEntryBlock()) {
    //     BVs[&BB] = TMeetOp().bottom(DomainVector.size());
    //   }
    //   for (const auto &Inst : getInstConstRange(BB)) {
    //     InstDomainValMap[&Inst] = TMeetOp().top(DomainVector.size());
    //   }
    // }
    // while (traverseCFG(F)) {
    // }
    // printInstDomainValMap(F);
    return std::make_tuple(DomainIdMap, DomainVector, BVs, InstDomainValMap);
  }

  void checkInDomain(const TDomainElem &Elem) {
    InternalRuntimeChecker(DomainIdMap.find(Elem) != DomainIdMap.end());
  }

}; // class Framework

/// @brief For each domain element type, we have to define:
///        - The default constructor
///        - The meet operators (for intersect/union)
///        - The top element
///        - Conversion to bool (for logging)
struct Bool {
  bool Value = false;
  Bool operator&(const Bool &Other) const {
    return {.Value = Value && Other.Value};
  }
  Bool operator|(const Bool &Other) const {
    return {.Value = Value || Other.Value};
  }
  static Bool top() { return {.Value = true}; }
  static Bool bottom() { return {.Value = false}; }
  explicit operator bool() const { return Value; }
};

template <> struct ValuePrinter<Bool> {
  static std::string print(const Bool &V) { return V ? "true" : "false"; };
};

struct Interval {
  enum { ItvMin = -1000, ItvMax = 1000 };
  virtual ~Interval() = default;
};

struct UnconditionalInterval : public Interval {
  int Low = 0, High = 0;

  UnconditionalInterval() {}

  explicit UnconditionalInterval(int _low, int _high)
      : Low(std::max(_low, (int)ItvMin)), High(std::min(_high, (int)ItvMax)) {
    if (Low > High) {
      Low = 0;
      High = -1;
    }
  }

  explicit UnconditionalInterval(int i)
      : Low(std::max(i, (int)ItvMin)), High(std::min(i, (int)ItvMax)) {}

  UnconditionalInterval operator&(const UnconditionalInterval &Other) const {
    if (Low > High) {
      return *this;
    }
    if (Other.Low > Other.High) {
      return Other;
    }
    return UnconditionalInterval(std::max(Low, Other.Low),
                                 std::min(High, Other.High));
  }

  UnconditionalInterval operator|(const UnconditionalInterval &Other) const {
    if (Low > High) {
      return Other;
    }
    if (Other.Low > Other.High) {
      return *this;
    }
    return UnconditionalInterval(std::min(Low, Other.Low),
                                 std::max(High, Other.High));
  }

  bool operator==(const UnconditionalInterval &Other) const {
    return Low == Other.Low && High == Other.High;
  }

  static UnconditionalInterval top() { return UnconditionalInterval(0, -1); }

  static UnconditionalInterval bottom() {
    return UnconditionalInterval(ItvMin, ItvMax);
  }
};

struct ConditionalInterval : public Interval {
  std::vector<UnconditionalInterval> UItvs;
  ConditionalInterval() {}

  ConditionalInterval(std::vector<UnconditionalInterval> _UItvs)
      : UItvs(_UItvs) {}

  ConditionalInterval operator&(const ConditionalInterval &Other) const {
    InternalRuntimeChecker(UItvs.size() == Other.UItvs.size());
    std::vector<UnconditionalInterval> ResultUItvs;
    for (size_t Index = 0; Index < UItvs.size(); ++Index) {
      ResultUItvs.push_back(UItvs.at(Index) & Other.UItvs.at(Index));
    }
    return ConditionalInterval(ResultUItvs);
  }

  ConditionalInterval operator|(const ConditionalInterval &Other) const {
    InternalRuntimeChecker(UItvs.size() == Other.UItvs.size());
    std::vector<UnconditionalInterval> ResultUItvs;
    for (size_t Index = 0; Index < UItvs.size(); ++Index) {
      ResultUItvs.push_back(UItvs.at(Index) | Other.UItvs.at(Index));
    }
    return ConditionalInterval(ResultUItvs);
  }

  dfa::UnconditionalInterval *getMeet() const {
    return new dfa::UnconditionalInterval(std::accumulate(
        UItvs.begin(), UItvs.end(), dfa::UnconditionalInterval::top(),
        [](dfa::UnconditionalInterval Acc,
           const dfa::UnconditionalInterval &U) { return Acc | U; }));
  }

  bool operator==(const ConditionalInterval &Other) const {
    return UItvs == Other.UItvs;
  }
};

struct IntervalWrapper {
private:
  std::variant<UnconditionalInterval, ConditionalInterval> Data;

public:
  IntervalWrapper(const UnconditionalInterval &UItv) : Data(UItv) {}
  IntervalWrapper(const ConditionalInterval &CItv) : Data(CItv) {}

  UnconditionalInterval *tryGetUnconditionalInterval() {
    return std::get_if<UnconditionalInterval>(&Data);
  }

  const UnconditionalInterval *tryGetUnconditionalInterval() const {
    return std::get_if<UnconditionalInterval>(&Data);
  }

  ConditionalInterval *tryGetConditionalInterval() {
    return std::get_if<ConditionalInterval>(&Data);
  }

  const ConditionalInterval *tryGetConditionalInterval() const {
    return std::get_if<ConditionalInterval>(&Data);
  }

  const UnconditionalInterval *forceGetUnconditionalInterval() const {
    const dfa::UnconditionalInterval *UItv = tryGetUnconditionalInterval();
    if (!UItv) {
      const dfa::ConditionalInterval *CItv = tryGetConditionalInterval();
      InternalRuntimeChecker(bool(CItv));
      UItv = CItv->getMeet();
    }
    return UItv;
  }

  void forceSetUnconditionalInterval(const UnconditionalInterval &SomeUItv) {
    if (tryGetUnconditionalInterval()) {
      *tryGetUnconditionalInterval() = SomeUItv;
    } else {
      InternalRuntimeChecker(bool(tryGetConditionalInterval()));
      for (auto &UItv : tryGetConditionalInterval()->UItvs) {
        UItv = SomeUItv;
      }
    }
  }

  IntervalWrapper operator&(const IntervalWrapper &Other) const {
    const UnconditionalInterval *UPtr = tryGetUnconditionalInterval();
    if (UPtr) {
      const UnconditionalInterval *UPtrOther =
          Other.tryGetUnconditionalInterval();
      InternalRuntimeChecker(bool(UPtrOther));
      return IntervalWrapper(UPtr->operator&(*UPtrOther));
    }
    const ConditionalInterval *CPtr = tryGetConditionalInterval();
    InternalRuntimeChecker(bool(CPtr));
    const ConditionalInterval *CPtrOther = Other.tryGetConditionalInterval();
    InternalRuntimeChecker(bool(CPtrOther));
    return IntervalWrapper(CPtr->operator&(*CPtrOther));
  }

  IntervalWrapper operator|(const IntervalWrapper &Other) const {
    const UnconditionalInterval *UPtr = tryGetUnconditionalInterval();
    if (UPtr) {
      const UnconditionalInterval *UPtrOther =
          Other.tryGetUnconditionalInterval();
      InternalRuntimeChecker(bool(UPtrOther));
      return IntervalWrapper(UPtr->operator|(*UPtrOther));
    }
    const ConditionalInterval *CPtr = tryGetConditionalInterval();
    InternalRuntimeChecker(bool(CPtr));
    const ConditionalInterval *CPtrOther = Other.tryGetConditionalInterval();
    InternalRuntimeChecker(bool(CPtrOther));
    return IntervalWrapper(CPtr->operator|(*CPtrOther));
  }

  bool operator==(const IntervalWrapper &Other) const {
    if (tryGetUnconditionalInterval()) {
      if (!Other.tryGetUnconditionalInterval()) {
        return false;
      }
      return *tryGetUnconditionalInterval() ==
             *Other.tryGetUnconditionalInterval();
    }
    InternalRuntimeChecker(static_cast<bool>(tryGetConditionalInterval()));
    if (!Other.tryGetConditionalInterval()) {
      return false;
    }
    return *tryGetConditionalInterval() == *Other.tryGetConditionalInterval();
  }

  static IntervalWrapper top() { return UnconditionalInterval(0, -1); }

  static IntervalWrapper bottom() {
    return UnconditionalInterval(Interval::ItvMin, Interval::ItvMax);
  }
};

template <> struct ValuePrinter<IntervalWrapper> {
  static std::string print(const IntervalWrapper &V) {
    char Buffer[1024] = {0};
    const UnconditionalInterval *UPtr = V.tryGetUnconditionalInterval();
    if (UPtr) {
      std::snprintf(Buffer, sizeof(Buffer), "[%d, %d]", UPtr->Low, UPtr->High);
    } else {
      const ConditionalInterval *CPtr = V.tryGetConditionalInterval();
      InternalRuntimeChecker(bool(CPtr));
      for (const UnconditionalInterval &AnInterval : CPtr->UItvs) {
        std::snprintf(Buffer + strlen(Buffer), sizeof(Buffer) - strlen(Buffer),
                      "[%d, %d]", AnInterval.Low, AnInterval.High);
      }
    }
    return Buffer;
  }
};

} // namespace dfa
