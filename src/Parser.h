
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
    TokenPtr token_;
    String quote_;
    Tokenizer::iterator iter_;
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
struct ParserOtherTypes
{
  struct SourceAsset {
    TokenPtr token_;
    CompileStatePtr compileState_;
    String filePath_;
    String fullFilePath_;
    String renameFilePath_;
    ParserDirectiveTypes::SourceAssetRequired required_{};
    bool generated_{};
    bool commandLine_{};
  };

  using SourceAssetList = std::list<SourceAsset>;

  struct Context
  {
    Parser* parser_{};
    Tokenizer* tokenizer_{};
    CompileStatePtr singleLineState_;

    Tokenizer& operator*() noexcept { return *tokenizer_; }
    const Tokenizer& operator*() const noexcept { return *tokenizer_; }

    Tokenizer* operator->() noexcept { return tokenizer_; }
    const Tokenizer* operator->() const noexcept { return tokenizer_; }
  };
};

//-----------------------------------------------------------------------------
struct Parser : public ParserTypes,
                public ParserDirectiveTypes,
                public ParserOtherTypes
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
  std::set<Path> alreadyIncludedSources_;

  Parser(
    const Config& config,
    Callbacks* callbacks = nullptr) noexcept;

  Parser() noexcept = delete;
  Parser(const Parser&) noexcept = delete;
  Parser(Parser&&) noexcept = delete;

  Parser& operator=(const Parser&) noexcept = delete;
  Parser& operator=(Parser&&) noexcept = delete;

  void parse() noexcept;
  void process(Context& context) noexcept;
  void processAssets() noexcept;

  [[nodiscard]] bool consumeLineParserDirective(Context& context) noexcept;
  bool consumeAssetOrSourceDirective(Context& context, bool isSource) noexcept;

  [[nodiscard]] std::pair<Tokenizer::iterator, bool> extractDirectiveEqualsQuoteOrResolveLater(
    Tokenizer::iterator iter,
    String& output,
    TokenPtr& outStringToken,
    TokenizerPtr& outUnresolved,
    bool warnIfNotFound = true) noexcept;

  [[nodiscard]] std::pair<Tokenizer::iterator, bool> extractDirectiveEqualsLiteral(
    Tokenizer::iterator iter,
    String& output,
    bool warnIfNotFound = true) noexcept;

  [[nodiscard]] static Tokenizer::iterator skipUntil(Tokenizer& tokenizer, std::function<bool(const TokenPtr&)>&& until) noexcept;
  [[nodiscard]] static Tokenizer::iterator skipUntil(std::function<bool(const TokenPtr&)>&& until, Tokenizer::iterator iter) noexcept;
  [[nodiscard]] static Tokenizer::iterator skipUntilAfter(Tokenizer& tokenizer, std::function<bool(const TokenPtr&)>&& until) noexcept;
  [[nodiscard]] static Tokenizer::iterator skipUntilAfter(std::function<bool(const TokenPtr&)>&& until, Tokenizer::iterator iter) noexcept;

  [[nodiscard]] bool consumeSeparator(Context& context, bool forcedOkay) noexcept;
  [[nodiscard]] bool consumeSeparators(Context& context, bool forcedOkay) noexcept;

  [[nodiscard]] Tokenizer::iterator consumeTo(Tokenizer::iterator  iter) noexcept;
  [[nodiscard]] Tokenizer::iterator consumeAfter(Tokenizer::iterator  iter) noexcept;

  [[nodiscard]] static TokenPtr pickValid(const TokenPtr& tokenPreferred, const TokenPtr& tokenBackup) noexcept { return tokenPreferred ? tokenPreferred : tokenBackup; }
  [[nodiscard]] TokenPtr validOrLastValid(Tokenizer& tokenizer, const TokenPtr& token) noexcept;
  [[nodiscard]] TokenPtr validOrLastValid(Tokenizer::iterator iter, const TokenPtr& token) noexcept { return validOrLastValid(iter.list(), token); }
  [[nodiscard]] CompileStatePtr pickState(Context& context, TokenPtr token) noexcept;

  [[nodiscard]] TokenizerPtr extract(Tokenizer::iterator first, Tokenizer::iterator last) noexcept;
  
  [[nodiscard]] QuoteResult parseQuote(Tokenizer::iterator iter) noexcept;

  void handleAsset(Context& context, SourceAssetDirective&) noexcept;
  void handleSource(Context& context, SourceAssetDirective&) noexcept;

  [[nodiscard]] static bool isOperator(const TokenConstPtr& token, Operator oper) noexcept;
  [[nodiscard]] static bool isLiteral(const TokenConstPtr& token) noexcept;
  [[nodiscard]] static bool isSeparator(const TokenConstPtr& token) noexcept;
  [[nodiscard]] static bool isQuote(const TokenConstPtr& token) noexcept;

  [[nodiscard]] static bool isCommaOrCloseDirective(const TokenConstPtr& token) noexcept;

  [[nodiscard]] static std::function<bool(const TokenConstPtr& token)> isOperatorFunc(Operator oper) noexcept;

protected:
  void prime() noexcept;
  Tokenizer& getTokenizer() noexcept;

  void fatal(Error error, const TokenConstPtr& token, const StringMap& mapping = {}) noexcept;
  void out(Error error, const TokenConstPtr& token, const StringMap& mapping = {}) noexcept;
  void out(Warning warning, const TokenConstPtr& token, const StringMap& mapping = {}) noexcept;
  void out(Informational info, const TokenConstPtr& token, const StringMap& mapping = {}) noexcept;
  [[nodiscard]] bool shouldAbort() noexcept;
};

} // namespace zax
