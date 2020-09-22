
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
  struct CompilerTypes
{
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

};

//-----------------------------------------------------------------------------
struct CompileDirectiveTypes
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

  struct SourceAssetDirective
  {
    TokenPtr token_;
    String file_;
    String rename_;
    TokenizerPtr unresolvedFile_;
    TokenizerPtr unresolvedRename_;
    SourceAssetRequired required_{};
    bool generated_{};
  };
};

//-----------------------------------------------------------------------------
struct CompilerOtherTypes
{
  struct SourceAsset {
    TokenPtr token_;
    CompileStatePtr compileState_;
    String filePath_;
    String fullFilePath_;
    String renameFilePath_;
    CompileDirectiveTypes::SourceAssetRequired required_{};
    bool generated_{};
    bool commandLine_{};
  };

  using SourceAssetList = std::list<SourceAsset>;
};

//-----------------------------------------------------------------------------
struct Compiler : public CompilerTypes,
                  public CompileDirectiveTypes,
                  public CompilerOtherTypes
{
  Config config_;
  Callbacks callbacks_;
  CompileStatePtr activeState_;
  OperatorLutPtr operatorLut_;

  ModulePtr rootModule_;
  ModuleMap imports_;
  ModuleList anonymousImports_;

  SourceAssetList pendingSources_;
  SourceAssetList pendingAssets_;

  SourceList sources_;
  SourceList processedSources_;

  Compiler(
    const Config& config,
    Callbacks* callbacks = nullptr) noexcept;

  Compiler() noexcept = delete;
  Compiler(const Compiler&) noexcept = delete;
  Compiler(Compiler&&) noexcept = delete;

  Compiler& operator=(const Compiler&) noexcept = delete;
  Compiler& operator=(Compiler&&) noexcept = delete;

  void compile() noexcept;
  void process(Tokenizer& tokenizer) noexcept;

  [[nodiscard]] bool consumeLineCompilerDirective(Tokenizer& tokenizer) noexcept;
  bool consumeAssetOrSourceDirective(Tokenizer& tokenizer, bool isSource) noexcept;

  [[nodiscard]] Tokenizer::iterator extractDirectiveEqualsQuoteOrResolveLater(
    Tokenizer::iterator iter,
    String& output,
    TokenizerPtr& outUnresolved,
    bool warnIfNotFound = true) noexcept;

  [[nodiscard]] Tokenizer::iterator extractDirectiveEqualsLiteral(
    Tokenizer::iterator iter,
    String& output,
    bool warnIfNotFound = true) noexcept;

  [[nodiscard]] static Tokenizer::iterator skipUntil(Tokenizer& tokenizer, std::function<bool(const TokenPtr&)>&& until) noexcept;
  [[nodiscard]] static Tokenizer::iterator skipUntil(std::function<bool(const TokenPtr&)>&& until, Tokenizer::iterator iter) noexcept;
  [[nodiscard]] static Tokenizer::iterator skipUntilAfter(Tokenizer& tokenizer, std::function<bool(const TokenPtr&)>&& until) noexcept;
  [[nodiscard]] static Tokenizer::iterator skipUntilAfter(std::function<bool(const TokenPtr&)>&& until, Tokenizer::iterator iter) noexcept;

  [[nodiscard]] bool consumeSeparator(Tokenizer& tokenizer, bool forcedOkay) noexcept;
  [[nodiscard]] bool consumeSeparators(Tokenizer& tokenizer, bool forcedOkay) noexcept;

  Tokenizer::iterator consumeTo(Tokenizer::iterator  iter) noexcept;
  [[nodiscard]] Tokenizer::iterator consumeAfter(Tokenizer::iterator  iter) noexcept;

  TokenPtr validOrLastValid(Tokenizer& tokenizer, const TokenPtr& token) noexcept;
  TokenPtr validOrLastValid(Tokenizer::iterator iter, const TokenPtr& token) noexcept { return validOrLastValid(iter.list(), token); }

  TokenizerPtr extract(Tokenizer::iterator first, Tokenizer::iterator last) noexcept;

  void handleAsset(SourceAssetDirective&) noexcept;
  void handleSource(SourceAssetDirective&) noexcept;

  static bool isOperator(const TokenConstPtr& token, Operator oper) noexcept;
  static bool isLiteral(const TokenConstPtr& token) noexcept;
  static bool isSeparator(const TokenConstPtr& token) noexcept;
  static bool isQuote(const TokenConstPtr& token) noexcept;

  static bool isCommaOrCloseDirective(const TokenConstPtr& token) noexcept;

  static std::function<bool(const TokenConstPtr& token)> isOperatorFunc(Operator oper) noexcept;

protected:
  void prime() noexcept;
  Tokenizer& getTokenizer() noexcept;

  void fatal(Error error, const TokenConstPtr& token, const StringMap& mapping = {}) noexcept;
  void out(Error error, const TokenConstPtr& token, const StringMap& mapping = {}) noexcept;
  void out(Warning warning, const TokenConstPtr& token, const StringMap& mapping = {}) noexcept;
  void out(Informational info, const TokenConstPtr& token, const StringMap& mapping = {}) noexcept;
  bool shouldAbort() noexcept;
};

} // namespace zax
