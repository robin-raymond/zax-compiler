
#pragma once

#include "types.h"
#include "Config.h"
#include "Errors.h"
#include "Warnings.h"
#include "Informationals.h"
#include "Token.h"
#include "TokenList.h"
#include "Tokenizer.h"

namespace zax
{

//-----------------------------------------------------------------------------
struct ParserTypes
{
  constexpr static StringView unknownPrefix{ "x-" };

  using Operator = TokenTypes::Operator;
  using Error = ErrorTypes::Error;
  using Warning = WarningTypes::Warning;
  using Informational = InformationalTypes::Informational;
  using TokenType = TokenTypes::Type;

  using ModuleMap = std::map<String, ModulePtr>;
  using ModuleList = std::list<ModulePtr>;
  using SourceList = std::list<SourcePtr>;

  struct Callbacks
  {
    std::function<void(Error, const TokenConstPtr& token, const StringMap&)> fatal_;
    std::function<void(Error, const TokenConstPtr& token, const StringMap&)> error_;
    std::function<void(Warning, const TokenConstPtr& token, const StringMap&)> warning_;
    std::function<void(Informational, const TokenConstPtr& token, const StringMap&)> info_;

    std::function<bool()> shouldAbort_;
  };

  struct QuoteResult {
    Tokenizer::iterator quoteIter_;
    Tokenizer::iterator afterIter_;
    String quote_;
  };
  struct NumberResult {
    Tokenizer::iterator numberIter_;
    Tokenizer::iterator afterIter_;
    String number_;
  };
  struct Extraction {
    ContextPtr context_;
    TokenizerPtr tokenizer_;

    bool hasValue() const noexcept { return static_cast<bool>(tokenizer_); }
  };
};

//-----------------------------------------------------------------------------
struct ParserDirectiveTypes
{
  enum class SourceAssetRequired {
    Yes,
    No,
    Warn
  };

  struct SourceAssetRequiredDeclare final : public zs::EnumDeclare<SourceAssetRequired, 3>
  {
    constexpr const Entries operator()() const noexcept
    {
      return { {
        {SourceAssetRequired::Yes, "yes"},
        {SourceAssetRequired::No, "no"},
        {SourceAssetRequired::Warn, "warn"}
      } };
    }
  };

  using SourceAssetRequiredTraits = zs::EnumTraits<SourceAssetRequired, SourceAssetRequiredDeclare>;

  enum class YesNo {
    Yes,
    No
  };
  struct YesNoDeclare final : public zs::EnumDeclare<YesNo, 2>
  {
    constexpr const Entries operator()() const noexcept
    {
      return { {
        {YesNo::Yes, "yes"},
        {YesNo::No, "no"}
      } };
    }
  };

  using YesNoTraits = zs::EnumTraits<YesNo, YesNoDeclare>;

  enum class YesNoAlwaysNever {
    Yes,
    No,
    Always,
    Never
  };
  struct YesNoAlwaysNeverDeclare final : public zs::EnumDeclare<YesNoAlwaysNever, 4>
  {
    constexpr const Entries operator()() const noexcept
    {
      return { {
        {YesNoAlwaysNever::Yes, "yes"},
        {YesNoAlwaysNever::No, "no"},
        {YesNoAlwaysNever::Always, "always"},
        {YesNoAlwaysNever::Never, "never"},
      } };
    }
  };
  using YesNoAlwaysNeverTraits = zs::EnumTraits<YesNoAlwaysNever, YesNoAlwaysNeverDeclare>;

  enum class FaultOptions {
    Yes,
    No,
    Always,
    Never,
    Error,
    Default,
    Lock,
    Unlock,
    Push,
    Pop
  };
  struct FaultOptionsDeclare final : public zs::EnumDeclare<FaultOptions, 10>
  {
    constexpr const Entries operator()() const noexcept
    {
      return { {
        {FaultOptions::Yes, "yes"},
        {FaultOptions::No, "no"},
        {FaultOptions::Always, "always"},
        {FaultOptions::Never, "never"},
        {FaultOptions::Error, "error"},
        {FaultOptions::Default, "default"},
        {FaultOptions::Lock, "lock"},
        {FaultOptions::Unlock, "unlock"},
        {FaultOptions::Push, "push"},
        {FaultOptions::Pop, "pop"},
      } };
    }
  };

  using FaultOptionsTraits = zs::EnumTraits<FaultOptions, FaultOptionsDeclare>;

  struct SourceAssetDirective
  {
    TokenConstPtr token_;
    String file_;
    String rename_;
    ParserTypes::Extraction unresolvedFile_;
    ParserTypes::Extraction unresolvedRename_;
    SourceAssetRequired required_{};
    bool generated_{};
  };

  struct DirectiveResult
  {
    Tokenizer::iterator openIter_;
    Tokenizer::iterator closeIter_;
    Tokenizer::iterator literalIter_;
    Tokenizer::iterator afterIter_;
    bool success_{ true };
  };
  
  struct DirectiveLiteral
  {
    Tokenizer::iterator literalIter_;
    Tokenizer::iterator afterIter_;
    String name_;
  };


  struct ParseDirectiveFunctions
  {
    std::function<bool(bool, StringView)> legalName_;
    std::function<bool(bool, Tokenizer::iterator, StringView)> noValueFunc_;
    std::function<bool(bool, Tokenizer::iterator, StringView, StringView)> literalValueFunc_;
    std::function<bool(bool, Tokenizer::iterator, StringView, StringView)> quoteValueFunc_;
    std::function<bool(bool, Tokenizer::iterator, StringView, StringView)> numberValueFunc_;
    std::function<bool(bool, Tokenizer::iterator, StringView, ParserTypes::Extraction&)> extractedValueFunc_;
  };
};

//-----------------------------------------------------------------------------
struct ParserOtherTypes
{
  struct SourceAsset {
    TokenConstPtr token_;
    CompileStatePtr compileState_;
    String filePath_;
    String fullFilePath_;
    String renameFilePath_;
    ParserDirectiveTypes::SourceAssetRequired required_{};
    bool generated_{};
    bool commandLine_{};
  };

  using SourceAssetList = std::list<SourceAsset>;
};

} // namespace zax
