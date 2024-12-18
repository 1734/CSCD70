#pragma once // NOLINT(llvm-header-guard)

#include <llvm/IR/InstVisitor.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/raw_ostream.h>

#include "Base.h"
#include "Utility.h"

namespace dfa {

struct Variable final : DomainBase<Variable> {
  const llvm::Value *const Var;
  Variable(const llvm::Value *const Var) : Var(Var) {}

  bool operator==(const Variable &Other) const { return Var == Other.Var; }

  bool contain(const llvm::Value *const Val) const final {

    /// @todo(CSCD70) Please complete this method.

    return Var == Val;
  }
  Variable replaceValueWith(const llvm::Value *const SrcVal,
                            const llvm::Value *const DstVal) const final {

    /// @todo(CSCD70) Please complete this method.

    return *this;
  }

  using DomainBase<Variable>::DomainIdMap_t;
  using DomainBase<Variable>::DomainVector_t;

  struct Initializer : public llvm::InstVisitor<Initializer> {
    DomainIdMap_t &DomainIdMap;
    DomainVector_t &DomainVector;
    explicit Initializer(DomainIdMap_t &DomainIdMap,
                         DomainVector_t &DomainVector,
                         std::unordered_map<llvm::ICmpInst *, llvm::Value *>
                             &ConditionToBindVariableMap)
        : DomainIdMap(DomainIdMap), DomainVector(DomainVector) {}
    void visitInstruction(llvm::Instruction &I);
    void visitFunction(llvm::Function &F);
  };
};

} // namespace dfa

llvm::raw_ostream &operator<<(llvm::raw_ostream &, const dfa::Variable &);

namespace std {

template <> struct hash<::dfa::Variable> {
  size_t operator()(const dfa::Variable &Var) const {
    return hash<const llvm::Value *>()(Var.Var);
  }
};

} // namespace std
