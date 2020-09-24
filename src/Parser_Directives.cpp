
#include "pch.h"
#include "Parser.h"

//#include "ParserException.h"
//#include "CompileState.h"
//#include "OperatorLut.h"
//#include "Source.h"
//#include "Tokenizer.h"

using namespace zax;

using namespace std::string_view_literals;

//-----------------------------------------------------------------------------
bool Parser::consumeLineParserDirective(Context& context) noexcept
{
  if (!isOperator(context->front(), Operator::DirectiveOpen))
    return false;

  auto iter{ context->begin() + 1 };

  auto literal{ *iter };
  if (!isLiteral(literal)) {
    out(Warning::DirectiveNotUnderstood, literal);
    (void)consumeAfter(skipUntil(*context, isOperatorFunc(Operator::DirectiveClose)));
    return true;
  }
  ++iter;

  if ("asset"sv == literal->token_) {
    (void)consumeTo(iter);
    return consumeAssetOrSourceDirective(context, false);
  }

  if ("source"sv == literal->token_) {
    (void)consumeTo(iter);
    return consumeAssetOrSourceDirective(context, true);
  }

  return false;
}

//-----------------------------------------------------------------------------
bool Parser::consumeAssetOrSourceDirective(Context& context, bool isSource) noexcept
{
  auto iter{ context->begin() };

  SourceAssetDirective asset;
  asset.token_ = validOrLastValid(iter, *iter);

  auto [resultIter, success] = extractDirectiveEqualsQuoteOrResolveLater(iter, asset.file_, asset.token_, asset.unresolvedFile_);
  iter = resultIter;

  bool foundRequired{};
  bool foundGenerated{};
  bool foundRename{};
  while (isOperator(*iter, Operator::Comma)) {
    ++iter;

    auto literal{ *iter };
    if (!isLiteral(literal)) {
      out(Warning::DirectiveNotUnderstood, validOrLastValid(iter, literal));
      iter = consumeTo(skipUntil(isCommaOrCloseDirective, iter));
      continue;
    }

    if ("required"sv == literal->token_) {
      String str;
      auto [afterIter, foundLiteral] = extractDirectiveEqualsLiteral(iter + 1, str);
      iter = afterIter;

      auto value{ SourceAssetRequiredTraits::toEnum(str) };
      if (foundRequired || (!value)) {
        iter = consumeTo(skipUntil(isCommaOrCloseDirective, iter));
        if (foundLiteral)
          out(Warning::DirectiveNotUnderstood, validOrLastValid(iter, literal));
        continue;
      }
      asset.required_ = *value;
      foundRequired = true;
      continue;
    }
    if ("generated"sv == literal->token_) {
      String str;
      auto [afterIter, foundLiteral] = extractDirectiveEqualsLiteral(iter + 1, str);
      iter = afterIter;

      auto value{ YesNoTraits::toEnum(str) };
      if (foundGenerated || (!value)) {
        iter = consumeTo(skipUntil(isCommaOrCloseDirective, iter));
        if (foundLiteral)
          out(Warning::DirectiveNotUnderstood, validOrLastValid(iter, literal));
        continue;
      }
      asset.generated_ = *value == YesNo::Yes ? true : false;
      foundGenerated = true;
      continue;
    }
    if ("rename"sv == literal->token_) {
      if (foundRename || isSource) {
        iter = consumeTo(skipUntil(isCommaOrCloseDirective, iter));
        out(Warning::DirectiveNotUnderstood, validOrLastValid(iter, literal));
        continue;
      }
      TokenPtr renameToken;
      iter = extractDirectiveEqualsQuoteOrResolveLater(++iter, asset.rename_, renameToken, asset.unresolvedRename_).first;
      continue;
    }

    iter = consumeTo(skipUntil(isCommaOrCloseDirective, iter));
    out(Warning::DirectiveNotUnderstood, validOrLastValid(iter, literal));
  }

  if (!isOperator(*iter, Operator::DirectiveClose)) {
    out(Error::Syntax, pickValid(validOrLastValid(iter, *iter), asset.token_));
    (void)consumeTo(iter);
    return true;
  }
  (void)consumeTo(++iter);

  if (success) {
    if (isSource)
      handleSource(context, asset);
    else
      handleAsset(context, asset);
  }
  return true;
}

