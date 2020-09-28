
#pragma once

#include "types.h"
#include "Panics.h"
#include "Errors.h"
#include "Warnings.h"

namespace zax
{

struct CompileState
{
  Errors errors_;
  Panics panics_;
  Warnings warnings_;
  int tabStopWidth_{ 8 };

  struct VariableDefaults {
    bool varies_{ true };
    bool mutable_{ true };

    inline bool varies() const { return varies_; }
    inline bool final() const { return !varies_; }
    inline bool _mutable() const { return mutable_; }
    inline bool immutable() const { return !mutable_; }
  } variableDefault_;

  struct TypeDefautls {
    bool mutable_{ true };
    bool inconstant_{ true };

    inline bool _mutable() const { return mutable_; }
    inline bool immutable() const { return !mutable_; }

    inline bool constant() const { return !inconstant_; }
    inline bool inconstant() const { return inconstant_; }
  } typeDefault_;

  struct FunctionDefaults {
    bool inconstant_{ true };

    inline bool constant() const { return !inconstant_; }
    inline bool inconstant() const { return inconstant_; }
  } functionDefault_;

  bool isWarningAnError(WarningTypes::Warning warning) const noexcept;

  static CompileStatePtr fork(const CompileStateConstPtr& original) noexcept;
};

} // namespace zax
