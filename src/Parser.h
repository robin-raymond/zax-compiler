
#pragma once

#include "types.h"
#include "ParserTypes.h"
#include "ParserDirectiveTypes.h"

namespace zax
{

//-----------------------------------------------------------------------------
struct Parser : public ParserTypes,
                public ParserDirectiveTypes
{
  const Puid id_{ puid() };
  Config config_;
  Callbacks callbacks_;
  OperatorLutPtr operatorLut_;

  ModuleWeakPtr importer_;
  ModulePtr module_;
  ModuleMap imports_;

  ContextPtr rootContext_;

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

  [[nodiscard]] std::optional<DirectiveResult> parseDirective(
    Context& context,
    Tokenizer::iterator iter,
    ParseDirectiveFunctions& functions) noexcept;
  [[nodiscard]] static std::optional<DirectiveLiteralResult> parseDirectiveLiteral(const Context& context, Tokenizer::iterator iter) noexcept;

  [[nodiscard]] bool consumeLineParserDirective(Context& context) noexcept;
  [[nodiscard]] bool consumeAssetOrSourceDirective(Context& context, Tokenizer::iterator iter, bool isSource) noexcept;
  [[nodiscard]] bool consumeTabStopDirective(Context& context, Tokenizer::iterator iter) noexcept;
  [[nodiscard]] bool consumeFileAssignDirective(Context& context, Tokenizer::iterator iter) noexcept;
  [[nodiscard]] bool consumeLineAssignDirective(Context& context, Tokenizer::iterator iter) noexcept;
  [[nodiscard]] bool consumePanicDirective(Context& context, Tokenizer::iterator iter) noexcept;
  [[nodiscard]] bool consumeWarningDirective(Context& context, Tokenizer::iterator iter) noexcept;
  [[nodiscard]] bool consumeErrorDirective(Context& context, Tokenizer::iterator iter) noexcept;
  [[nodiscard]] bool consumeDeprecateDirective(Context& context, Tokenizer::iterator iter) noexcept;
  [[nodiscard]] bool consumeExportDirective(Context& context, Tokenizer::iterator iter) noexcept;
  [[nodiscard]] bool consumeVariablesDirective(Context& context, Tokenizer::iterator iter) noexcept;
  [[nodiscard]] bool consumeTypesDirective(Context& context, Tokenizer::iterator iter) noexcept;
  [[nodiscard]] bool consumeFunctionsDirective(Context& context, Tokenizer::iterator iter) noexcept;

  [[nodiscard]] bool consumeAlias(Context& context) noexcept;
  [[nodiscard]] bool consumeKeywordAlias(Context& context, Tokenizer::iterator iter) noexcept;

  [[nodiscard]] static std::optional<QuoteResult> parseQuote(Tokenizer::iterator iter) noexcept;
  [[nodiscard]] static std::optional<NumberResult> parseSimpleNumber(const Context& context, Tokenizer::iterator iter) noexcept;

  [[nodiscard]] static Tokenizer::iterator skipUntil(Tokenizer& tokenizer, std::function<bool(const TokenPtr&)>&& until) noexcept;
  [[nodiscard]] static Tokenizer::iterator skipUntil(std::function<bool(const TokenPtr&)>&& until, Tokenizer::iterator iter) noexcept;
  [[nodiscard]] static Tokenizer::iterator skipUntilAfter(Tokenizer& tokenizer, std::function<bool(const TokenPtr&)>&& until) noexcept;
  [[nodiscard]] static Tokenizer::iterator skipUntilAfter(std::function<bool(const TokenPtr&)>&& until, Tokenizer::iterator iter) noexcept;

  [[nodiscard]] bool consumeSeparator(Context& context, bool forcedOkay) noexcept;
  [[nodiscard]] bool consumeSeparators(Context& context, bool forcedOkay) noexcept;

  [[nodiscard]] Tokenizer::iterator consumeTo(Tokenizer::iterator  iter) noexcept;
  [[nodiscard]] Tokenizer::iterator consumeAfter(Tokenizer::iterator  iter) noexcept;

  [[nodiscard]] Tokenizer::iterator consumeTo(std::function<bool(const TokenPtr&)>&& until, Tokenizer::iterator  iter) noexcept;
  [[nodiscard]] Tokenizer::iterator consumeAfter(std::function<bool(const TokenPtr&)>&& until, Tokenizer::iterator  iter) noexcept;

  [[nodiscard]] static TokenConstPtr pickValid(const TokenConstPtr& tokenPreferred, const TokenConstPtr& tokenBackup) noexcept { return tokenPreferred ? tokenPreferred : tokenBackup; }
  [[nodiscard]] static TokenConstPtr pickValid(const TokenConstPtr& tokenPreferred, const TokenConstPtr& tokenBackup1, const TokenConstPtr& tokenBackup2) noexcept { return pickValid(pickValid(tokenPreferred, tokenBackup1), tokenBackup2); }
  [[nodiscard]] TokenConstPtr validOrLastValid(const TokenConstPtr& token, Tokenizer& tokenizer) const noexcept;
  [[nodiscard]] TokenConstPtr validOrLastValid(const TokenConstPtr& token, Tokenizer::iterator iter) const noexcept { return validOrLastValid(token, iter.list()); }

  [[nodiscard]] Extraction extract(Context& context, Tokenizer::iterator first, Tokenizer::iterator last) noexcept;

  void handleAsset(Context& context, SourceAssetDirective&) noexcept;
  void handleSource(Context& context, SourceAssetDirective&) noexcept;

  [[nodiscard]] static std::optional<Operator> extractOperator(const Context& context, const TokenConstPtr& token) noexcept;
  [[nodiscard]] static bool isOperator(const Context& context, const TokenConstPtr& token, Operator oper) noexcept;
  [[nodiscard]] bool isOperatorOrAlternative(const Context& context, const TokenConstPtr& token, Operator oper) noexcept;
  [[nodiscard]] static bool isOperatorOrAlternative(const Context& context, const OperatorLut& lut, const TokenConstPtr& token, Operator oper) noexcept;
  [[nodiscard]] static bool isLiteral(const TokenConstPtr& token) noexcept;
  [[nodiscard]] static bool isLiteral(const TokenConstPtr& token, StringView value) noexcept;
  [[nodiscard]] static bool isNumber(const TokenConstPtr& token) noexcept;
  [[nodiscard]] static bool isSeparator(const TokenConstPtr& token) noexcept;
  [[nodiscard]] static bool isQuote(const TokenConstPtr& token) noexcept;

  [[nodiscard]] static bool isKeyword(const Context& context, const TokenConstPtr& token, Keyword keyword) noexcept;
  [[nodiscard]] static optional<Keyword> extractKeyword(const Context& context, const TokenConstPtr& token) noexcept;

  [[nodiscard]] static bool isCommaOrCloseDirective(const Context& context, const TokenConstPtr& token) noexcept;
  [[nodiscard]] static bool isUnknownExtension(const StringView name) noexcept;

  [[nodiscard]] static std::function<bool(const TokenConstPtr& token)> isSeparatorFunc() noexcept;
  [[nodiscard]] static std::function<bool(const TokenConstPtr& token)> isOperatorFunc(const Context& context, Operator oper) noexcept;
  [[nodiscard]] std::function<bool(const TokenConstPtr& token)> isOperatorOrAlternativeFunc(Operator oper) noexcept;
  [[nodiscard]] static std::function<bool(const TokenConstPtr& token)> isOperatorOrAlternativeFunc(const OperatorLut& lut, Operator oper) noexcept;

protected:
  void prime() noexcept;
  Tokenizer& getSourceTokenizer() noexcept;
  TokenizerPtr getSourceTokenizerPtr() noexcept;

  Context& getSourceContext() noexcept;
  ContextPtr getSourceContextPtr() noexcept;

public:
  void fatal(Error error, const TokenConstPtr& token, const StringMap& mapping = {}) noexcept;
  void out(Error error, const TokenConstPtr& token, const StringMap& mapping = {}) noexcept;
  void out(Warning warning, const TokenConstPtr& token, const StringMap& mapping = {}) noexcept;
  void out(Informational info, const TokenConstPtr& token, const StringMap& mapping = {}) noexcept;
  [[nodiscard]] bool shouldAbort() noexcept;
};

} // namespace zax