//-----------------------------------------------------------------------------
std::pair<Tokenizer::iterator, bool> Parser::extractDirectiveEqualsQuoteOrResolveLater(
  Tokenizer::iterator iter,
  String& output,
  TokenPtr& outStringToken,
  TokenizerPtr& outUnresolved,
  bool warnIfNotFound
) noexcept
{
  output = {};

  auto equalsToken{ *iter };
  if (!isOperator(equalsToken, Operator::Assign)) {
    if (warnIfNotFound) {
      out(Warning::DirectiveNotUnderstood, validOrLastValid(iter, equalsToken));
      iter = consumeTo(skipUntil(iter.list(), isCommaOrCloseDirective));
    }
    return std::make_pair(iter, false);
  }

  ++iter;

  if (!*iter) {
    out(Error::Syntax, validOrLastValid(iter, equalsToken));
    iter = consumeTo(skipUntil(iter.list(), isCommaOrCloseDirective));
    return std::make_pair(iter, false);
  }

  auto quoteResult{ parseQuote(iter) };
  iter = quoteResult.iter_;
  output = quoteResult.quote_;

  if (!quoteResult.token_) {
    auto afterIter{ skipUntil(isCommaOrCloseDirective, iter) };
    outUnresolved = extract(iter, afterIter);
    iter = afterIter;
  }
  else {
    outStringToken = quoteResult.token_;
  }
  return std::make_pair(iter, true);
}

//-----------------------------------------------------------------------------
std::pair<Tokenizer::iterator, bool> Parser::extractDirectiveEqualsLiteral(
  Tokenizer::iterator iter,
  String& output,
  bool warnIfNotFound) noexcept
{
  output = {};
  
  auto equalsToken{ *iter };
  if (!isOperator(equalsToken, Operator::Assign)) {
    if (warnIfNotFound) {
      out(Warning::DirectiveNotUnderstood, validOrLastValid(iter, equalsToken));
      iter = consumeAfter(skipUntil(iter.list(), isCommaOrCloseDirective));
    }
    return std::make_pair(iter, false);
  }
  ++iter;

  if (!isLiteral(*iter)) {
    out(Error::Syntax, validOrLastValid(iter, pickValid(*iter, equalsToken)));
    return std::make_pair(consumeAfter(skipUntil(iter.list(), isCommaOrCloseDirective)), false);
  }

  output = (*iter)->token_;
  return std::make_pair(++iter, true);
}

//-----------------------------------------------------------------------------
void Parser::handleAsset(Context& context, SourceAssetDirective& asset) noexcept
{
  if ((asset.unresolvedFile_) ||
      (asset.unresolvedRename_)) {
#define TODO_RESOLVED_ASSET 1
#define TODO_RESOLVED_ASSET 2
    return;
  }

  SourceAsset newAsset;
  newAsset.token_ = asset.token_;
  newAsset.compileState_ = pickState(context, asset.token_);
  newAsset.filePath_ = asset.file_;
  newAsset.fullFilePath_ = asset.file_;
  newAsset.required_ = asset.required_;
  newAsset.renameFilePath_ = asset.rename_;
  newAsset.generated_ = asset.generated_;

  std::list<LocateWildCardFilesResult> results;
  locateWildCardFiles(results, asset.token_->actualOrigin_.filePath_->filePath_, asset.file_);

  if (results.size()) {
    for (auto& located : results) {
      newAsset.filePath_ = located.path_;
      newAsset.fullFilePath_ = located.fullPath_;
      newAsset.renameFilePath_ = asset.rename_;

      String& replacingStr{ newAsset.renameFilePath_ };
      for (auto& match : located.foundMatches_) {
        auto pos{ replacingStr.find_first_of("?*"sv) };
        if (String::npos == pos)
          break;
        replacingStr = replacingStr.substr(0, pos) + match + replacingStr.substr(pos + 1);
      }
      pendingAssets_.push_front(newAsset);
    }
  }
  else {
    pendingAssets_.push_back(newAsset);
  }
}

//-----------------------------------------------------------------------------
void Parser::handleSource(Context& context, SourceAssetDirective& source) noexcept
{
  if (source.unresolvedFile_) {
#define TODO_RESOLVED_SOURCE 1
#define TODO_RESOLVED_SOURCE 2
    return;
  }
  
  SourceAsset newSource;
  newSource.token_ = source.token_;
  newSource.compileState_ = pickState(context, source.token_);
  newSource.filePath_ = source.file_;
  newSource.fullFilePath_ = source.file_;
  newSource.required_ = source.required_;
  newSource.generated_ = source.generated_;

  std::list<LocateWildCardFilesResult> results;
  locateWildCardFiles(results, source.token_->actualOrigin_.filePath_->filePath_, source.file_);

  if (results.size()) {
    for (auto& located : results) {
      newSource.filePath_ = located.path_;
      newSource.fullFilePath_ = located.fullPath_;
      pendingSources_.push_back(newSource);
    }
  }
  else {
    pendingSources_.push_front(newSource);
  }
}
