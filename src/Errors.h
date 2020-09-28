
#pragma once

#include "types.h"
#include "Faults.h"

namespace zax
{

struct ErrorTypes
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
    DuplicateSymbol,
    ConditionExpectsBoolean,
    MissingEndOfQuote,
    MissingEndOfComment,
    CompilesDirectiveError,
    RequiresDirectiveError,
    ValueNotCaptured,
    UnmatchedPush,
    ScopeFlowControlSkipsDeclaration,
    InlineFunctionNotFinal,
    ConstantSyntax,
    Syntax,
    OutputFailure
  };

  struct ErrorDeclare final : public zs::EnumDeclare<Error, 47>
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
        {Error::DuplicateSymbol, "duplicate-symbol"},
        {Error::ConditionExpectsBoolean, "condition-expects-boolean"},
        {Error::MissingEndOfQuote, "missing-end-of-quote"},
        {Error::MissingEndOfComment, "missing-end-of-comment"},
        {Error::CompilesDirectiveError, "compiles-directive-error"},
        {Error::RequiresDirectiveError, "requires-directive-error"},
        {Error::ValueNotCaptured, "value-not-captured"},
        {Error::UnmatchedPush, "unmatched-push"},
        {Error::ScopeFlowControlSkipsDeclaration, "scope-flow-control-skips-declaration"},
        {Error::InlineFunctionNotFinal, "inline-function-not-final"},
        {Error::ConstantSyntax, "constant-syntax"},
        {Error::Syntax, "syntax"},
        {Error::OutputFailure, "output-failure"}
      } };
    }
  };

  struct ErrorHumanReadableDeclare final : public zs::EnumDeclare<Error, 47>
  {
    constexpr const Entries operator()() const noexcept
    {
      return { {
        {Error::ErrorDirective, "$message$"},
        {Error::MissingArgument, "missing-argument"},
        {Error::LiteralContainsInvalidSequence, "literal-contains-invalid-sequence"},
        {Error::IncompatibleDirective, "incompatible-directive"},
        {Error::DeprecateDirective, "deprecate-directive"},
        {Error::ImportedModuleNotFound, "imported-module-not-found"},
        {Error::ImportedModuleFailure, "imported-module-failure"},
        {Error::SourceNotFound, "a source file ($file$) was requested to be parsed but it cannot be located"},
        {Error::AssetNotFound, "an asset file ($file$) was requested to be copied but it cannot be located"},
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
        {Error::DuplicateSymbol, "duplicate-symbol"},
        {Error::ConditionExpectsBoolean, "condition-expects-boolean"},
        {Error::MissingEndOfQuote, "a quote ' or \" was started but the matching end quote ' or \" is missing"},
        {Error::MissingEndOfComment, "a comment /* or /** was started but the matching end comment */ or **/ is missing"},
        {Error::CompilesDirectiveError, "compiles-directive-error"},
        {Error::RequiresDirectiveError, "requires-directive-error"},
        {Error::ValueNotCaptured, "value-not-captured"},
        {Error::UnmatchedPush, "unmatched-push"},
        {Error::ScopeFlowControlSkipsDeclaration, "scope-flow-control-skips-declaration"},
        {Error::InlineFunctionNotFinal, "inline-function-not-final"},
        {Error::ConstantSyntax, "a constant was found to contain a syntax error"},
        {Error::Syntax, "a syntax error was found"},
        {Error::OutputFailure, "an attempt to generate output or copy an asset to the output ($file$) failed"}
      } };
    }
  };

  using ErrorTraits = zs::EnumTraits<Error, ErrorDeclare>;
  using ErrorHumanReadableTraits = zs::EnumTraits<Error, ErrorHumanReadableDeclare>;

};

using ErrorFaults = Faults<ErrorTypes::Error, ErrorTypes::ErrorTraits>;
using Errors = ErrorFaults;

} // namespace zax
