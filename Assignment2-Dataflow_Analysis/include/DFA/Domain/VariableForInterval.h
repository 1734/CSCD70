#pragma once // NOLINT(llvm-header-guard)

#include <llvm/IR/InstVisitor.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/raw_ostream.h>

#include "Base.h"
#include "Utility.h"

namespace dfa {

struct VariableForInterval final : DomainBase<VariableForInterval> {
  const llvm::Value *const Var;
  VariableForInterval(const llvm::Value *const Var) : Var(Var) {}

  bool operator==(const VariableForInterval &Other) const {
    return Var == Other.Var;
  }

  bool contain(const llvm::Value *const Val) const final {

    /// @todo(CSCD70) Please complete this method.

    return Var == Val;
  }
  VariableForInterval
  replaceValueWith(const llvm::Value *const SrcVal,
                   const llvm::Value *const DstVal) const final {

    /// @todo(CSCD70) Please complete this method.

    return *this;
  }

  using DomainBase<VariableForInterval>::DomainIdMap_t;
  using DomainBase<VariableForInterval>::DomainVector_t;
  struct Initializer : public llvm::InstVisitor<Initializer> {
    DomainIdMap_t &DomainIdMap;
    DomainVector_t &DomainVector;
    std::unordered_map<llvm::ICmpInst *, llvm::Value *>
        &ConditionToBindVariableMap;
    explicit Initializer(DomainIdMap_t &DomainIdMap,
                         DomainVector_t &DomainVector,
                         std::unordered_map<llvm::ICmpInst *, llvm::Value *>
                             &ConditionToBindVariableMap)
        : DomainIdMap(DomainIdMap), DomainVector(DomainVector),
          ConditionToBindVariableMap(ConditionToBindVariableMap) {}
    void visitInstruction(llvm::Instruction &I);
    void visitFunction(llvm::Function &F);
  };
};

} // namespace dfa

llvm::raw_ostream &operator<<(llvm::raw_ostream &,
                              const dfa::VariableForInterval &);

namespace std {

template <> struct hash<::dfa::VariableForInterval> {
  size_t operator()(const dfa::VariableForInterval &Var) const {
    return hash<const llvm::Value *>()(Var.Var);
  }
};

} // namespace std
