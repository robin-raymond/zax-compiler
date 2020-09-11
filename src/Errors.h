
#pragma once

#include "types.h"
#include "Faults.h"

namespace zax
{

struct ErrorsTypes
{
  enum class Error
  {
    ErrorDirective,
    MissingArgument,
    LiteralContainsInvalidSequence,
    IncompatibleDirective,
    DeprecateDirective,
    ImportedModuleNotFound,
    ImportedModuleFailure,
    SourceNotFound,
    AssetNotFound,
    WildCharacterMismatch,
    FinalFunctionPointsToNothing,
    DereferencePointerToNothing,
    TokenExpected,
    TokenUnexpected,
    AsConversionNotCompatible,
    SoaAosIncompatible,
    ConstantOverflow,
    NeedsDereferencing,
    IncompatibleTypes,
    NoViableOuterTypeCast,
    FunctionNotFound,
    TypeNotFound,
    FunctionCandidateNotFound,
    FunctionCandidateAmbiguous,
    OutercastAmbiguous,
    ExceptAmbiguous,
    EnumToUnderlyingNeedsAsOperator,
    EnumToIncompatibleType,
    RangeIteratorNotFound,
    NamedScopeNotFound,
    NamedScopeInaccessible,
    LineDirectiveWithoutFile,
    BadAlignment,
    DuplicateCase,
    ConditionExpectsBoolean,
    MissingEndOfComments,
    CompilesDirectiveError,
    RequiresDirectiveError,
    ValueNotCaptured
  };

  struct ErrorDeclare final : public zs::EnumDeclare<Error, 39>
  {
    constexpr const Entries operator()() const noexcept
    {
      return { {
        {Error::ErrorDirective, "error-directive"},
        {Error::MissingArgument, "missing-argument"},
        {Error::LiteralContainsInvalidSequence, "literal-contains-invalid-sequence"},
        {Error::IncompatibleDirective, "incompatible-directive"},
        {Error::DeprecateDirective, "deprecate-directive"},
        {Error::ImportedModuleNotFound, "imported-module-not-found"},
        {Error::ImportedModuleFailure, "imported-module-failure"},
        {Error::SourceNotFound, "source-not-found"},
        {Error::AssetNotFound, "asset-not-found"},
        {Error::WildCharacterMismatch, "wild-character-mismatch"},
        {Error::FinalFunctionPointsToNothing, "final-function-points-to-nothing"},
        {Error::DereferencePointerToNothing, "dereference-pointer-to-nothing"},
        {Error::TokenExpected, "token-expected"},
        {Error::TokenUnexpected, "token-unexpected"},
        {Error::AsConversionNotCompatible, "as-conversion-not-compatible"},
        {Error::SoaAosIncompatible, "soa-aos-incompatible"},
        {Error::ConstantOverflow, "constant-overflow"},
        {Error::NeedsDereferencing, "needs-dereferencing"},
        {Error::IncompatibleTypes, "incompatible-types"},
        {Error::NoViableOuterTypeCast, "no-viable-outer-type-cast"},
        {Error::FunctionNotFound, "function-not-found"},
        {Error::TypeNotFound, "type-not-found"},
        {Error::FunctionCandidateNotFound, "function-candidate-not-found"},
        {Error::FunctionCandidateAmbiguous, "function-candidate-ambiguous"},
        {Error::OutercastAmbiguous, "outercast-ambiguous"},
        {Error::ExceptAmbiguous, "except-ambiguous"},
        {Error::EnumToUnderlyingNeedsAsOperator, "enum-to-underlying-needs-as-operator"},
        {Error::EnumToIncompatibleType, "enum-to-incompatible-type"},
        {Error::RangeIteratorNotFound, "range-iterator-not-found"},
        {Error::NamedScopeNotFound, "named-scope-not-found"},
        {Error::NamedScopeInaccessible, "named-scope-inaccessible"},
        {Error::LineDirectiveWithoutFile, "line-directive-without-file"},
        {Error::BadAlignment, "bad-alignment"},
        {Error::DuplicateCase, "duplicate-case"},
        {Error::ConditionExpectsBoolean, "condition-expects-boolean"},
        {Error::MissingEndOfComments, "missing-end-of-comments"},
        {Error::CompilesDirectiveError, "compiles-directive-error"},
        {Error::RequiresDirectiveError, "requires-directive-error"},
        {Error::ValueNotCaptured, "value-not-captured"}
      } };
    }
  };

  using ErrorTraits = zs::EnumTraits<Error, ErrorDeclare>;

};

using ErrorFaults = Faults<ErrorsTypes::Error, ErrorsTypes::ErrorTraits>;
using Errors = ErrorFaults;

} // namespace zax
