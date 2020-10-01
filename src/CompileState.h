
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

  struct VariableDefaults
  {
    bool varies_{ true };
    bool mutable_{ true };

    inline bool varies() const { return varies_; }
    inline bool _final() const { return !varies_; }

    inline bool _mutable() const { return mutable_; }
    inline bool immutable() const { return !mutable_; }
  } variableDefaults_;

  struct TypeDefaults
  {
    bool mutable_{ true };
    bool constant_{ false };

    inline bool _mutable() const { return mutable_; }
    inline bool immutable() const { return !mutable_; }

    inline bool constant() const { return constant_; }
    inline bool inconstant() const { return !constant_; }
  } typeDefaults_;

  struct FunctionDefaults
  {
    bool constant_{ false };

    inline bool constant() const { return constant_; }
    inline bool inconstant() const { return !constant_; }
  } functionDefaults_;

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

  std::stack<VariableDefaults> variableDefaultsStack_;
  std::stack<TypeDefaults> typeDefaultsStack_;
  std::stack<FunctionDefaults> functionDefaultsStack_;
  std::stack<Export> exportStack_;

  bool isWarningAnError(WarningTypes::Warning warning) const noexcept;

  static CompileStatePtr fork(const CompileStateConstPtr& original) noexcept;

  void pushVariableDefaults() noexcept { return variableDefaultsStack_.push(variableDefaults_); }
  void pushTypeDefaults() noexcept { return typeDefaultsStack_.push(typeDefaults_); }
  void pushFunctionDefaults() noexcept { return functionDefaultsStack_.push(functionDefaults_); }
  void pushExport() noexcept { return exportStack_.push(export_); }

  template <typename TType>
  bool pop(std::stack<TType>& s, TType& original) noexcept { if (s.size() < 1) return false; original = s.top(); s.pop(); return true; }

  bool popVariableDefaults() noexcept { return pop(variableDefaultsStack_, variableDefaults_); }
  bool popTypeDefaults() noexcept { return pop(typeDefaultsStack_, typeDefaults_); }
  bool popFunctionDefaults() noexcept { return pop(functionDefaultsStack_, functionDefaults_); }
  bool popExport() noexcept { return pop(exportStack_, export_); }
};

} // namespace zax
