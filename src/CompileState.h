
#pragma once

#include "types.h"
#include "Panics.h"
#include "Errors.h"
#include "Warnings.h"
#include "Source.h"
#include "helpers.h"

namespace zax
{

struct CompileState
{
  Errors errors_;
  Panics panics_;
  Warnings warnings_;
  int tabStopWidth_{ 8 };

  struct VariableDefaults
  {
    bool varies_{ true };
    bool mutable_{ true };

    inline bool varies() const { return varies_; }
    inline bool _final() const { return !varies_; }

    inline bool _mutable() const { return mutable_; }
    inline bool immutable() const { return !mutable_; }
  } variableDefault_;

  struct TypeDefautls
  {
    bool mutable_{ true };
    bool constant_{ false };

    inline bool _mutable() const { return mutable_; }
    inline bool immutable() const { return !mutable_; }

    inline bool constant() const { return constant_; }
    inline bool inconstant() const { return !constant_; }
  } typeDefault_;

  struct FunctionDefaults
  {
    bool constant_{ false };

    inline bool constant() const { return constant_; }
    inline bool inconstant() const { return !constant_; }
  } functionDefault_;

  struct Deprecate
  {
    enum class Context
    {
      Import,
      All,
      Local
    };

    struct ContextDeclare final : public zs::EnumDeclare<Context, 3>
    {
      constexpr const Entries operator()() const noexcept
      {
        return { {
          {Context::Import, "import"},
          {Context::All, "all"},
          {Context::Local, "local"}
        } };
      }
    };

    using ContextTraits = zs::EnumTraits<Context, ContextDeclare>;

    SourceTypes::Origin origin_;
    Context context_;
    bool forceError_{};
    std::optional<SemanticVersion> min_;
    std::optional<SemanticVersion> max_;
  };
  std::optional<Deprecate> deprecate_;

  struct Export
  {
    bool export_{};

    bool visible() const noexcept { return export_; }
    bool hidden() const noexcept { return !export_; }
  } export_;

  bool isWarningAnError(WarningTypes::Warning warning) const noexcept;

  static CompileStatePtr fork(const CompileStateConstPtr& original) noexcept;
};

} // namespace zax
