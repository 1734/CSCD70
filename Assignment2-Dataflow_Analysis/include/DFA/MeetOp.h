#pragma once // NOLINT(llvm-header-guard)

#include <vector>

namespace dfa {

template <typename TValue> struct MeetOpBase {
  using DomainVal_t = std::vector<TValue>;
  /// @brief Apply the meet operator using two operands.
  /// @param LHS
  /// @param RHS
  /// @return
  virtual DomainVal_t operator()(const DomainVal_t &LHS,
                                 const DomainVal_t &RHS) const = 0;
  /// @brief Return a domain value that represents the top element, used when
  ///        doing the initialization.
  /// @param DomainSize
  /// @return
  virtual DomainVal_t top(const std::size_t DomainSize) const = 0;
  virtual DomainVal_t bottom(const std::size_t DomainSize) const = 0;
};

template <typename TValue> struct Intersect final : MeetOpBase<TValue> {
  using DomainVal_t = typename MeetOpBase<TValue>::DomainVal_t;

  DomainVal_t operator()(const DomainVal_t &LHS,
                         const DomainVal_t &RHS) const final {

    /// @todo(CSCD70) Please complete this method.
    InternalRuntimeChecker(LHS.size() == RHS.size());
    DomainVal_t Result;
    auto It1 = LHS.begin();
    auto It2 = RHS.begin();
    while (It1 != LHS.end() && It2 != RHS.end()) {
      Result.push_back(*It1 & *It2);
      ++It1;
      ++It2;
    }
    return Result;
  }
  DomainVal_t top(const std::size_t DomainSize) const final {

    /// @todo(CSCD70) Please complete this method.
    DomainVal_t Result;
    for (std::size_t I = 0; I < DomainSize; ++I) {
      Result.push_back(TValue::top());
    }
    return Result;
  }

  DomainVal_t bottom(const std::size_t DomainSize) const final {
    DomainVal_t Result;
    for (std::size_t I = 0; I < DomainSize; ++I) {
      Result.push_back(TValue::bottom());
    }
    return Result;
  }
};

/// @todo(CSCD70) Please add another subclass for the Union meet operator.
template <typename TValue> struct Union final : MeetOpBase<TValue> {
  using DomainVal_t = typename MeetOpBase<TValue>::DomainVal_t;

  DomainVal_t operator()(const DomainVal_t &LHS,
                         const DomainVal_t &RHS) const final {

    /// @todo(CSCD70) Please complete this method.
    InternalRuntimeChecker(LHS.size() == RHS.size());
    DomainVal_t Result;
    auto It1 = LHS.begin();
    auto It2 = RHS.begin();
    while (It1 != LHS.end() && It2 != RHS.end()) {
      Result.push_back(*It1 | *It2);
      ++It1;
      ++It2;
    }
    return Result;
  }
  DomainVal_t top(const std::size_t DomainSize) const final {

    /// @todo(CSCD70) Please complete this method.
    DomainVal_t Result;
    for (std::size_t I = 0; I < DomainSize; ++I) {
      Result.push_back(TValue::top());
    }
    return Result;
  }

  DomainVal_t bottom(const std::size_t DomainSize) const final {
    DomainVal_t Result;
    for (std::size_t I = 0; I < DomainSize; ++I) {
      Result.push_back(TValue::bottom());
    }
    return Result;
  }
};

} // namespace dfa
