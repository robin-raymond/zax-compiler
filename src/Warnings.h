
#pragma once

#include "types.h"
#include "Faults.h"

namespace zax
{

struct WarningTypes
{
  enum class Warning
  {
    WarningDirective,
    ToDo,
    IntrinsicTypeCastOverflow,
    SwitchEnum,
    SwitchEnumDefault,
    ConditionNotBoolean,
    SwitchBoolean,
    ShiftCountOverflow,
    ShiftNegative,
    DanglingReferenceOrPointer,
    DeprecateDirective,
    DirectiveNotUnderstood,
    SourceNotFound,
    AssetNotFound,
    Shadowing,
    UninitializedData,
    LifetimeLinkageToUnrelatedPointer,
    NamingConvention,
    ResultNotCaptured,
    VariableDeclaredButNotUsed,
    DuplicateSpecifier,
    SpecifierIgnored,
    TaskNotDeep,
    PromiseNotDeep,
    UnknownDirective,
    UnknownDirectiveArgument,
    Forever,
    DivideByZero,
    AlwaysTrue,
    AlwaysFalse,
    FloatEqual,
    SizeofZero,
    CpuAlignmentNotSupported,
    UpgradeDirective,
    StatementSeparatorOperatorRedundant,
    ExportDisabledFromExportNever,
    RedundantAccessViaSelf,
    RedundantAccessViaOwn,
    BadStyle
  };

  struct WarningDeclare final : public zs::EnumDeclare<Warning, 39>
  {
    constexpr const Entries operator()() const noexcept
    {
      return { {
        {Warning::WarningDirective, "warning-directive"},
        {Warning::ToDo, "to-do"},
        {Warning::IntrinsicTypeCastOverflow, "intrinsic-type-cast-overflow"},
        {Warning::SwitchEnum, "switch-enum"},
        {Warning::SwitchEnumDefault, "switch-enum-default"},
        {Warning::ConditionNotBoolean, "condition-not-boolean"},
        {Warning::SwitchBoolean, "switch-boolean"},
        {Warning::ShiftCountOverflow, "shift-count-overflow"},
        {Warning::ShiftNegative, "shift-negative"},
        {Warning::DanglingReferenceOrPointer, "dangling-reference-or-pointer"},
        {Warning::DeprecateDirective, "deprecate-directive"},
        {Warning::DirectiveNotUnderstood, "directive-not-understood"},
        {Warning::SourceNotFound, "source-not-found"},
        {Warning::AssetNotFound, "asset-not-found"},
        {Warning::Shadowing, "shadowing"},
        {Warning::UninitializedData, "uninitialized-data"},
        {Warning::LifetimeLinkageToUnrelatedPointer, "lifetime-linkage-to-unrelated-pointer"},
        {Warning::NamingConvention, "naming-convention"},
        {Warning::ResultNotCaptured, "result-not-captured"},
        {Warning::VariableDeclaredButNotUsed, "variable-declared-but-not-used"},
        {Warning::DuplicateSpecifier, "duplicate-specifier"},
        {Warning::SpecifierIgnored, "specifier-ignored"},
        {Warning::TaskNotDeep, "task-not-deep"},
        {Warning::PromiseNotDeep, "promise-not-deep"},
        {Warning::UnknownDirective, "unknown-directive"},
        {Warning::UnknownDirectiveArgument, "unknown-directive-argument"},
        {Warning::Forever, "forever"},
        {Warning::DivideByZero, "divide-by-zero"},
        {Warning::AlwaysTrue, "always-true"},
        {Warning::AlwaysFalse, "always-false"},
        {Warning::FloatEqual, "float-equal"},
        {Warning::SizeofZero, "sizeof-zero"},
        {Warning::CpuAlignmentNotSupported, "cpu-alignment-not-supported"},
        {Warning::UpgradeDirective, "upgrade-directive"},
        {Warning::StatementSeparatorOperatorRedundant, "statement-separator-operator-redundant"},
        {Warning::ExportDisabledFromExportNever, "export-disabled-from-export-never"},
        {Warning::RedundantAccessViaSelf, "redundant-access-via-self"},
        {Warning::RedundantAccessViaOwn, "redundant-access-via-own"},
        {Warning::BadStyle, "bad-style"}
      } };
    }
  };

  using WarningTraits = zs::EnumTraits<Warning, WarningDeclare>;
};

using WarningFaults = Faults<WarningTypes::Warning, WarningTypes::WarningTraits>;
using Warnings = WarningFaults;

} // namespace zax
