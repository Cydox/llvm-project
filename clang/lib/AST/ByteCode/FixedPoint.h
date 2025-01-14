//===------- FixedPoint.h - Fixedd point types for the VM -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_AST_INTERP_FIXED_POINT_H
#define LLVM_CLANG_AST_INTERP_FIXED_POINT_H

#include "clang/AST/APValue.h"
#include "clang/AST/ComparisonCategories.h"
#include "llvm/ADT/APFixedPoint.h"

namespace clang {
namespace interp {

using APInt = llvm::APInt;
using APSInt = llvm::APSInt;

/// Wrapper around fixed point types.
class FixedPoint final {
private:
  llvm::APFixedPoint V;

public:
  FixedPoint(llvm::APFixedPoint &&V) : V(std::move(V)) {}
  FixedPoint(llvm::APFixedPoint &V) : V(V) {}
  FixedPoint(APInt V, llvm::FixedPointSemantics Sem) : V(V, Sem) {}
  // This needs to be default-constructible so llvm::endian::read works.
  FixedPoint()
      : V(APInt(0, 0ULL, false),
          llvm::FixedPointSemantics(0, 0, false, false, false)) {}

  static FixedPoint Zero(llvm::FixedPointSemantics Sem) {
    return FixedPoint(APInt(Sem.getWidth(), 0ULL, Sem.isSigned()), Sem);
  }

  operator bool() const { return V.getBoolValue(); }
  template <typename Ty, typename = std::enable_if_t<std::is_integral_v<Ty>>>
  explicit operator Ty() const {
    // FIXME
    return 0;
  }

  void print(llvm::raw_ostream &OS) const { OS << V; }

  APValue toAPValue(const ASTContext &) const { return APValue(V); }
  APSInt toAPSInt(unsigned BitWidth = 0) const { return V.getValue(); }

  unsigned bitWidth() const { return V.getWidth(); }
  bool isSigned() const { return V.isSigned(); }
  bool isZero() const { return V.getValue().isZero(); }
  bool isNegative() const { return V.getValue().isNegative(); }
  bool isPositive() const { return V.getValue().isNonNegative(); }
  bool isMin() const {
    return V.getValue() == APSInt::getMinValue(V.getSemantics().getWidth(),
                                               !V.getSemantics().isSigned());
  }

  FixedPoint truncate(unsigned BitWidth) const { return *this; }

  FixedPoint toSemantics(const llvm::FixedPointSemantics &Sem,
                         bool *Overflow) const {
    return FixedPoint(V.convert(Sem, Overflow));
  }

  llvm::APFloat toFloat(const llvm::fltSemantics *Sem) const {
    return V.convertToFloat(*Sem);
  }

  std::string toDiagnosticString(const ASTContext &Ctx) const {
    return V.toString();
  }

  ComparisonCategoryResult compare(const FixedPoint &Other) const {
    if (Other.V == V)
      return ComparisonCategoryResult::Equal;
    return ComparisonCategoryResult::Unordered;
  }

  static bool neg(const FixedPoint &A, FixedPoint *R) {
    bool Overflow = false;
    *R = FixedPoint(A.V.negate(&Overflow));
    return Overflow;
  }

  static bool add(const FixedPoint A, const FixedPoint B, unsigned Bits,
                  FixedPoint *R) {
    bool Overflow = false;
    *R = FixedPoint(A.V.add(B.V, &Overflow));
    return Overflow;
  }
  static bool sub(const FixedPoint A, const FixedPoint B, unsigned Bits,
                  FixedPoint *R) {
    return true;
  }
  static bool mul(const FixedPoint A, const FixedPoint B, unsigned Bits,
                  FixedPoint *R) {
    return true;
  }
  static bool div(const FixedPoint A, const FixedPoint B, unsigned Bits,
                  FixedPoint *R) {
    return true;
  }
  static bool increment(const FixedPoint &A, FixedPoint *R) { return true; }
  static bool decrement(const FixedPoint &A, FixedPoint *R) { return true; }
};

inline FixedPoint getSwappedBytes(FixedPoint F) { return F; }

inline llvm::raw_ostream &operator<<(llvm::raw_ostream &OS, FixedPoint F) {
  F.print(OS);
  return OS;
}

} // namespace interp
} // namespace clang

#endif
