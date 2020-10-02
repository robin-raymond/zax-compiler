
#pragma once

#include "types.h"
#include "Token.h"
#include "Errors.h"
#include "Warnings.h"
#include "Informationals.h"
#include "Tokenizer.h"
#include "ParserTypes.h"

namespace zax
{

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

  enum class PushPop {
    Push,
    Pop
  };
  struct PushPopDeclare final : public zs::EnumDeclare<PushPop, 2>
  {
    constexpr const Entries operator()() const noexcept
    {
      return { {
        {PushPop::Push, "push"},
        {PushPop::Pop, "pop"}
      } };
    }
  };

  using PushPopTraits = zs::EnumTraits<PushPop, PushPopDeclare>;

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


  enum class Inline {
    Maybe,
    Always,
    Descope,
    Never
  };
  struct InlineDeclare final : public zs::EnumDeclare<Inline, 4>
  {
    constexpr const Entries operator()() const noexcept
    {
      return { {
        {Inline::Maybe, "maybe"},
        {Inline::Always, "always"},
        {Inline::Descope, "descope"},
        {Inline::Never, "never"}
      } };
    }
  };

  using InlineTraits = zs::EnumTraits<Inline, InlineDeclare>;

  enum class Return {
    Normal,
    Never,
    Interrupt
  };
  struct ReturnDeclare final : public zs::EnumDeclare<Return, 3>
  {
    constexpr const Entries operator()() const noexcept
    {
      return { {
        {Return::Normal, "normal"},
        {Return::Never, "never"},
        {Return::Interrupt, "interrupt"}
      } };
    }
  };

  using ReturnTraits = zs::EnumTraits<Return, ReturnDeclare>;

  enum class ResolveOption {
    Trial,
    Lazy,
    Last,
    Now
  };
  struct ResolveOptionDeclare final : public zs::EnumDeclare<ResolveOption, 4>
  {
    constexpr const Entries operator()() const noexcept
    {
      return { {
        {ResolveOption::Trial, "trial"},
        {ResolveOption::Lazy, "lazy"},
        {ResolveOption::Last, "last"},
        {ResolveOption::Now, "now"}
      } };
    }
  };

  using ResolveOptionTraits = zs::EnumTraits<ResolveOption, ResolveOptionDeclare>;


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
  
  struct DirectiveLiteralResult
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

  struct Descope
  {
    TokenConstPtr literal_;
  };
  struct Discard
  {
    TokenConstPtr literal_;
  };

  struct SourceAsset {
    TokenConstPtr token_;
    CompileStatePtr compileState_;
    String filePath_;
    String fullFilePath_;
    String renameFilePath_;
    SourceAssetRequired required_{};
    bool generated_{};
    bool commandLine_{};
    int parentTabStopWidth_{ 8 };
  };

  using SourceAssetList = std::list<SourceAsset>;

  struct Resolve
  {
    ResolveOption option_{};
    bool retry_{ true };
  };
};

} // namespace zax
